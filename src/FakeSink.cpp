/*
 * FakeSink.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "FakeSink.h"
#include "Player.h"

static void SinkHandoffCall(GstElement *fakesink,
GstBuffer *buffer,
GstPad *pad,
gpointer ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;
	Player *player = (Player *)data->player_;
	uint32_t size = gst_buffer_get_size(buffer);

	AbstractSink *sink = player->GetSink();
	static_cast<FakeSink *>(sink)->AddBytes(size);
}

FakeSink::FakeSink(): bytes_returned_(0) {
}

FakeSink::~FakeSink() {
}

uint32_t FakeSink::GetBytesReturned() {
	return bytes_returned_;
}

void FakeSink::InitSink(void *ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;

	g_object_set(data->sink_, "signal-handoffs", TRUE, NULL);

	g_signal_connect(data->sink_, "handoff", G_CALLBACK(SinkHandoffCall), data);
}

void FakeSink::AddBytes(uint32_t bytes) {
	bytes_returned_ += bytes;
}

const char *FakeSink::GetName() {
	return "fakesink";
}
