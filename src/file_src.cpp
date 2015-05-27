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

	data_->src_data_ = this;
	data_->other_data_ = NULL;
}

FileSrc::~FileSrc() {
	delete data_;
}

void FileSrc::InitSrc(void *other_data) {
	data_->other_data_ = other_data;

	PlayerHelpers::Data *data = PLAYER_DATA_CAST(data_->other_data_);

	data->src_ = gst_element_factory_make(GetName(), "src");
	g_assert(data->src_);

	g_object_set(data->src_, "location", path_, NULL);
}

const char *FileSrc::GetName() {
	return "filesrc";
}
