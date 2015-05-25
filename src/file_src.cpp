/*
 * file_src.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "file_src.h"
#include "player.h"

FileSrc::FileSrc(const char *path): path_(path) {
}

FileSrc::~FileSrc() {
}

void FileSrc::InitSrc(AbstractSrcHelpers::Data *ptr) {
	PlayerHelpers::Data *data = static_cast<PlayerHelpers::Data *>(ptr->other_data_);

	data->src_ = gst_element_factory_make(GetName(), "src");
	g_assert(data->src_);

	g_object_set(data->src_, "location", path_, NULL);
}

const char *FileSrc::GetName() {
	return "filesrc";
}
