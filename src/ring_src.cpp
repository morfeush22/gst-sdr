/*
 * ring_src.cpp
 *
 *  Created on: May 17, 2015
 *      Author: morfeush22
 */

#include "player.h"
#include "ring_src.h"

#include <gst/app/app.h>

#include <fstream>
#include <unistd.h>

#define RESOLUTION 0.001
#define BUFF_SIZE 100000	//100*4kB (float)
#define APP_SRC_BUFF_SIZE 1000*25	//25kB, size of internal appsrc buffer

RingSrc::RingSrc(float threshold):
threshold_(threshold),
current_ratio_(1.0) {
	data_.abstract_src_ = this;
	data_.ring_buffer_ = new BlockingRingBuffer(BUFF_SIZE);
}

RingSrc::~RingSrc() {
	data_.abstract_src_ = NULL;
	delete data_.ring_buffer_;
}

const char *RingSrc::GetName() {
	return "appsrc";
}

static gboolean ReadData(gpointer container_ptr) {
	AbstractSrcHelpers::Data *container = (AbstractSrcHelpers::Data *)container_ptr;
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)container->other_data_;
	RingSrcHelpers::Data *src_data = (RingSrcHelpers::Data *)container->src_data_;

	GstBuffer *buffer;
	GstMapInfo map;
	float *it;
	gint size;
	GstFlowReturn ret;

	buffer = gst_buffer_new_and_alloc(BUFF_SIZE*sizeof(float));
	gst_buffer_map(buffer, &map, GST_MAP_WRITE);
	it = (float *)map.data;

	size = src_data->ring_buffer_->ReadFrom(it, BUFF_SIZE);

	gst_buffer_unmap(buffer, &map);

	ret = gst_app_src_push_buffer(GST_APP_SRC(data->src_), buffer);

	if(ret !=  GST_FLOW_OK){
		return FALSE;
	}

	if(size != BUFF_SIZE){
		gst_app_src_end_of_stream((GstAppSrc *)data->src_);
		return FALSE;
	}

	((RingSrc *)src_data->abstract_src_)->ProcessThreshold(container);

	return TRUE;
}


static void StartFeed(GstElement *pipeline, guint size, gpointer container_ptr) {
	RingSrcHelpers::Data *data = (RingSrcHelpers::Data *)((AbstractSrcHelpers::Data *)container_ptr)->src_data_;

	if(data->source_id_ == 0) {
		data->source_id_ = g_idle_add((GSourceFunc)ReadData, container_ptr);
	}
}

static void StopFeed(GstElement *pipeline, gpointer container_ptr) {
	RingSrcHelpers::Data *data = (RingSrcHelpers::Data *)((AbstractSrcHelpers::Data *)container_ptr)->src_data_;

	if (data->source_id_ != 0) {
		g_source_remove(data->source_id_);
		data->source_id_ = 0;
	}
}

void RingSrc::InitSrc(AbstractSrcHelpers::Data *ptr) {
	data_.source_id_ = 0;

	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr->other_data_;

	data->src_ = gst_element_factory_make(GetName(), "src");
	g_assert(data->src_);

	ptr->src_data_ = &data_;

	g_signal_connect(data->src_, "need-data", G_CALLBACK(StartFeed), ptr);
	g_signal_connect(data->src_, "enough-data", G_CALLBACK(StopFeed), ptr);

	gst_app_src_set_max_bytes(GST_APP_SRC(data->src_), APP_SRC_BUFF_SIZE);
}

float RingSrc::DecrementRatio(void *player_ptr) {
	current_ratio_ -= RESOLUTION;
	if(current_ratio_<(1-threshold_))
		current_ratio_ = 1-threshold_;

	Player *player = static_cast<Player *>(player_ptr);
	player->SetPlaybackSpeed(current_ratio_);

	return current_ratio_;
}

float RingSrc::IncrementRatio(void *player_ptr) {
	current_ratio_ += RESOLUTION;
	if(current_ratio_>(1+threshold_))
		current_ratio_ = 1+threshold_;

	Player *player = static_cast<Player *>(player_ptr);
	player->SetPlaybackSpeed(current_ratio_);

	return current_ratio_;
}

size_t RingSrc::ParseThreshold(float fraction) {
	return size_t(BUFF_SIZE)*fraction;
}

void RingSrc::ProcessThreshold(AbstractSrcHelpers::Data *ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr->other_data_;
	RingSrcHelpers::Data *src_data = (RingSrcHelpers::Data *)ptr->src_data_;

	g_print("current: %lu - size: %d - lesser: %lu - upper: %lu - read: %lu\n", src_data->ring_buffer_->DataStored(), BUFF_SIZE, ParseThreshold(0.5-threshold_), ParseThreshold(0.5+threshold_), ParseThreshold(0.25));

	if(data->ready_) {
		float ratio;

		if(src_data->ring_buffer_->DataStored()<ParseThreshold(0.5-threshold_)) {
			ratio = DecrementRatio(data->player_);
			g_warning("current ratio: %f\n", ratio);
			return;
		}

		if(src_data->ring_buffer_->DataStored()>ParseThreshold(0.5+threshold_)) {
			ratio = IncrementRatio(data->player_);
			g_warning("current ratio: %f\n", ratio);
			return;
		}
	}
}

void RingSrc::Write(float *buffer, size_t length) {
	data_.ring_buffer_->WriteInto(buffer, length);
}

void RingSrc::LastFrame() {
	data_.ring_buffer_->LastFrame();
}
