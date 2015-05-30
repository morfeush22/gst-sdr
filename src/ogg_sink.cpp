/*
 * ogg_sink.cpp
 *
 *  Created on: May 29, 2015
 *      Author: morfeush22
 */

#include "ogg_sink.h"
#include "player.h"

GstPadProbeReturn OggSinkHelpers::UnlinkCall(GstPad *pad, GstPadProbeInfo *info, gpointer container_ptr) {
	AbstractSinkHelpers::Data *container = ABSTRACT_SINK_DATA_CAST(container_ptr);
	PlayerHelpers::Data *data = PLAYER_DATA_CAST(container->other_data);
	OggSinkHelpers::Data *sink_data = OGG_SINK_DATA_CAST(container->sink_data);

	GstPad *sinkpad;

	if(!g_atomic_int_compare_and_exchange(&sink_data->removing, FALSE, TRUE))
		return GST_PAD_PROBE_OK;

	sinkpad = gst_element_get_static_pad(sink_data->queue, "sink");
	gst_pad_unlink(sink_data->teepad, sinkpad);
	gst_object_unref(sinkpad);

	gst_bin_remove(GST_BIN(data->pipeline), sink_data->queue);
	gst_bin_remove(GST_BIN(data->pipeline), sink_data->encoder);
	gst_bin_remove(GST_BIN(data->pipeline), sink_data->muxer);
	gst_bin_remove(GST_BIN(data->pipeline), sink_data->sink);

	gst_element_set_state(sink_data->sink, GST_STATE_NULL);
	gst_element_set_state(sink_data->muxer, GST_STATE_NULL);
	gst_element_set_state(sink_data->encoder, GST_STATE_NULL);
	gst_element_set_state(sink_data->queue, GST_STATE_NULL);

	gst_object_unref(sink_data->sink);
	gst_object_unref(sink_data->muxer);
	gst_object_unref(sink_data->encoder);
	gst_object_unref(sink_data->queue);

	gst_element_release_request_pad(data->tee, sink_data->teepad);
	gst_object_unref(sink_data->teepad);

	sink_data->linked = false;

	return GST_PAD_PROBE_REMOVE;
}

OggSink::OggSink(const char *path):
path_(path),
data_(new AbstractSinkHelpers::Data) {
	OggSinkHelpers::Data *temp = new OggSinkHelpers::Data;
	temp->abstract_sink = this;
	temp->linked = false;

	data_->sink_data = temp;
	data_->other_data = NULL;
}

OggSink::~OggSink() {
	delete OGG_SINK_DATA_CAST(data_->sink_data);
	delete data_;
}

void OggSink::InitSink(void *other_data) {
	if(linked())
		return;

	data_->other_data = other_data;

	PlayerHelpers::Data *data = PLAYER_DATA_CAST(data_->other_data);
	OggSinkHelpers::Data *sink_data = OGG_SINK_DATA_CAST(data_->sink_data);

	GstPad *sinkpad;
	GstPadTemplate *templ;

	templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(data->tee), "src_%u");
	sink_data->teepad = gst_element_request_pad(data->tee, templ, NULL, NULL);

	char buff[100];

	strcpy(buff, name());
	strcat(buff, "_queue");

	sink_data->queue = gst_element_factory_make("queue", buff);
	g_assert(sink_data->queue);

	strcpy(buff, name());
	strcat(buff, "_encoder");

	sink_data->encoder = gst_element_factory_make("vorbisenc", buff);
	g_assert(sink_data->encoder);

	strcpy(buff, name());
	strcat(buff, "_muxer");

	sink_data->muxer = gst_element_factory_make("oggmux", buff);
	g_assert(sink_data->muxer);

	strcpy(buff, name());
	strcat(buff, "_sink");

	sink_data->sink = gst_element_factory_make("filesink", buff);
	g_assert(sink_data->sink);

	g_object_set(sink_data->sink, "location", path_, NULL);

	sink_data->removing = FALSE;

	gst_object_ref(sink_data->queue);
	gst_object_ref(sink_data->encoder);
	gst_object_ref(sink_data->muxer);
	gst_object_ref(sink_data->sink);

	gst_bin_add_many(GST_BIN(data->pipeline),
			sink_data->queue,
			sink_data->encoder,
			sink_data->muxer,
			sink_data->sink,
			NULL);

	g_assert(gst_element_link_many(
			sink_data->queue,
			sink_data->encoder,
			sink_data->muxer,
			sink_data->sink,
			NULL)
	);

	gst_element_sync_state_with_parent(sink_data->queue);
	gst_element_sync_state_with_parent(sink_data->encoder);
	gst_element_sync_state_with_parent(sink_data->muxer);
	gst_element_sync_state_with_parent(sink_data->sink);

	sinkpad = gst_element_get_static_pad(sink_data->queue, "sink");
	gst_pad_link(sink_data->teepad, sinkpad);
	gst_object_unref(sinkpad);

	sink_data->linked = true;
}

const char *OggSink::name() const {
	return "oggsink";
}

void OggSink::Finish() {
	if(!linked()) {
		return;
	}

	gst_pad_add_probe(OGG_SINK_DATA_CAST(data_->sink_data)->teepad, GST_PAD_PROBE_TYPE_IDLE, OggSinkHelpers::UnlinkCall, data_, NULL);
}

bool OggSink::linked() const {
	return OGG_SINK_DATA_CAST(data_->sink_data)->linked;
}
