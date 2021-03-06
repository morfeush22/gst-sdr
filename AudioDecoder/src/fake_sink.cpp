/*
 * fake_sink.cpp
 *
 *  Created on: May 13, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#include "fake_sink.h"
#include "player.h"
#include <stdio.h>

uint16_t FakeSink::count_ = 0;

GstPadProbeReturn FakeSinkHelpers::UnlinkCallFakeSink(GstPad *pad, GstPadProbeInfo *info, gpointer container_ptr) {
	AbstractSinkHelpers::Data *container = ABSTRACT_SINK_DATA_CAST(container_ptr);
	PlayerHelpers::Data *player_data = PLAYER_DATA_CAST(container->other_data);
	FakeSinkHelpers::Data *sink_data = FAKE_SINK_DATA_CAST(container->sink_data);

	GstPad *sinkpad;

	if(!g_atomic_int_compare_and_exchange(&sink_data->removing, FALSE, TRUE))
		return GST_PAD_PROBE_OK;

	sinkpad = gst_element_get_static_pad(sink_data->queue, "sink");
	gst_pad_unlink(sink_data->teepad, sinkpad);
	gst_object_unref(sinkpad);

	gst_bin_remove(GST_BIN(player_data->pipeline), sink_data->queue);
	gst_bin_remove(GST_BIN(player_data->pipeline), sink_data->sink);

	gst_element_set_state(sink_data->sink, GST_STATE_NULL);
	gst_element_set_state(sink_data->queue, GST_STATE_NULL);

	gst_object_unref(sink_data->sink);
	gst_object_unref(sink_data->queue);

	gst_element_release_request_pad(player_data->tee, sink_data->teepad);
	gst_object_unref(sink_data->teepad);

	sink_data->linked = false;
	FAKE_SINK_CAST(sink_data->abstract_sink)->count_--;

	return GST_PAD_PROBE_REMOVE;
}

void SinkHandoffCall(GstElement *fakesink, GstBuffer *buffer, GstPad *pad, gpointer sink_ptr) {
	FakeSink *sink = FAKE_SINK_CAST(sink_ptr);

	uint32_t size = gst_buffer_get_size(buffer);

	sink->AddBytes(size);
}

FakeSink::FakeSink():
data_(new AbstractSinkHelpers::Data),
bytes_returned_(0) {
	FakeSinkHelpers::Data *temp = new FakeSinkHelpers::Data;
	temp->abstract_sink = this;
	temp->linked = false;

	data_->sink_data = temp;
	data_->other_data = NULL;
}

FakeSink::~FakeSink() {
	FakeSinkHelpers::Data *sink_data = FAKE_SINK_DATA_CAST(data_->sink_data);

	delete sink_data;
	delete data_;
}

uint32_t FakeSink::bytes_returned() const {
	return bytes_returned_;
}

void FakeSink::InitSink(void *other_data) {
	if(linked())
		return;

	count_ ++;

	data_->other_data = other_data;

	PlayerHelpers::Data *player_data = PLAYER_DATA_CAST(data_->other_data);
	FakeSinkHelpers::Data *sink_data = FAKE_SINK_DATA_CAST(data_->sink_data);

	GstPad *sinkpad;
	GstPadTemplate *templ;

	templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(player_data->tee), "src_%u");
	sink_data->teepad = gst_element_request_pad(player_data->tee, templ, NULL, NULL);

	char buff[100];
	char count_buff[3];

	sprintf(count_buff, "%d", count_);
	count_buff[2] = '\0';

	strcpy(buff, name());
	strcat(buff, "_queue_");
	strcat(buff, count_buff);

	sink_data->queue = gst_element_factory_make("queue", buff);
	g_assert(sink_data->queue);

	strcpy(buff, name());
	strcat(buff, "_sink_");
	strcat(buff, count_buff);

	sink_data->sink = gst_element_factory_make(name(), buff);
	g_assert(sink_data->sink);

	g_object_set(sink_data->sink, "signal-handoffs", TRUE, NULL);
	g_signal_connect(sink_data->sink, "handoff", G_CALLBACK(SinkHandoffCall), this);

	sink_data->removing = false;

	gst_object_ref(sink_data->queue);
	gst_object_ref(sink_data->sink);

	gst_bin_add_many(GST_BIN(player_data->pipeline),
			sink_data->queue,
			sink_data->sink,
			NULL);

	g_assert(gst_element_link_many(
			sink_data->queue,
			sink_data->sink,
			NULL)
	);

	gst_element_sync_state_with_parent(sink_data->queue);
	gst_element_sync_state_with_parent(sink_data->sink);

	sinkpad = gst_element_get_static_pad(sink_data->queue, "sink");
	gst_pad_link(sink_data->teepad, sinkpad);
	gst_object_unref(sinkpad);

	sink_data->linked = true;
}

void FakeSink::AddBytes(uint32_t bytes) {
	bytes_returned_ += bytes;
}

const char *FakeSink::name() const {
	return "fakesink";
}

void FakeSink::Finish() {
	if(!linked()) {
		return;
	}

	gst_pad_add_probe(FAKE_SINK_DATA_CAST(data_->sink_data)->teepad, GST_PAD_PROBE_TYPE_IDLE, FakeSinkHelpers::UnlinkCallFakeSink, data_, NULL);
}

bool FakeSink::linked() const {
	return FAKE_SINK_DATA_CAST(data_->sink_data)->linked;
}

uint32_t FakeSink::num_src_pads() const {
	gint num;
	g_object_get(G_OBJECT(PLAYER_DATA_CAST(data_->other_data)->tee), "num-src-pads", &num, NULL);
	return num;
}

float FakeSink::playback_speed() const {
	gfloat num;
	g_object_get(G_OBJECT(PLAYER_DATA_CAST(data_->other_data)->pitch), "tempo", &num, NULL);
	return num;
}
