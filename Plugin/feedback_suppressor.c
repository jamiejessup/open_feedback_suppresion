/*
  LV2 Sampler Example Plugin
  Copyright 2011-2012 David Robillard <d@drobilla.net>
  Copyright 2011 Gabriel M. Beddingfield <gabriel@teuton.org>
  Copyright 2011 James Morris <jwm.art.net@gmail.com>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifndef __cplusplus
#    include <stdbool.h>
#endif

#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/log/logger.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#include <fftw3.h>

#include "./uris.h"
#include "biquad_filter.h"

#define DEFAULT_BANK_SIZE 12
#define MAX_BANK_SIZE 12
#define DEFAULT_N 24*2048

enum {
	FS_CONTROL 	= 0,
	FS_NOTIFY  	= 1,
	FS_IN		= 2,
	FS_OUT     	= 3
};

typedef struct {
	char *path;
	size_t path_len;
	float *notch_fcs;
	size_t list_len;
} FilterList;

typedef struct {
	// Features
	LV2_URID_Map*        map;
	LV2_Worker_Schedule* schedule;
	LV2_Log_Log*     log;

	// Forge for creating atoms
	LV2_Atom_Forge forge;

	// Logger convenience API
	LV2_Log_Logger logger;

	double sample_rate;
	//Filter List
	FilterList *filter_list;

	BiQuadFilter *filter_bank;
	unsigned max_filters;
	unsigned bank_index;

	//stuff for the FFT I/O buffers
	unsigned N;
	unsigned sample_count;
	double *fft_audio_in;
	fftw_complex *fft_audio_out;
	fftw_plan plan;
	float *power_spectrum;
	float *power_spectrum_db;

	// Ports
	const LV2_Atom_Sequence* control_port;
	LV2_Atom_Sequence*       notify_port;
	float*                   output_port;
	const float* input_port;

	// Forge frame for notify port (for writing worker replies)
	LV2_Atom_Forge_Frame notify_frame;

	// URIs
	FeedbackSuppressorURIs uris;
} FeedbackSuppressor;


/**
   An atom-like message used internally to apply/free filter lists.

   This is only used internally to communicate with the worker, it is never
   sent to the outside world via a port since it is not POD.  It is convenient
   to use an Atom header so actual atoms can be easily sent through the same
   ringbuffer.
 */
typedef struct {
	LV2_Atom atom;
	FilterList*  list;
} FilterListMessage;

/**
   Load a new filter list and return it.

   Since this is of course not a real-time safe action, this is called in the
   worker thread only.  The sample is loaded and returned only, plugin state is
   not modified.
 */
static FilterList*
load_filter_list(FeedbackSuppressor *self, const char *path) {
	const size_t path_len  = strlen(path);
	lv2_log_trace(&self->logger, "Loading filter list %s\n", path);

	FilterList* const  list  = (FilterList*)malloc(sizeof(FilterList));


	char line[50];
	//parse the file to load a filter list
	FILE *list_file;
	list_file = fopen(path,"rt");
	float notches[MAX_BANK_SIZE];
	int i = 0;
	if(list_file != NULL) {
		while(fgets(line,50,list_file)) {
			//convert the line to a float
			if(i==MAX_BANK_SIZE) break;
			notches[i] = atof(line);
		}
	}

	list->list_len = i+1;
	list->path     = (char*)malloc(path_len + 1);
	list->notch_fcs = (float*)malloc((i+1)*sizeof(float));
	for(int j=0; j<i+1; j++){
		list->notch_fcs[j] = notches[j];
	}
	list->path_len = path_len;
	memcpy(list->path, path, path_len + 1);

	return list;


}


/**
 * Free a filter list
 */
static void
free_filter_list(FeedbackSuppressor* self, FilterList* list)
{
	if (list) {
		lv2_log_trace(&self->logger, "Freeing %s\n", list->path);
		free(list->path);
		free(list->notch_fcs);
		free(list);
	}
}

/**
   Do work in a non-realtime thread.

   This is called for every piece of work scheduled in the audio thread using
   self->schedule->schedule_work().  A reply can be sent back to the audio
   thread using the provided respond function.
 */
