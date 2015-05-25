/*
 * fake_sink.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "fake_sink.h"
#include "player.h"

static GstPadProbeReturn UnlinkCall(GstPad *pad, GstPadProbeInfo *info, gpointer ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)((AbstractSinkHelpers::Data *)ptr)->other_data_;
	FakeSinkHelpers::Data *sink_data = (FakeSinkHelpers::Data *)((AbstractSinkHelpers::Data *)ptr)->sink_data_;

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

	sink_data->abstract_sink_->Finish((AbstractSinkHelpers::Data *)ptr);

	return GST_PAD_PROBE_REMOVE;
}

static void SinkHandoffCall(GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer ptr) {
	FakeSink *sink = (FakeSink *)ptr;

	uint32_t size = gst_buffer_get_size(buffer);

	sink->AddBytes(size);
}

FakeSink::FakeSink():
bytes_returned_(0),
linked_(false) {
	data_.removing_ = FALSE;
}

FakeSink::~FakeSink() {
}

uint32_t FakeSink::GetBytesReturned() {
	return bytes_returned_;
}

void FakeSink::InitSink(AbstractSinkHelpers::Data *ptr) {
	if(IsLinked())
		return;

	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr->other_data_;
	FakeSinkHelpers::Data *sink_data = (FakeSinkHelpers::Data *)ptr->sink_data_;

	GstPad *sinkpad;
	GstPadTemplate *templ;

	templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(data->tee_), "src_%u");
	sink_data->teepad_ = gst_element_request_pad(data->tee_, templ, NULL, NULL);

	char buff[100];

	strcpy(buff, GetName());
	strcat(buff, "_queue");

	sink_data->queue_ = gst_element_factory_make("queue", buff);
	g_assert(sink_data->queue_);

	strcpy(buff, GetName());
	strcat(buff, "_sink");

	sink_data->sink_ = gst_element_factory_make(GetName(), buff);
	g_assert(sink_data->sink_);

	g_object_set(sink_data->sink_, "signal-handoffs", TRUE, NULL);
	g_signal_connect(sink_data->sink_, "handoff", G_CALLBACK(SinkHandoffCall), this);

	sink_data->removing_ = false;

	gst_object_ref(sink_data->queue_);
	gst_object_ref(sink_data->sink_);

	gst_bin_add_many(GST_BIN(data->pipeline_),
			sink_data->queue_,
			sink_data->sink_,
			NULL);

	g_assert(gst_element_link_many(
			sink_data->queue_,
			sink_data->sink_,
			NULL)
	);

	gst_element_sync_state_with_parent(sink_data->queue_);
	gst_element_sync_state_with_parent(sink_data->sink_);

	sinkpad = gst_element_get_static_pad(sink_data->queue_, "sink");
	gst_pad_link(sink_data->teepad_, sinkpad);
	gst_object_unref(sinkpad);

	linked_ = true;
}

void FakeSink::AddBytes(uint32_t bytes) {
	bytes_returned_ += bytes;
}

const char *FakeSink::GetName() const {
	return "fakesink";
}

void FakeSink::Finish(AbstractSinkHelpers::Data *ptr) {
	//check if exists

	FakeSinkHelpers::Data *sink_data = (FakeSinkHelpers::Data *)ptr->sink_data_;

	gst_pad_add_probe(sink_data->teepad_, GST_PAD_PROBE_TYPE_IDLE, UnlinkCall, ptr, (GDestroyNotify)g_free);
}

bool FakeSink::IsLinked() const {
	return linked_;
}
