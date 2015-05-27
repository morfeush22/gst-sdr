/*
 * fake_sink.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "fake_sink.h"
#include "player.h"

static GstPadProbeReturn UnlinkCall(GstPad *pad, GstPadProbeInfo *info, gpointer container_ptr) {
	AbstractSinkHelpers::Data *container = ABSTRACT_SINK_DATA_CAST(container_ptr);
	PlayerHelpers::Data *data = PLAYER_DATA_CAST(container->other_data_);
	FakeSinkHelpers::Data *sink_data = FAKE_SINK_DATA_CAST(container->sink_data_);

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

	FAKE_SINK_CAST(sink_data->abstract_sink_)->Finish();

	return GST_PAD_PROBE_REMOVE;
}

static void SinkHandoffCall(GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer sink_ptr) {
	FakeSink *sink = FAKE_SINK_CAST(sink_ptr);

	uint32_t size = gst_buffer_get_size(buffer);

	sink->AddBytes(size);
}

FakeSink::FakeSink():
bytes_returned_(0),
linked_(false) {
	data_ = new AbstractSinkHelpers::Data;

	FakeSinkHelpers::Data *temp = new FakeSinkHelpers::Data;
	temp->abstract_sink_ = this;

	data_->sink_data_ = temp;
	data_->other_data_ = NULL;
}

FakeSink::~FakeSink() {
	delete FAKE_SINK_DATA_CAST(data_->sink_data_);
	delete data_;
}

uint32_t FakeSink::GetBytesReturned() {
	return bytes_returned_;
}

void FakeSink::InitSink(void *other_data) {
	if(IsLinked())
		return;

	data_->other_data_ = other_data;

	PlayerHelpers::Data *data = PLAYER_DATA_CAST(data_->other_data_);
	FakeSinkHelpers::Data *sink_data = FAKE_SINK_DATA_CAST(data_->sink_data_);

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

void FakeSink::Finish() {
	gboolean connected = gst_object_has_ancestor(reinterpret_cast<GstObject *>(FAKE_SINK_DATA_CAST(data_->sink_data_)->sink_),
			reinterpret_cast<GstObject *>(PLAYER_DATA_CAST(data_->other_data_)->pipeline_));

	if(IsLinked() && !connected) {
		linked_ = false;
		return;
	}

	gst_pad_add_probe(FAKE_SINK_DATA_CAST(data_->sink_data_)->teepad_, GST_PAD_PROBE_TYPE_IDLE, UnlinkCall, &data_, NULL);
}

bool FakeSink::IsLinked() const {
	return linked_;
}
