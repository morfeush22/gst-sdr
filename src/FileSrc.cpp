/*
 * FileSrc.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "FileSrc.h"
#include "Player.h"

FileSrc::FileSrc(const char *path): path_(path) {
}

FileSrc::~FileSrc() {
}

void FileSrc::InitSrc(void *ptr) {
	Player *player = static_cast<Player *>(ptr);

	player->src_ = gst_element_factory_make(GetName(), "src");
	g_assert(player->src_);

	g_object_set(player->src_, "location", path_, NULL);
}

const char *FileSrc::GetName() {
	return "filesrc";
}
