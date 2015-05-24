/*
 * ring_src.cpp
 *
 *  Created on: May 17, 2015
 *      Author: morfeush22
 */

#include "ring_src.h"

#include <gst/app/app.h>

#include <fstream>
#include <unistd.h>
#include "player.h"

#define RESOLUTION 0.001
#define BUFF_SIZE 100000	//100*4kB (float)
#define APP_SRC_BUFF_SIZE 1000*25	//25kB, size of internal appsrc buffer

RingSrc::RingSrc(float threshold):
source_id_(0),
threshold_(threshold),
current_ratio_(1.0) {
	ring_buffer_ = new BlockingRingBuffer(BUFF_SIZE);
}

RingSrc::~RingSrc() {
	delete ring_buffer_;
}

const char *RingSrc::GetName() {
	return "appsrc";
}

static gboolean ReadData(gpointer *ptr) {
	Player *player = (Player *)ptr;
	RingSrc *src = (RingSrc *)player->GetSrc();

	GstBuffer *buffer;
	GstMapInfo map;
	float *it;
	gint size;

	GstFlowReturn ret;

	buffer = gst_buffer_new_and_alloc(BUFF_SIZE*sizeof(float));
	gst_buffer_map(buffer, &map, GST_MAP_WRITE);
	it = (float *)map.data;

	size = src->GetRingBuffer()->ReadFrom(it, BUFF_SIZE);

	gst_buffer_unmap(buffer, &map);

	ret = gst_app_src_push_buffer(GST_APP_SRC(player->src_), buffer);

	if(ret !=  GST_FLOW_OK){
		return FALSE;
	}

	if(size != BUFF_SIZE){
		gst_app_src_end_of_stream((GstAppSrc *)player->src_);
		return FALSE;
	}

	src->ProcessThreshold(ptr);

	return TRUE;
}


static void StartFeed(GstElement *pipeline, guint size, gpointer *ptr) {
	Player *player = (Player *)ptr;
	RingSrc *src = (RingSrc *)player->GetSrc();

	if(src->source_id_ == 0) {
		src->source_id_ = g_idle_add((GSourceFunc)ReadData, player);
	}
}

static void StopFeed(GstElement *pipeline, gpointer *ptr) {
	Player *player = (Player *)ptr;
	RingSrc *src = (RingSrc *)player->GetSrc();

	if (src->source_id_ != 0) {
		g_source_remove(src->source_id_);
		src->source_id_ = 0;
	}
}

void RingSrc::InitSrc(void *ptr) {
	Player *player = static_cast<Player *>(ptr);

	player->src_ = gst_element_factory_make(GetName(), "src");
	g_assert(player->src_);

	g_signal_connect(player->src_, "need-data", G_CALLBACK(StartFeed), player);
	g_signal_connect(player->src_, "enough-data", G_CALLBACK(StopFeed), player);

	gst_app_src_set_max_bytes(GST_APP_SRC(player->src_), APP_SRC_BUFF_SIZE);
}

float RingSrc::DecrementRatio(void *ptr) {
	current_ratio_ -= RESOLUTION;
	if(current_ratio_<(1-threshold_))
		current_ratio_ = 1-threshold_;

	Player *player = static_cast<Player *>(ptr);
	player->SetPlaybackSpeed(current_ratio_);

	return current_ratio_;
}

float RingSrc::IncrementRatio(void *ptr) {
	current_ratio_ += RESOLUTION;
	if(current_ratio_>(1+threshold_))
		current_ratio_ = 1+threshold_;

	Player *player = static_cast<Player *>(ptr);
	player->SetPlaybackSpeed(current_ratio_);

	return current_ratio_;
}

size_t RingSrc::ParseThreshold(float fraction) {
	return size_t(BUFF_SIZE)*fraction;
}

void RingSrc::ProcessThreshold(void *ptr) {
	Player *player = static_cast<Player *>(ptr);

	//g_print("current: %lu - size: %d - lesser: %lu - upper: %lu - read: %lu\n", ring_buffer_->DataStored(), BUFF_SIZE, ParseThreshold(0.5-threshold_), ParseThreshold(0.5+threshold_), ParseThreshold(0.25));

	if(player->ready_) {
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

BlockingRingBuffer *RingSrc::GetRingBuffer() {
	return ring_buffer_;
}
