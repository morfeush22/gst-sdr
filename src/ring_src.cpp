/*
 * ring_src.cpp
 *
 *  Created on: May 17, 2015
 *      Author: morfeush22
 */

#include "player.h"
#include "ring_src.h"
#include <gst/app/app.h>

#define RESOLUTION 0.001
#define BUFF_SIZE 960000	//960kB
#define BUFF_CHUNK 9600	//9.6kB
#define APP_SRC_BUFF_SIZE 25000	//25kB, size of internal appsrc buffer

RingSrc::RingSrc(float threshold):
threshold_(threshold),
current_ratio_(1.0) {
	data_ = new AbstractSrcHelpers::Data;

	RingSrcHelpers::Data *temp = new RingSrcHelpers::Data;
	temp->abstract_src_ = this;
	temp->ring_buffer_ = new BlockingRingBuffer(BUFF_SIZE);

	data_->src_data_ = temp;
	data_->other_data_ = NULL;
}

RingSrc::~RingSrc() {
	delete RING_SRC_DATA_CAST(data_->src_data_)->ring_buffer_;
	delete RING_SRC_DATA_CAST(data_->src_data_);
	delete data_;
}

const char *RingSrc::GetName() {
	return "appsrc";
}

static gboolean ReadData(gpointer container_ptr) {
	AbstractSrcHelpers::Data *container = ABSTRACT_SRC_DATA_CAST(container_ptr);
	PlayerHelpers::Data *data = PLAYER_DATA_CAST(container->other_data_);
	RingSrcHelpers::Data *src_data = RING_SRC_DATA_CAST(container->src_data_);

	GstBuffer *buffer;
	GstMapInfo map;
	char *it;
	gint size;
	GstFlowReturn ret;

	buffer = gst_buffer_new_and_alloc(BUFF_CHUNK);
	gst_buffer_map(buffer, &map, GST_MAP_WRITE);
	it = (char *)map.data;

	size = src_data->ring_buffer_->ReadFrom(it, BUFF_CHUNK);

	gst_buffer_unmap(buffer, &map);

	ret = gst_app_src_push_buffer(GST_APP_SRC(data->src_), buffer);

	if(ret !=  GST_FLOW_OK){
		return FALSE;
	}

	if(size != BUFF_CHUNK){
		gst_app_src_end_of_stream((GstAppSrc *)data->src_);
		return FALSE;
	}

	RING_DATA_CAST(src_data->abstract_src_)->ProcessThreshold(container);

	return TRUE;
}


static void StartFeed(GstElement *pipeline, guint size, gpointer container_ptr) {
	RingSrcHelpers::Data *data = RING_SRC_DATA_CAST(ABSTRACT_SRC_DATA_CAST(container_ptr)->src_data_);

	if(data->source_id_ == 0) {
		data->source_id_ = g_idle_add((GSourceFunc)ReadData, container_ptr);
	}
}

static void StopFeed(GstElement *pipeline, gpointer container_ptr) {
	RingSrcHelpers::Data *data = RING_SRC_DATA_CAST(ABSTRACT_SRC_DATA_CAST(container_ptr)->src_data_);

	if (data->source_id_ != 0) {
		g_source_remove(data->source_id_);
		data->source_id_ = 0;
	}
}

void RingSrc::InitSrc(void *other_data) {
	data_->other_data_ = other_data;

	PlayerHelpers::Data *data = PLAYER_DATA_CAST(data_->other_data_);
	RingSrcHelpers::Data *src_data = RING_SRC_DATA_CAST(data_->src_data_);

	src_data->source_id_ = 0;

	data->src_ = gst_element_factory_make(GetName(), "src");
	g_assert(data->src_);

	g_signal_connect(data->src_, "need-data", G_CALLBACK(StartFeed), data_);
	g_signal_connect(data->src_, "enough-data", G_CALLBACK(StopFeed), data_);

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
	PlayerHelpers::Data *data = PLAYER_DATA_CAST(ptr->other_data_);
	RingSrcHelpers::Data *src_data = RING_SRC_DATA_CAST(ptr->src_data_);

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

void RingSrc::Write(char *buffer, size_t length) {
	RING_SRC_DATA_CAST(data_->src_data_)->ring_buffer_->WriteInto(buffer, length);
}

void RingSrc::LastFrame() {
	RING_SRC_DATA_CAST(data_->src_data_)->ring_buffer_->LastFrame();
}
