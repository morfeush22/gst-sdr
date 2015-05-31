/*
 * file_src.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "file_src.h"

FileSrc::FileSrc(const char *path):
data_(new AbstractSrcHelpers::Data),
path_(path) {
	FileSrcHelpers::Data *temp = new FileSrcHelpers::Data;
	temp->abstract_src = this;

	data_->src_data = temp;
	data_->other_data = NULL;
}

FileSrc::~FileSrc() {
	delete FILE_SRC_DATA_CAST(data_->src_data);
	delete data_;
}

void FileSrc::SetSrc(void *other_data) {
	data_->other_data = other_data;

	PlayerHelpers::Data *player_data = PLAYER_DATA_CAST(data_->other_data);
	FileSrcHelpers::Data *src_data = FILE_SRC_DATA_CAST(data_->src_data);

	src_data->src = gst_element_factory_make(name(), "src");
	g_assert(src_data->src);

	g_object_set(src_data->src, "location", path_, NULL);

	gst_bin_add_many(GST_BIN(player_data->pipeline),
			src_data->src,
			NULL
			);
}

const char *FileSrc::name() const {
	return "filesrc";
}

void FileSrc::LinkSrc() {
	PlayerHelpers::Data *player_data = PLAYER_DATA_CAST(data_->other_data);
	FileSrcHelpers::Data *src_data = FILE_SRC_DATA_CAST(data_->src_data);

	g_assert(gst_element_link_many(
			src_data->src,
			player_data->iddemux,
			NULL)
	);
}