static LV2_Worker_Status
work(LV2_Handle                  instance,
		LV2_Worker_Respond_Function respond,
		LV2_Worker_Respond_Handle   handle,
		uint32_t                    size,
		const void*                 data)
{
	FeedbackSuppressor*        self = (FeedbackSuppressor*)instance;
	const LV2_Atom* atom = (const LV2_Atom*)data;
	if (atom->type == self->uris.freeFilterList) {
		// Free old filter list
		const FilterListMessage* msg = (const FilterListMessage*)data;
		free_filter_list(self, msg->list);
	} else {
		// Handle set message (load filter list).
		const LV2_Atom_Object* obj = (const LV2_Atom_Object*)data;

		// Get file path from message
		const LV2_Atom* file_path = read_set_file(&self->uris, obj);
		if (!file_path) {
			return LV2_WORKER_ERR_UNKNOWN;
		}

		//load the filter list
		FilterList *list = load_filter_list(self,LV2_ATOM_BODY_CONST(file_path));
		// Loaded filter list, send it to run() to be applied.
		if (list) {
			respond(handle, sizeof(list), &list);
		}
	}

	return LV2_WORKER_SUCCESS;
}

/**
   Handle a response from work() in the audio thread.

   When running normally, this will be called by the host after run().  When
   freewheeling, this will be called immediately at the point the work was
   scheduled.
 */
static LV2_Worker_Status
work_response(LV2_Handle  instance,
		uint32_t    size,
		const void* data)
{
	FeedbackSuppressor* self = (FeedbackSuppressor*)instance;

	FilterListMessage msg = { { sizeof(FilterList*), self->uris.freeFilterList },
			self->filter_list };

	// Send a message to the worker to free the current filter list
	self->schedule->schedule_work(self->schedule->handle, sizeof(msg), &msg);

	// Install the new filter list
	self->filter_list = *(FilterList*const*)data;

	for(unsigned i = 0; i<self->max_filters; i++){
		self->filter_bank[i].enabled = false;
	}


	// add the required notches from the list to filter bank
#define DEFAULT_Q 50
	int i=0;
	for(; i<self->filter_list->list_len; i++){
		if(i==MAX_BANK_SIZE) break;
		addNotchFilterToBank(self->filter_bank,
				self->filter_list->notch_fcs[i],self->sample_rate,i,50);
	}
	self->bank_index = i;


	// Send a notification that we're using a new filter list.
	lv2_atom_forge_frame_time(&self->forge, 0);
	write_set_file(&self->forge, &self->uris,
			self->filter_list->path,
			self->filter_list->path_len);

	return LV2_WORKER_SUCCESS;
}

static void
connect_port(LV2_Handle instance,
		uint32_t   port,
		void*      data)
{
	FeedbackSuppressor* self = (FeedbackSuppressor*)instance;
	switch (port) {
	case FS_CONTROL:
		self->control_port = (const LV2_Atom_Sequence*)data;
		break;
	case FS_NOTIFY:
		self->notify_port = (LV2_Atom_Sequence*)data;
		break;
	case FS_IN:
		self->input_port = (const float*)data;
		break;
	case FS_OUT:
		self->output_port = (float*)data;
		break;
	default:
		break;
	}
}

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
		double                    rate,
		const char*               path,
		const LV2_Feature* const* features)
{
	// Allocate and initialise instance structure.
	FeedbackSuppressor* self = (FeedbackSuppressor*)malloc(sizeof(FeedbackSuppressor));
	if (!self) {
		return NULL;
	}
	memset(self, 0, sizeof(FeedbackSuppressor));

	// Get host features
	for (int i = 0; features[i]; ++i) {
		if (!strcmp(features[i]->URI, LV2_URID__map)) {
			self->map = (LV2_URID_Map*)features[i]->data;
		} else if (!strcmp(features[i]->URI, LV2_WORKER__schedule)) {
			self->schedule = (LV2_Worker_Schedule*)features[i]->data;
		} else if (!strcmp(features[i]->URI, LV2_LOG__log)) {
			self->log = (LV2_Log_Log*)features[i]->data;
		}
	}
	if (!self->map) {
		lv2_log_error(&self->logger, "Missing feature urid:map\n");
		goto fail;
	} else if (!self->schedule) {
		lv2_log_error(&self->logger, "Missing feature work:schedule\n");
		goto fail;
	}

	self->sample_rate = rate;

	self->max_filters = DEFAULT_BANK_SIZE;

	//allocate some memory for the filter bank
	self->filter_bank = (BiQuadFilter*) malloc(self->max_filters *sizeof(BiQuadFilter));

	for(unsigned i = 0; i<self->max_filters; i++){
		self->filter_bank[i].enabled = false;
	}

	// Map URIs and initialise forge/logger
	map_sampler_uris(self->map, &self->uris);
	lv2_atom_forge_init(&self->forge, self->map);
	lv2_log_logger_init(&self->logger, self->map, self->log);

	//init the FFT stuff
	self->N = DEFAULT_N;
	self->fft_audio_in = (double*) fftw_malloc(sizeof(double) * self->N);
	self->fft_audio_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * self->N);
	self->plan = fftw_plan_dft_r2c_1d(self->N, self->fft_audio_in,
			self->fft_audio_out, FFTW_ESTIMATE);
	self->power_spectrum = (float*) malloc(self->N *sizeof(float));
	self->power_spectrum_db = (float*) malloc(self->N *sizeof(float));

	return (LV2_Handle)self;

	fail:
	free(self);
	return 0;
}

