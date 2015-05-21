/*
 * PulseSink.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "PulseSink.h"
#include "Player.h"

PulseSink::PulseSink():
queue_(NULL),
sink_(NULL) {
}

PulseSink::~PulseSink() {
}

void PulseSink::InitSink(void *ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;

	char buff[100];
	strcpy(buff, GetName());
	strcat(buff, "_queue");

	queue_ = gst_element_factory_make("queue", buff);
	g_assert(queue_);

	strcpy(buff, GetName());
	strcat(buff, "_sink");

	sink_ = gst_element_factory_make(GetName(), buff);
	g_assert(sink_);

	gst_bin_add_many(GST_BIN(data->pipeline_),
			sink_,
			queue_,
			NULL);

	g_assert(gst_element_link_many(
			data->tee_,
			queue_,
			sink_,
			NULL)
	);
}

const char* PulseSink::GetName() const {
	return "pulsesink";
}

void PulseSink::Finish(void *ptr) {
}
