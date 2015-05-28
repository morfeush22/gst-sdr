/*
 * file_src.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "file_src.h"
#include "player.h"

FileSrc::FileSrc(const char *path): path_(path) {
	data_ = new AbstractSrcHelpers::Data;

	data_->src_data = this;
	data_->other_data = NULL;
}

FileSrc::~FileSrc() {
	delete data_;
}

void FileSrc::InitSrc(void *other_data) {
	data_->other_data = other_data;

	PlayerHelpers::Data *data = PLAYER_DATA_CAST(data_->other_data);

	data->src = gst_element_factory_make(get_name(), "src");
	g_assert(data->src);

	g_object_set(data->src, "location", path_, NULL);
}

const char *FileSrc::get_name() {
	return "filesrc";
}
