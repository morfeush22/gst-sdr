/*
 * FakeSink.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "FakeSink.h"
#include "Player.h"

static GstPadProbeReturn UnlinkCall(GstPad *pad, GstPadProbeInfo *info, gpointer ptr) {
	Player *player = (Player *)((AbstractSinkHelpers::Data *)ptr)->other_data_;
	FakeSink *sink = (FakeSink *)((AbstractSinkHelpers::Data *)ptr)->sink_;

	GstPad *sinkpad;

	if(!g_atomic_int_compare_and_exchange(&sink->removing_, FALSE, TRUE))
		return GST_PAD_PROBE_OK;

	sinkpad = gst_element_get_static_pad(sink->queue_, "sink");
	gst_pad_unlink(sink->teepad_, sinkpad);
	gst_object_unref(sinkpad);

	gst_bin_remove(GST_BIN(player->pipeline_), sink->queue_);
	gst_bin_remove(GST_BIN(player->pipeline_), sink->sink_);

	gst_element_set_state(sink->sink_, GST_STATE_NULL);
	gst_element_set_state(sink->queue_, GST_STATE_NULL);

	gst_object_unref(sink->sink_);
	gst_object_unref(sink->queue_);

	gst_element_release_request_pad(player->tee_, sink->teepad_);
	gst_object_unref(sink->teepad_);

	return GST_PAD_PROBE_REMOVE;
}

static void SinkHandoffCall(GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer ptr) {
	FakeSink *sink = (FakeSink *)ptr;

	uint32_t size = gst_buffer_get_size(buffer);

	sink->AddBytes(size);
}

FakeSink::FakeSink():
queue_(NULL),
sink_(NULL),
teepad_(NULL),
removing_(false),
bytes_returned_(0) {
}

FakeSink::~FakeSink() {
}

uint32_t FakeSink::GetBytesReturned() {
	return bytes_returned_;
}

void FakeSink::InitSink(void *ptr) {
	Player *player = static_cast<Player *>(ptr);

	GstPad *sinkpad;
	GstPadTemplate *templ;

	templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(player->tee_), "src_%u");
	teepad_ = gst_element_request_pad(player->tee_, templ, NULL, NULL);

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

	gst_bin_add_many(GST_BIN(player->pipeline_),
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

	g_object_set(sink_, "signal-handoffs", TRUE, NULL);

	g_signal_connect(sink_, "handoff", G_CALLBACK(SinkHandoffCall), this);
}

void FakeSink::AddBytes(uint32_t bytes) {
	bytes_returned_ += bytes;
}

const char *FakeSink::GetName() const {
	return "fakesink";
}

void FakeSink::FinishEarly(void *ptr) {
	AbstractSinkHelpers::Data data;

	data.sink_ = this;
	data.other_data_ = ptr;

	gst_pad_add_probe(teepad_, GST_PAD_PROBE_TYPE_IDLE, UnlinkCall, &data, (GDestroyNotify)g_free);
}
