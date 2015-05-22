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
#include <unistd.h>

#define RESOLUTION 0.001

#define BUFF_SIZE 1000	//1kB
#define MULTIPLIER 100

#define HOW_MANY 1
#define FILE_CHUNKS 0.5

#define APP_SRC_BUFF_SIZE 1000*25	//25kB

RingSrc::RingSrc(const char *path, float threshold):
source_id_(0),
threshold_(threshold),
path_(path),
current_ratio_(1.0) {
	file_wrapper_ = new FileWrapper(path_, FILE_CHUNKS*BUFF_SIZE*MULTIPLIER);	//500kB
	ring_buffer_ = new RingBuffer<char>(HOW_MANY*BUFF_SIZE*MULTIPLIER);	//100kB
}

RingSrc::~RingSrc() {
	delete file_wrapper_;
	delete ring_buffer_;
}

const char *RingSrc::GetName() {
	return "appsrc";
}

static gboolean ReadData(PlayerHelpers::Data *ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;
	Player *player = (Player *)data->player_;
	RingSrc *src = (RingSrc *)player->GetSrc();

	src->ProcessThreshold(ptr);

	GstBuffer *buffer;
	char *it;
	gint size;
	GstFlowReturn ret;
	GstMapInfo map;

	buffer = gst_buffer_new_and_alloc(BUFF_SIZE);
	gst_buffer_map(buffer, &map, GST_MAP_WRITE);
	it = (char *)map.data;

	size = src->GetRingBuffer()->sReadFrom(it, BUFF_SIZE);

	gst_buffer_unmap(buffer, &map);

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
	guint *source_id = src->GetSourceId();

	//g_print("start feed... %lu\n", src->GetRingBuffer()->DataStored());

	if (*source_id == 0) {
		*source_id = g_idle_add((GSourceFunc)ReadData, ptr);
	}
}

static void StopFeed(GstElement *pipeline, PlayerHelpers::Data *ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;
	Player *player = (Player *)data->player_;
	RingSrc *src = (RingSrc *)player->GetSrc();
	guint *source_id = src->GetSourceId();

	//g_print("stop feed... %lu\n", src->GetRingBuffer()->DataStored());

	if (*source_id != 0) {
		g_source_remove(*source_id);
		*source_id = 0;
	}
}

void RingSrc::InitSrc(void *ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;

	data->src_ = gst_element_factory_make(GetName(), "src");
	g_assert(data->src_);

	g_signal_connect(data->src_, "need-data", G_CALLBACK(StartFeed), data);
	g_signal_connect(data->src_, "enough-data", G_CALLBACK(StopFeed), data);

	gst_app_src_set_max_bytes(GST_APP_SRC(data->src_), APP_SRC_BUFF_SIZE);
}

float RingSrc::DecrementRatio(void *ptr) {
	current_ratio_ -= RESOLUTION;
	if(current_ratio_<(1-threshold_))
		current_ratio_ = 1-threshold_;

	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;
	Player *player = (Player *)data->player_;
	player->SetPlaybackSpeed(current_ratio_);

	return current_ratio_;
}

float RingSrc::IncrementRatio(void *ptr) {
	current_ratio_ += RESOLUTION;
	if(current_ratio_>(1+threshold_))
		current_ratio_ = 1+threshold_;

	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;
	Player *player = (Player *)data->player_;
	player->SetPlaybackSpeed(current_ratio_);

	return current_ratio_;
}

guint *RingSrc::GetSourceId() {
	return &source_id_;
}

size_t RingSrc::ReadFromFile() {
	const char *const *ptr = file_wrapper_->GetCurrentChunkPointer();

	uint32_t returned = file_wrapper_->GetNextChunk();

	//g_print("sleeping...\n");
	//sleep(3);
	//g_print("wakeing...\n");

	return ring_buffer_->sWriteInto(const_cast<char *>(*ptr), returned);
}

size_t RingSrc::ParseThreshold(float percent) {
	return size_t(HOW_MANY*BUFF_SIZE*MULTIPLIER)*percent;
}

void RingSrc::ProcessThreshold(void *ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;

	g_print("current: %lu - size: %d - lesser: %lu - upper: %lu - read: %lu\n", ring_buffer_->DataStored(), (HOW_MANY*BUFF_SIZE*MULTIPLIER), ParseThreshold(0.5-threshold_), ParseThreshold(0.5+threshold_), ParseThreshold(0.25));

	if(ring_buffer_->DataStored()<ParseThreshold(0.25)) {
		ReadFromFile();
		ProcessThreshold(ptr);
		return;
	}

	if(data->ready_) {
		float ratio;

		if(ring_buffer_->DataStored()<ParseThreshold(0.5-threshold_)) {
			ratio = DecrementRatio(ptr);
			g_warning("current ratio: %f\n", ratio);
			return;
		}

		if(ring_buffer_->DataStored()>ParseThreshold(0.5+threshold_)) {
			ratio = IncrementRatio(ptr);
			g_warning("current ratio: %f\n", ratio);
			return;
		}
	}
}

RingBuffer<char> *RingSrc::GetRingBuffer() {
	return ring_buffer_;
}
