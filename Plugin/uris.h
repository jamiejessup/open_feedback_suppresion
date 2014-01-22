/*
 * Author: Jamie Jessup 2013
 *         jessup.jamie@gmail.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef FS_URIS_H
#define FS_URIS_H

#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"

#define FS_URI          "http://jamiejessup.com/open_feedback_suppressor"
#define FS__filterList      FS_URI "#filterList"
#define FS__applyFilterList FS_URI "#applyFilterList"
#define FS__freeFilterList  FS_URI "#freeFilterList"
#define FS__fftExecute  FS_URI "#fftExecute"
#define FS__detectionResults  FS_URI "#detectionResults"

typedef struct {
	LV2_URID atom_Blank;
	LV2_URID atom_Path;
	LV2_URID atom_Resource;
	LV2_URID atom_Sequence;
	LV2_URID atom_URID;
	LV2_URID atom_eventTransfer;
	LV2_URID applyFilterList;
	LV2_URID filterList;
	LV2_URID freeFilterList;
	LV2_URID fftExecute;
	LV2_URID detectionResults;
	LV2_URID midi_Event;
	LV2_URID patch_Set;
	LV2_URID patch_property;
	LV2_URID patch_value;
} FeedbackSuppressorURIs;

static inline void
map_sampler_uris(LV2_URID_Map* map, FeedbackSuppressorURIs* uris)
{
	uris->atom_Blank         = map->map(map->handle, LV2_ATOM__Blank);
	uris->atom_Path          = map->map(map->handle, LV2_ATOM__Path);
	uris->atom_Resource      = map->map(map->handle, LV2_ATOM__Resource);
	uris->atom_Sequence      = map->map(map->handle, LV2_ATOM__Sequence);
	uris->atom_URID          = map->map(map->handle, LV2_ATOM__URID);
	uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);
	uris->applyFilterList     = map->map(map->handle, FS__applyFilterList);
	uris->freeFilterList      = map->map(map->handle, FS__freeFilterList);
	uris->filterList          = map->map(map->handle, FS__filterList);
	uris->fftExecute			= map->map(map->handle, FS__fftExecute);
	uris->detectionResults			= map->map(map->handle, FS__detectionResults);
	uris->midi_Event         = map->map(map->handle, LV2_MIDI__MidiEvent);
	uris->patch_Set          = map->map(map->handle, LV2_PATCH__Set);
	uris->patch_property     = map->map(map->handle, LV2_PATCH__property);
	uris->patch_value        = map->map(map->handle, LV2_PATCH__value);
}

static inline bool
is_object_type(const FeedbackSuppressorURIs* uris, LV2_URID type)
{
	return type == uris->atom_Resource
		|| type == uris->atom_Blank;
}

/**
 * Write a message like the following to @p forge:
 * []
 *     a patch:Set ;
 *     patch:property eg:sample ;
 *     patch:value </home/me/foo.wav> .
 */
static inline LV2_Atom*
write_set_file(LV2_Atom_Forge*    forge,
               const FeedbackSuppressorURIs* uris,
               const char*        filename,
               const size_t       filename_len)
{
	LV2_Atom_Forge_Frame frame;
	LV2_Atom* set = (LV2_Atom*)lv2_atom_forge_blank(
		forge, &frame, 1, uris->patch_Set);

	lv2_atom_forge_property_head(forge, uris->patch_property, 0);
	lv2_atom_forge_urid(forge, uris->filterList);
	lv2_atom_forge_property_head(forge, uris->patch_value, 0);
	lv2_atom_forge_path(forge, filename, filename_len);

	lv2_atom_forge_pop(forge, &frame);

	return set;
}

/**
 * Get the file path from a message like:
 * []
 *     a patch:Set ;
 *     patch:property eg:sample ;
 *     patch:value </home/me/foo.wav> .
 */
static inline const LV2_Atom*
read_set_file(const FeedbackSuppressorURIs*     uris,
              const LV2_Atom_Object* obj)
{
	if (obj->body.otype != uris->patch_Set) {
		fprintf(stderr, "Ignoring unknown message type %d\n", obj->body.otype);
		return NULL;
	}

	/* Get property URI. */
	const LV2_Atom* property = NULL;
	lv2_atom_object_get(obj, uris->patch_property, &property, 0);
	if (!property) {
		fprintf(stderr, "Malformed set message has no body.\n");
		return NULL;
	} else if (property->type != uris->atom_URID) {
		fprintf(stderr, "Malformed set message has non-URID property.\n");
		return NULL;
	} else if (((const LV2_Atom_URID*)property)->body != uris->filterList) {
		fprintf(stderr, "Set message for unknown property.\n");
		return NULL;
	}

	/* Get value. */
	const LV2_Atom* file_path = NULL;
	lv2_atom_object_get(obj, uris->patch_value, &file_path, 0);
	if (!file_path) {
		fprintf(stderr, "Malformed set message has no value.\n");
		return NULL;
	} else if (file_path->type != uris->atom_Path) {
		fprintf(stderr, "Set message value is not a Path.\n");
		return NULL;
	}

	return file_path;
}

#endif  /* FS_URIS_H */
