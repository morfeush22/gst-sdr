/*
 * ring_src.cpp
 *
 *  Created on: May 17, 2015
 *      Author: morfeush22
 */

#include "player.h"
#include "ring_src.h"
#include <gst/app/app.h>

#define RESOLUTION 0.000001	//resolution of pitch increments/decrements
#define BOUND 0.001	//upper/lower bound of playback speed, relative to 1.0

#define BUFF_CHUNK 200	//0.2kB
#define BUFF_SIZE (BUFF_CHUNK*10000)	//2MB, should be multiple of BUFF_CHUNK, size of ring buffer
#define APP_SRC_BUFF_SIZE 2000	//2.0kB, size of internal appsrc buffer
#define APP_SRC_BUFF_PERCENT 100	//percent of appsrc buffer fullness to emit need-data

RingSrc::RingSrc(float threshold):
threshold_(threshold),
current_ratio_(1.0) {
	data_ = new AbstractSrcHelpers::Data;

	RingSrcHelpers::Data *temp = new RingSrcHelpers::Data;
	temp->abstract_src = this;
	temp->ring_buffer = new BlockingRingBuffer(BUFF_SIZE);

	data_->src_data = temp;
	data_->other_data = NULL;
}

RingSrc::~RingSrc() {
	delete RING_SRC_DATA_CAST(data_->src_data)->ring_buffer;
	delete RING_SRC_DATA_CAST(data_->src_data);
	delete data_;
}

const char *RingSrc::name() const {
	return "appsrc";
}

static gboolean ReadData(gpointer container_ptr) {
	AbstractSrcHelpers::Data *container = ABSTRACT_SRC_DATA_CAST(container_ptr);
	PlayerHelpers::Data *data = PLAYER_DATA_CAST(container->other_data);
	RingSrcHelpers::Data *src_data = RING_SRC_DATA_CAST(container->src_data);

	GstBuffer *buffer;
	GstMapInfo map;
	char *it;
	gint size;
	GstFlowReturn ret;

	buffer = gst_buffer_new_and_alloc(BUFF_CHUNK);
	gst_buffer_map(buffer, &map, GST_MAP_WRITE);
	it = reinterpret_cast<char *>(map.data);

	size = src_data->ring_buffer->ReadFrom(it, BUFF_CHUNK);

	gst_buffer_unmap(buffer, &map);

	ret = gst_app_src_push_buffer(GST_APP_SRC(data->src), buffer);

	if(ret !=  GST_FLOW_OK){
		return FALSE;
	}

	if(size != BUFF_CHUNK){
		gst_app_src_end_of_stream(reinterpret_cast<GstAppSrc *>(data->src));
		return FALSE;
	}

	RING_SRC_CAST(src_data->abstract_src)->ProcessThreshold(container);

	return TRUE;
}


static void StartFeed(GstElement *pipeline, guint size, gpointer container_ptr) {
	RingSrcHelpers::Data *data = RING_SRC_DATA_CAST(ABSTRACT_SRC_DATA_CAST(container_ptr)->src_data);

	if(data->source_id == 0) {
		data->source_id = g_idle_add(reinterpret_cast<GSourceFunc>(ReadData), container_ptr);
	}
}

static void StopFeed(GstElement *pipeline, gpointer container_ptr) {
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

	g_signal_connect(data->src, "need-data", G_CALLBACK(StartFeed), data_);
	g_signal_connect(data->src, "enough-data", G_CALLBACK(StopFeed), data_);

	g_object_set(data->src,
			"max-bytes", APP_SRC_BUFF_SIZE,
			"min-percent", APP_SRC_BUFF_PERCENT,
			NULL);
}

float RingSrc::DecrementRatio(void *player_ptr) {
	current_ratio_ -= RESOLUTION;
	if(current_ratio_<(1-BOUND))
		current_ratio_ = 1-BOUND;

	Player *player = static_cast<Player *>(player_ptr);
	player->set_playback_speed(current_ratio_);

	return current_ratio_;
}

float RingSrc::IncrementRatio(void *player_ptr) {
	current_ratio_ += RESOLUTION;
	if(current_ratio_>(1+BOUND))
		current_ratio_ = 1+BOUND;

	Player *player = static_cast<Player *>(player_ptr);
	player->set_playback_speed(current_ratio_);

	return current_ratio_;
}

size_t RingSrc::ParseThreshold(float fraction) {
	return static_cast<size_t>(BUFF_SIZE)*fraction;
}

void RingSrc::ProcessThreshold(AbstractSrcHelpers::Data *ptr) {
	PlayerHelpers::Data *data = PLAYER_DATA_CAST(ptr->other_data);
	RingSrcHelpers::Data *src_data = RING_SRC_DATA_CAST(ptr->src_data);

	g_print("current: %lu - size: %d - lesser: %lu - upper: %lu\n", src_data->ring_buffer->DataStored(), BUFF_SIZE, ParseThreshold(0.5-threshold_), ParseThreshold(0.5+threshold_));

	if(data->ready) {
		float ratio;

		if(src_data->ring_buffer->DataStored()<ParseThreshold(0.5-threshold_)) {
			ratio = DecrementRatio(data->player);
			g_warning("current ratio: %f\n", ratio);
			return;
		}

		if(src_data->ring_buffer->DataStored()>ParseThreshold(0.5+threshold_)) {
			ratio = IncrementRatio(data->player);
			g_warning("current ratio: %f\n", ratio);
			return;
		}
	}
}

void RingSrc::Write(char *buffer, size_t length) {
	RING_SRC_DATA_CAST(data_->src_data)->ring_buffer->WriteInto(buffer, length);
}

void RingSrc::set_last_frame(bool to) {
	RING_SRC_DATA_CAST(data_->src_data)->ring_buffer->set_last_frame(to);
}
