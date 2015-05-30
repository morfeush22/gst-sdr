/*
 * ring_src.cpp
 *
 *  Created on: May 17, 2015
 *      Author: morfeush22
 */

#include "ring_src.h"
#include <gst/app/app.h>

#define BOUND 0.001	//upper/lower bound of playback speed, relative to 1.0
#define RESOLUTION 0.000001	//resolution of pitch increments/decrements

#define APP_SRC_BUFF_PERCENT 100	//percent of appsrc buffer fullness to emit need-data
#define APP_SRC_BUFF_DIVIDER 250	//divider of ring buffer length to obtain appsrc buffer length
#define BUFF_CHUNK_DIVIDER 10000	//divider of ring buffer length to obrain single chunk length

RingSrc::RingSrc(float threshold, size_t buffer_length):
ring_buffer_(new BlockingRingBuffer(buffer_length)),
threshold_(threshold),
current_ratio_(1.0),
data_(new AbstractSrcHelpers::Data) {
	RingSrcHelpers::Data temp = {this, buffer_length, buffer_length/BUFF_CHUNK_DIVIDER};
	RingSrcHelpers::Data *data_ptr = new RingSrcHelpers::Data(temp);

	data_->src_data = data_ptr;
	data_->other_data = NULL;
}

RingSrc::~RingSrc() {
	delete RING_SRC_DATA_CAST(data_->src_data);
	delete data_;
	delete ring_buffer_;
}

const char *RingSrc::name() const {
	return "appsrc";
}

gboolean RingSrcHelpers::ReadData(gpointer container_ptr) {
	AbstractSrcHelpers::Data *container = ABSTRACT_SRC_DATA_CAST(container_ptr);
	PlayerHelpers::Data *data = PLAYER_DATA_CAST(container->other_data);
	RingSrcHelpers::Data *src_data = RING_SRC_DATA_CAST(container->src_data);

	RingSrc *src = RING_SRC_CAST(src_data->abstract_src);

	GstBuffer *buffer;
	GstMapInfo map;
	float *it;
	size_t size;
	GstFlowReturn ret;

	buffer = gst_buffer_new_and_alloc(src_data->buff_chunk_size*sizeof(float));
	gst_buffer_map(buffer, &map, GST_MAP_WRITE);
	it = reinterpret_cast<float *>(map.data);

	size = src->ring_buffer_->ReadFrom(it, src_data->buff_chunk_size);

	gst_buffer_unmap(buffer, &map);

	ret = gst_app_src_push_buffer(GST_APP_SRC(data->src), buffer);

	if(ret !=  GST_FLOW_OK){
		return FALSE;
	}

	if(size != src_data->buff_chunk_size) {
	//if(src->ring_buffer_->last_frame()) {
		gst_app_src_end_of_stream(reinterpret_cast<GstAppSrc *>(data->src));
		return FALSE;
	}

	src->ProcessThreshold(PLAYER_CAST(data->player));

	return TRUE;
}


void RingSrcHelpers::StartFeed(GstElement *pipeline, guint size, gpointer container_ptr) {
	RingSrcHelpers::Data *data = RING_SRC_DATA_CAST(ABSTRACT_SRC_DATA_CAST(container_ptr)->src_data);

	if(data->source_id == 0) {
		data->source_id = g_idle_add(reinterpret_cast<GSourceFunc>(ReadData), container_ptr);
	}
}

void RingSrcHelpers::StopFeed(GstElement *pipeline, gpointer container_ptr) {
	RingSrcHelpers::Data *data = RING_SRC_DATA_CAST(ABSTRACT_SRC_DATA_CAST(container_ptr)->src_data);

	if (data->source_id != 0) {
		g_source_remove(data->source_id);
		data->source_id = 0;
	}
}

void RingSrc::InitSrc(void *other_data) {
	data_->other_data = other_data;

	PlayerHelpers::Data *data = PLAYER_DATA_CAST(data_->other_data);
	RingSrcHelpers::Data *src_data = RING_SRC_DATA_CAST(data_->src_data);

	src_data->source_id = 0;

	data->src = gst_element_factory_make(name(), "src");
	g_assert(data->src);

	g_signal_connect(data->src, "need-data", G_CALLBACK(RingSrcHelpers::StartFeed), data_);
	g_signal_connect(data->src, "enough-data", G_CALLBACK(RingSrcHelpers::StopFeed), data_);

	g_object_set(data->src,
			"max-bytes", src_data->buff_size/APP_SRC_BUFF_DIVIDER,
			"min-percent", APP_SRC_BUFF_PERCENT,
			NULL);
}

float RingSrc::DecrementRatio(Player *player) {
	current_ratio_ -= RESOLUTION;
	if(current_ratio_<(1-BOUND))
		current_ratio_ = 1-BOUND;

	player->set_playback_speed(current_ratio_);

	return current_ratio_;
}

float RingSrc::IncrementRatio(Player *player) {
	current_ratio_ += RESOLUTION;
	if(current_ratio_>(1+BOUND))
		current_ratio_ = 1+BOUND;

	player->set_playback_speed(current_ratio_);

	return current_ratio_;
}

size_t RingSrc::ParseThreshold(float fraction) {
	return static_cast<size_t>(RING_SRC_DATA_CAST(data_->src_data)->buff_size)*fraction;
}

void RingSrc::ProcessThreshold(Player *player) {
	if(player->ready()) {
		float ratio;

		if(ring_buffer_->DataStored()<ParseThreshold(0.5-threshold_)) {
			ratio = DecrementRatio(player);
			g_warning("current ratio: %f\n", ratio);
			return;
		}

		if(ring_buffer_->DataStored()>ParseThreshold(0.5+threshold_)) {
			ratio = IncrementRatio(player);
			g_warning("current ratio: %f\n", ratio);
			return;
		}
	}
}

void RingSrc::Write(float *buffer, size_t length) {
	ring_buffer_->WriteInto(buffer, length);
}

void RingSrc::set_last_frame(bool to) {
	ring_buffer_->set_last_frame(to);
}
