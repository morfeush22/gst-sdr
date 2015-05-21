/*
 * FileSink.cpp
 *
 *  Created on: May 21, 2015
 *      Author: morfeush22
 */

#include "FileSink.h"
#include "Player.h"

FileSink::FileSink(const char *path): path_(path), queue_(NULL), sink_(NULL) {
}

FileSink::~FileSink() {
}

void FileSink::InitSink(void *ptr) {
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

	g_object_set(sink_, "location", path_, NULL);
}

const char* FileSink::GetName() const {
	return "filesink";
}

void FileSink::Finish(void* data) {
}
