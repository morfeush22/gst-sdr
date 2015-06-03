/*
 * ring_src.cpp
 *
 *  Created on: May 17, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#include "ring_src.h"
#include <gst/app/app.h>

#define BOUND 0.001	//upper/lower bound of playback speed, relative to 1.0
#define RESOLUTION 0.000001	//resolution of pitch increments/decrements

#define APP_SRC_BUFF_PERCENT 100	//percent of appsrc buffer fullness to emit need-data
#define APP_SRC_BUFF_DIVIDER 250	//divider of ring buffer length to obtain appsrc buffer length
#define BUFF_CHUNK_DIVIDER 10000	//divider of ring buffer length to obrain single chunk length

RingSrc::RingSrc(float threshold, size_t buffer_length):
data_(new AbstractSrcHelpers::Data),
threshold_(threshold),
current_ratio_(1.0),
buff_size_(buffer_length),
buff_chunk_size_(buffer_length/BUFF_CHUNK_DIVIDER),
ring_buffer_(new BlockingRingBuffer(buffer_length)) {
	RingSrcHelpers::Data *temp = new RingSrcHelpers::Data;
	temp->abstract_src = this;

	data_->src_data = temp;
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

gboolean RingSrcHelpers::ReadData(gpointer src_data_ptr) {
	RingSrcHelpers::Data *src_data = RING_SRC_DATA_CAST(src_data_ptr);
	RingSrc *a_src = RING_SRC_CAST(src_data->abstract_src);
	GstElement *curr_pipe = GST_ELEMENT_PARENT(src_data->src);

	GstBuffer *buffer;
	GstMapInfo map;
	uint8_t *it;
	size_t size;
	GstFlowReturn ret;

	gboolean paused = FALSE;

	buffer = gst_buffer_new_and_alloc(a_src->buff_chunk_size_);
	gst_buffer_map(buffer, &map, GST_MAP_WRITE);
	it = reinterpret_cast<uint8_t *>(map.data);

	if(!a_src->ring_buffer_->DataStored()) {
		g_assert(gst_element_set_state(curr_pipe, GST_STATE_PAUSED));
		paused = TRUE;
	}

	size = a_src->ring_buffer_->ReadFrom(it, a_src->buff_chunk_size_);

	if(paused == TRUE)
		g_assert(gst_element_set_state(curr_pipe, GST_STATE_PLAYING));

	gst_buffer_unmap(buffer, &map);

	ret = gst_app_src_push_buffer(GST_APP_SRC(src_data->src), buffer);

	if(ret !=  GST_FLOW_OK){
		return FALSE;
	}

	//if(size != src->buff_chunk_size_) {	//sth wrong with this one, with ring buffer
	if(a_src->ring_buffer_->last_frame()) {
		gst_app_src_end_of_stream(reinterpret_cast<GstAppSrc *>(src_data->src));
		return FALSE;
	}

	a_src->ProcessThreshold();

	return TRUE;
}


void RingSrcHelpers::StartFeed(GstElement *pipeline, guint size, gpointer src_data_ptr) {
	RingSrcHelpers::Data *src_data = RING_SRC_DATA_CAST(src_data_ptr);

	if(src_data->source_id == 0) {
		src_data->source_id = g_idle_add(reinterpret_cast<GSourceFunc>(ReadData), src_data_ptr);
	}
}

void RingSrc::LinkSrc() {
	PlayerHelpers::Data *player_data = PLAYER_DATA_CAST(data_->other_data);
	RingSrcHelpers::Data *src_data = RING_SRC_DATA_CAST(data_->src_data);

	g_assert(gst_element_link_many(
			src_data->src,
			player_data->iddemux,
			NULL)
	);
}

void RingSrcHelpers::StopFeed(GstElement *pipeline, gpointer src_data_ptr) {
	RingSrcHelpers::Data *src_data = RING_SRC_DATA_CAST(src_data_ptr);

	if (src_data->source_id != 0) {
		g_source_remove(src_data->source_id);
		src_data->source_id = 0;
	}
}

void RingSrc::SetSrc(void *other_data) {
	data_->other_data = other_data;

	PlayerHelpers::Data *player_data = PLAYER_DATA_CAST(data_->other_data);
	RingSrcHelpers::Data *src_data = RING_SRC_DATA_CAST(data_->src_data);

	src_data->src = gst_element_factory_make(name(), "src");
	g_assert(src_data->src);
	src_data->source_id = 0;

	g_signal_connect(src_data->src, "need-data", G_CALLBACK(RingSrcHelpers::StartFeed), data_->src_data);
	g_signal_connect(src_data->src, "enough-data", G_CALLBACK(RingSrcHelpers::StopFeed), data_->src_data);

	g_object_set(src_data->src,
			"max-bytes", buff_size_/APP_SRC_BUFF_DIVIDER,
			"min-percent", APP_SRC_BUFF_PERCENT,
			NULL);

	gst_bin_add_many(GST_BIN(player_data->pipeline),
			src_data->src,
			NULL
			);
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
	return buff_size_*fraction;
}

void RingSrc::ProcessThreshold() {
	Player *player = PLAYER_CAST(PLAYER_DATA_CAST(data_->other_data)->player);

	if(player->ready()) {
		float ratio;

		if(ring_buffer_->DataStored()<ParseThreshold(0.5-threshold_)) {
			ratio = DecrementRatio(player);
			//g_warning("current ratio: %f\n", ratio);
			return;
		}

		if(ring_buffer_->DataStored()>ParseThreshold(0.5+threshold_)) {
			ratio = IncrementRatio(player);
			//g_warning("current ratio: %f\n", ratio);
			return;
		}
	}
}

void RingSrc::Write(uint8_t *buffer, size_t length) {
	ring_buffer_->WriteInto(buffer, length);
}

void RingSrc::set_last_frame(bool to) {
	ring_buffer_->set_last_frame(to);
}
