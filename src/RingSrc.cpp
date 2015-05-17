/*
 * RingSrc.cpp
 *
 *  Created on: May 17, 2015
 *      Author: morfeush22
 */

#include "RingSrc.h"
#include "Player.h"
#include <gst/app/app.h>

#include <fstream>

#define BUFF_SIZE 4096

RingSrc::RingSrc(const char *path):
source_id_(0) {
	//for now, read from file
	std::ifstream in_file(path, std::ifstream::binary);
	if(in_file.is_open()) {
		in_file.seekg(0, std::ios::end);
		size_ = in_file.tellg();
		in_file.seekg(0, std::ios::beg);

		buff_ = new char[size_];

		in_file.read(buff_, size_);

		in_file.close();
	}
}

RingSrc::~RingSrc() {
	delete[] buff_;
}

const char *RingSrc::GetName() {
	return "appsrc";
}

static gboolean ReadData(PlayerHelpers::Data *ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;
	Player *player = (Player *)data->player_;
	RingSrc *src = (RingSrc *)player->GetSrc();

	GstBuffer *buffer;
	char *it;
	gint size;
	GstFlowReturn ret;
	GstMapInfo map;

	buffer = gst_buffer_new_and_alloc(BUFF_SIZE);
	gst_buffer_map(buffer, &map, GST_MAP_WRITE);
	it = (char *)map.data;

	if(src->GetSize() >= BUFF_SIZE)
		size = BUFF_SIZE;
	else
		size = src->GetSize();

	memcpy(it, src->GetBuff(), size);

	gst_buffer_unmap(buffer, &map);

	src->SetSize(size);
	src->SetBuff(size);

	ret = gst_app_src_push_buffer(GST_APP_SRC(data->src_), buffer);


	if(ret !=  GST_FLOW_OK){
		return FALSE;
	}

	if(size != BUFF_SIZE){
		gst_app_src_end_of_stream((GstAppSrc *)data->src_);
		return FALSE;
	}

	return TRUE;
}


static void StartFeed(GstElement *pipeline, guint size, PlayerHelpers::Data *ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;
	Player *player = (Player *)data->player_;
	RingSrc *src = (RingSrc *)player->GetSrc();
	guint source_id = src->GetSourceId();

	if (source_id == 0) {
		source_id = g_idle_add((GSourceFunc)ReadData, data);
	}
}

static void StopFeed(GstElement *pipeline, PlayerHelpers::Data *ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;
	Player *player = (Player *)data->player_;
	RingSrc *src = (RingSrc *)player->GetSrc();
	guint source_id = src->GetSourceId();

	if (source_id != 0) {
		g_source_remove(source_id);
		source_id = 0;
	}
}

void RingSrc::InitSrc(void *ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;

	g_signal_connect(data->src_, "need-data", G_CALLBACK(StartFeed), data);
	g_signal_connect(data->src_, "enough-data", G_CALLBACK(StopFeed), data);
}

uint32_t RingSrc::GetSize() {
	return size_;
}

void RingSrc::SetSize(uint32_t to_size) {
	size_ -= to_size;
}

char *RingSrc::GetBuff() {
	return buff_;
}

void RingSrc::SetBuff(uint32_t point_to) {
	buff_ += point_to;
}

uint32_t RingSrc::DecrementPlaybackSpeed(GstElement *src, uint32_t current_speed) {
	GstCaps *src_caps;
	gchar *text;

	current_speed += 1;
	text = g_strdup_printf(AUDIO_CAPS, current_speed);

	src_caps = gst_caps_from_string(text);

	g_object_set(G_OBJECT(src), "caps", src_caps, NULL);

	gst_caps_unref(src_caps);
	g_free(text);

	return current_speed;
}

uint32_t RingSrc::IncrementPlaybackSpeed(GstElement *src, uint32_t current_speed) {
	GstCaps *src_caps;
	gchar *text;

	current_speed -= 1;
	text = g_strdup_printf(AUDIO_CAPS, current_speed);

	src_caps = gst_caps_from_string(text);

	g_object_set(G_OBJECT(src), "caps", src_caps, NULL);

	gst_caps_unref(src_caps);
	g_free(text);

	return current_speed;
}

guint &RingSrc::GetSourceId() {
	return source_id_;
}