static void
cleanup(LV2_Handle instance)
{
	FeedbackSuppressor* self = (FeedbackSuppressor*)instance;
	free_filter_list(self, self->filter_list);
	free(self->filter_bank);
	fftw_free(self->fft_audio_in); fftw_free(self->fft_audio_out);
    fftw_destroy_plan(self->plan);
	free(self->power_spectrum);
	free(self->power_spectrum_db);
	free(self);
}

static void
run(LV2_Handle instance,
		uint32_t   sample_count)
{
	FeedbackSuppressor*     self        = (FeedbackSuppressor*)instance;
	FeedbackSuppressorURIs* uris        = &self->uris;
	const float* input		= self->input_port;
	float*       output      = self->output_port;

	// Set up forge to write directly to notify output port.
	const uint32_t notify_capacity = self->notify_port->atom.size;
	lv2_atom_forge_set_buffer(&self->forge,
			(uint8_t*)self->notify_port,
			notify_capacity);

	// Start a sequence in the notify output port.
	lv2_atom_forge_sequence_head(&self->forge, &self->notify_frame, 0);


	// Read incoming events
	LV2_ATOM_SEQUENCE_FOREACH(self->control_port, ev) {
		if (is_object_type(uris, ev->body.type)) {
			const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
			if (obj->body.otype == uris->patch_Set) {
				// Received a set message, send it to the worker.
				lv2_log_trace(&self->logger, "Queueing set message\n");
				self->schedule->schedule_work(self->schedule->handle,
						lv2_atom_total_size(&ev->body),
						&ev->body);
			} else {
				lv2_log_trace(&self->logger,
						"Unknown object type %d\n", obj->body.otype);
			}
		}
	}

	// Add zeros to end if sample not long enough (or not playing)
	for (uint32_t pos = 0; pos < sample_count; ++pos) {
		output[pos] = processFilterBank(self->filter_bank,input[pos],self->max_filters);
	}
}

static LV2_State_Status
save(LV2_Handle                instance,
		LV2_State_Store_Function  store,
		LV2_State_Handle          handle,
		uint32_t                  flags,
		const LV2_Feature* const* features)
{
	FeedbackSuppressor* self = (FeedbackSuppressor*)instance;
	if (!self->filter_list) {
		return LV2_STATE_SUCCESS;
	}

	LV2_State_Map_Path* map_path = NULL;
	for (int i = 0; features[i]; ++i) {
		if (!strcmp(features[i]->URI, LV2_STATE__mapPath)) {
			map_path = (LV2_State_Map_Path*)features[i]->data;
		}
	}

	char* apath = map_path->abstract_path(map_path->handle, self->filter_list->path);

	store(handle,
			self->uris.filterList,
			apath,
			strlen(self->filter_list->path) + 1,
			self->uris.atom_Path,
			LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);

	free(apath);

	return LV2_STATE_SUCCESS;
}

static LV2_State_Status
restore(LV2_Handle                  instance,
		LV2_State_Retrieve_Function retrieve,
		LV2_State_Handle            handle,
		uint32_t                    flags,
		const LV2_Feature* const*   features)
{
	FeedbackSuppressor* self = (FeedbackSuppressor*)instance;

	size_t   size;
	uint32_t type;
	uint32_t valflags;

	const void* value = retrieve(
			handle,
			self->uris.filterList,
			&size, &type, &valflags);

	if (value) {
		const char* path = (const char*)value;
		lv2_log_trace(&self->logger, "Restoring file %s\n", path);
		free_filter_list(self, self->filter_list);
		self->filter_list = load_filter_list(self, path);
	}

	return LV2_STATE_SUCCESS;
}

static const void*
extension_data(const char* uri)
{
	static const LV2_State_Interface  state  = { save, restore };
	static const LV2_Worker_Interface worker = { work, work_response, NULL };
	if (!strcmp(uri, LV2_STATE__interface)) {
		return &state;
	} else if (!strcmp(uri, LV2_WORKER__interface)) {
		return &worker;
	}
	return NULL;
}

static const LV2_Descriptor descriptor = {
		FS_URI,
		instantiate,
		connect_port,
		NULL,  // activate,
		run,
		NULL,  // deactivate,
		cleanup,
		extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
	switch (index) {
	case 0:
		return &descriptor;
	default:
		return NULL;
	}
}
