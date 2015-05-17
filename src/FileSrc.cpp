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
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;

	g_object_set(data->src_, "location", path_, NULL);
}

const char *FileSrc::GetName() {
	return "filesrc";
}
