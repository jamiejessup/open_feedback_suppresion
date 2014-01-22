/*
This file is part of the Open Feedback Suppression LV2 Plugin
Copyright 2014, Jamie Jessup

Open Feedback Suppression is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Open Feedback Suppression is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Ardour Scene Manager. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>

#include <gtk/gtk.h>

#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#include "./uris.h"

#define FS_UI_URI "http://jamiejessup.com/open_feedback_suppressor#ui"

typedef struct {
	LV2_Atom_Forge forge;

	LV2_URID_Map* map;
	FeedbackSuppressorURIs   uris;

	LV2UI_Write_Function write;
	LV2UI_Controller     controller;

	GtkWidget* box;
	GtkWidget* button;
	GtkWidget* label;
} FeedbackSuppressorUI;

static void
on_load_clicked(GtkWidget* widget,
                void*      handle)
{
	FeedbackSuppressorUI* ui = (FeedbackSuppressorUI*)handle;

	/* Create a dialog to select a sample file. */
	GtkWidget* dialog = gtk_file_chooser_dialog_new(
		"Load Sample",
		NULL,
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);

	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Filter Lists");
	gtk_file_filter_add_pattern(filter,"*.ofs");
	gtk_file_chooser_add_filter((GtkFileChooser *) dialog,filter);

	/* Run the dialog, and return if it is cancelled. */
	if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT) {
		gtk_widget_destroy(dialog);
		return;
	}

	/* Get the file path from the dialog. */
	char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

	/* Got what we need, destroy the dialog and filter. */
	gtk_widget_destroy(dialog);

#define OBJ_BUF_SIZE 1024
	uint8_t obj_buf[OBJ_BUF_SIZE];
	lv2_atom_forge_set_buffer(&ui->forge, obj_buf, OBJ_BUF_SIZE);

	LV2_Atom* msg = write_set_file(&ui->forge, &ui->uris,
	                               filename, strlen(filename));

	ui->write(ui->controller, 0, lv2_atom_total_size(msg),
	          ui->uris.atom_eventTransfer,
	          msg);

	g_free(filename);
}

static LV2UI_Handle
instantiate(const LV2UI_Descriptor*   descriptor,
            const char*               plugin_uri,
            const char*               bundle_path,
            LV2UI_Write_Function      write_function,
            LV2UI_Controller          controller,
            LV2UI_Widget*             widget,
            const LV2_Feature* const* features)
{
	FeedbackSuppressorUI* ui = (FeedbackSuppressorUI*)malloc(sizeof(FeedbackSuppressorUI));
	ui->map        = NULL;
	ui->write      = write_function;
	ui->controller = controller;
	ui->box        = NULL;
	ui->button     = NULL;
	ui->label      = NULL;

	*widget = NULL;

	for (int i = 0; features[i]; ++i) {
		if (!strcmp(features[i]->URI, LV2_URID_URI "#map")) {
			ui->map = (LV2_URID_Map*)features[i]->data;
		}
	}

	if (!ui->map) {
		fprintf(stderr, "feedback_suppressor_ui: Host does not support urid:Map\n");
		free(ui);
		return NULL;
	}

	map_sampler_uris(ui->map, &ui->uris);

	lv2_atom_forge_init(&ui->forge, ui->map);

	ui->box = gtk_vbox_new(FALSE, 4);
	ui->label = gtk_label_new("Select Filter List");
	ui->button = gtk_button_new_with_label("Load Filter List");
	gtk_box_pack_start(GTK_BOX(ui->box), ui->label, TRUE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(ui->box), ui->button, FALSE, FALSE, 4);
	g_signal_connect(ui->button, "clicked",
	                 G_CALLBACK(on_load_clicked),
	                 ui);

	*widget = ui->box;

	return ui;
}

static void
cleanup(LV2UI_Handle handle)
{
	FeedbackSuppressorUI* ui = (FeedbackSuppressorUI*)handle;
	gtk_widget_destroy(ui->button);
	free(ui);
}

static void
port_event(LV2UI_Handle handle,
           uint32_t     port_index,
           uint32_t     buffer_size,
           uint32_t     format,
           const void*  buffer)
{
	FeedbackSuppressorUI* ui = (FeedbackSuppressorUI*)handle;
	if (format == ui->uris.atom_eventTransfer) {
		const LV2_Atom* atom = (const LV2_Atom*)buffer;
		if (atom->type == ui->uris.atom_Blank) {
			const LV2_Atom_Object* obj      = (const LV2_Atom_Object*)atom;
			const LV2_Atom*        file_uri = read_set_file(&ui->uris, obj);
			if (!file_uri) {
				fprintf(stderr, "Unknown message sent to UI.\n");
				return;
			}

			const char* uri = (const char*)LV2_ATOM_BODY_CONST(file_uri);
			gtk_label_set_text(GTK_LABEL(ui->label), uri);
		} else {
			fprintf(stderr, "Unknown message type.\n");
		}
	} else {
		fprintf(stderr, "Unknown format.\n");
	}
}

static const void*
extension_data(const char* uri)
{
	return NULL;
}

static const LV2UI_Descriptor descriptor = {
	FS_UI_URI,
	instantiate,
	cleanup,
	port_event,
	extension_data
};

LV2_SYMBOL_EXPORT
const LV2UI_Descriptor*
lv2ui_descriptor(uint32_t index)
{
	switch (index) {
	case 0:
		return &descriptor;
	default:
		return NULL;
	}
}
