/*
 * pulse_sink.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "pulse_sink.h"
#include "player.h"

static GstPadProbeReturn UnlinkCall(GstPad *pad, GstPadProbeInfo *info, gpointer ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)((AbstractSinkHelpers::Data *)ptr)->other_data_;
	PulseSinkHelpers::Data *sink_data = (PulseSinkHelpers::Data *)((AbstractSinkHelpers::Data *)ptr)->sink_data_;

	GstPad *sinkpad;

	if(!g_atomic_int_compare_and_exchange(&sink_data->removing_, FALSE, TRUE))
		return GST_PAD_PROBE_OK;

	sinkpad = gst_element_get_static_pad(sink_data->queue_, "sink");
	gst_pad_unlink(sink_data->teepad_, sinkpad);
	gst_object_unref(sinkpad);

	gst_bin_remove(GST_BIN(data->pipeline_), sink_data->queue_);
	gst_bin_remove(GST_BIN(data->pipeline_), sink_data->sink_);

	gst_element_set_state(sink_data->sink_, GST_STATE_NULL);
	gst_element_set_state(sink_data->queue_, GST_STATE_NULL);

	gst_object_unref(sink_data->sink_);
	gst_object_unref(sink_data->queue_);

	gst_element_release_request_pad(data->tee_, sink_data->teepad_);
	gst_object_unref(sink_data->teepad_);

	((PulseSink *)sink_data->abstract_sink_)->Finish((AbstractSinkHelpers::Data *)ptr);

	return GST_PAD_PROBE_REMOVE;
}

PulseSink::PulseSink():
linked_(false) {
	data_.removing_ = FALSE;
}

PulseSink::~PulseSink() {
}

void PulseSink::InitSink(AbstractSinkHelpers::Data *ptr) {
	if(IsLinked())
		return;

	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr->other_data_;

	GstPad *sinkpad;
	GstPadTemplate *templ;

	templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(data->tee_), "src_%u");
	data_.teepad_ = gst_element_request_pad(data->tee_, templ, NULL, NULL);

	char buff[100];

	strcpy(buff, GetName());
	strcat(buff, "_queue");

	data_.queue_ = gst_element_factory_make("queue", buff);
	g_assert(data_.queue_);

	strcpy(buff, GetName());
	strcat(buff, "_sink");

	data_.sink_ = gst_element_factory_make(GetName(), buff);
	g_assert(data_.sink_);

	data_.removing_ = false;

	gst_object_ref(data_.queue_);
	gst_object_ref(data_.sink_);

	gst_bin_add_many(GST_BIN(data->pipeline_),
			data_.queue_,
			data_.sink_,
			NULL);

	g_assert(gst_element_link_many(
			data_.queue_,
			data_.sink_,
			NULL)
	);

	gst_element_sync_state_with_parent(data_.queue_);
	gst_element_sync_state_with_parent(data_.sink_);

	sinkpad = gst_element_get_static_pad(data_.queue_, "sink");
	gst_pad_link(data_.teepad_, sinkpad);
	gst_object_unref(sinkpad);

	linked_ = true;
}

const char *PulseSink::GetName() const {
	return "pulsesink";
}

void PulseSink::Finish(AbstractSinkHelpers::Data *ptr) {
	//check if exists

	PulseSinkHelpers::Data *sink_data = (PulseSinkHelpers::Data *)ptr->sink_data_;

	gst_pad_add_probe(sink_data->teepad_, GST_PAD_PROBE_TYPE_IDLE, UnlinkCall, ptr, (GDestroyNotify)g_free);
}

bool PulseSink::IsLinked() const {
	return linked_;
}
