/*
 * PulseSink.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "PulseSink.h"
#include "Player.h"

static GstPadProbeReturn UnlinkCall(GstPad *pad, GstPadProbeInfo *info, gpointer ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)((AbstractSinkHelpers::Data *)ptr)->other_data_;
	PulseSink *sink = (PulseSink *)((AbstractSinkHelpers::Data *)ptr)->sink_;

	GstPad *sinkpad;

	if(!g_atomic_int_compare_and_exchange(&sink->removing_, FALSE, TRUE))
		return GST_PAD_PROBE_OK;

	sinkpad = gst_element_get_static_pad(sink->queue_, "sink");
	gst_pad_unlink(sink->teepad_, sinkpad);
	gst_object_unref(sinkpad);

	gst_bin_remove(GST_BIN(data->pipeline_), sink->queue_);
	gst_bin_remove(GST_BIN(data->pipeline_), sink->sink_);

	gst_element_set_state(sink->sink_, GST_STATE_NULL);
	gst_element_set_state(sink->queue_, GST_STATE_NULL);

	gst_object_unref(sink->sink_);
	gst_object_unref(sink->queue_);

	gst_element_release_request_pad(data->tee_, sink->teepad_);
	gst_object_unref(sink->teepad_);

	return GST_PAD_PROBE_REMOVE;
}

PulseSink::PulseSink():
queue_(NULL),
sink_(NULL),
teepad_(NULL),
removing_(false) {
}

PulseSink::~PulseSink() {
}

void PulseSink::InitSink(void *ptr) {
	PlayerHelpers::Data *data = static_cast<PlayerHelpers::Data *>(ptr);

	GstPad *sinkpad;
	GstPadTemplate *templ;

	templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(data->tee_), "src_%u");
	teepad_ = gst_element_request_pad(data->tee_, templ, NULL, NULL);

	char buff[100];

	strcpy(buff, GetName());
	strcat(buff, "_queue");

	queue_ = gst_element_factory_make("queue", buff);
	g_assert(queue_);

	strcpy(buff, GetName());
	strcat(buff, "_sink");

	sink_ = gst_element_factory_make(GetName(), buff);
	g_assert(sink_);

	removing_ = false;

	gst_bin_add_many(GST_BIN(data->pipeline_),
			queue_,
			sink_,
			NULL);

	g_assert(gst_element_link_many(
			queue_,
			sink_,
			NULL)
	);

	gst_element_sync_state_with_parent(queue_);
	gst_element_sync_state_with_parent(sink_);

	sinkpad = gst_element_get_static_pad(queue_, "sink");
	gst_pad_link(teepad_, sinkpad);
	gst_object_unref(sinkpad);
}

const char* PulseSink::GetName() const {
	return "pulsesink";
}

void PulseSink::Finish(void *ptr) {
	AbstractSinkHelpers::Data data;

	data.sink_ = this;
	data.other_data_ = ptr;

	gst_pad_add_probe(teepad_, GST_PAD_PROBE_TYPE_IDLE, UnlinkCall, &data, (GDestroyNotify)g_free);
}

