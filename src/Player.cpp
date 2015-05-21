/*
 * Player.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "Player.h"
#include <stdio.h>

static void SaveTags(const GstTagList *list, const gchar *tag, gpointer ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;
	Player *player = (Player *)data->player_;
	std::map<const char *, char *, PlayerHelpers::CmpStr>::iterator it;

	gint count = gst_tag_list_get_tag_size(list, tag);

	for(gint i = 0; i < count; i++) {
		if((it = player->GetTagsMap()->find(gst_tag_get_nick(tag))) != player->GetTagsMap()->end()) {
			if(gst_tag_get_type(tag) == G_TYPE_STRING)
				gst_tag_list_get_string_index(list, tag, i, &it->second);
			else {
				 GstDateTime *time;

				 gst_tag_list_get_date_time_index(list, tag, i, &time);
				 gint year = gst_date_time_get_year(time);
				 char *char_year = new char[5];
				 sprintf(char_year, "%d", year);
				 char_year[4] = '\0';
				 it->second = char_year;

				 gst_date_time_unref(time);
			}
		}
	}
}

static gboolean BusCall(GstBus *bus, GstMessage *message, gpointer ptr) {
	PlayerHelpers::Data *data = (PlayerHelpers::Data *)ptr;

	switch(GST_MESSAGE_TYPE(message)) {

	case GST_MESSAGE_ERROR: {
		gchar *debug;
		GError *err;

		gst_message_parse_error(message, &err, &debug);
		g_print("GStreamer error: %s\n", err->message);
		g_error_free(err);

		g_free(debug);
		g_main_loop_quit(data->loop_);
		break;
	}

	case GST_MESSAGE_WARNING: {
		gchar *debug;
		GError *err;

		gst_message_parse_warning(message, &err, &debug);
		g_print("GStreamer warning: %s\n", err->message);
		g_error_free(err);

		g_free(debug);
		break;
	}

	case GST_MESSAGE_ASYNC_DONE: {
		g_print ("GStreamer: prerolled, lock'n'load\n");
		data->ready_ = TRUE;
		break;
	}

	case GST_MESSAGE_TAG: {
		GstTagList *tags = NULL;

		gst_message_parse_tag(message, &tags);

		gst_tag_list_foreach(tags, SaveTags, data);

		gst_tag_list_free (tags);
		break;
	}

	case GST_MESSAGE_EOS:
		g_main_loop_quit(data->loop_);
		break;

	default:
		break;
	}

	return TRUE;
}

Player::Player(AbstractSrc *src, uint32_t sample_rate):
sample_rate_(sample_rate),
src_(src) {
	gst_init (NULL, NULL);
	data_.player_ = this;
	Init();
}

Player::~Player() {

}

void Player::Process() {
	gst_element_set_state((GstElement*)data_.pipeline_, GST_STATE_PLAYING);
	data_.loop_ = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(data_.loop_);
}

void Player::ConstructObjects() {
	data_.pipeline_ = gst_pipeline_new("player");

	data_.iddemux_ = gst_element_factory_make("id3demux", "tag_demuxer");
	data_.decoder_ = gst_element_factory_make("faad", "decoder");
	data_.parser_ = gst_element_factory_make("aacparse", "parser");

	data_.pitch_ = gst_element_factory_make("pitch", "pitch");
	data_.converter_ = gst_element_factory_make("audioconvert", "converter");

	data_.tee_ = gst_element_factory_make("tee", "tee");

	g_assert(data_.pipeline_
			&& data_.iddemux_
			&& data_.decoder_
			&& data_.parser_
			&& data_.pitch_
			&& data_.converter_
			&& data_.tee_);	//check if all elements created
}

void Player::SetPropeties() {
	GstBus *bus;

	src_->InitSrc(static_cast<void *>(&data_));

	gst_bin_add_many(GST_BIN(data_.pipeline_),
			data_.src_,
			data_.iddemux_,
			data_.decoder_,
			data_.parser_,
			data_.pitch_,
			data_.converter_,
			data_.tee_,
			NULL);	//add elements to bin

	bus = gst_element_get_bus(data_.pipeline_);
	gst_bus_add_watch(bus, BusCall, &data_);

	SetTagsFilters();

	gst_object_unref(bus);
}

void Player::SetTagsFilters() {
	tags_map_[GST_TAG_TITLE] = NULL;
	tags_map_[GST_TAG_ARTIST] = NULL;
	tags_map_[GST_TAG_ALBUM] = NULL;
	tags_map_[GST_TAG_GENRE] = NULL;
	tags_map_[GST_TAG_DATE_TIME] = NULL;
}

std::map<const char*, char*, PlayerHelpers::CmpStr> *Player::GetTagsMap() {
	return &tags_map_;
}

const uint32_t Player::GetSampleRate() const {
	return sample_rate_;
}

AbstractSink *Player::AddSink(AbstractSink *sink) {
	sink->InitSink(static_cast<void *>(&data_));
	sinks_.push_back(sink);
	return sink;
}

void Player::RemoveSink(AbstractSink *sink) {
	std::list<AbstractSink *>::iterator it;
	it = sinks_.begin();
	while(it != sinks_.end()) {
		if(*(*it) == *sink) {
			AbstractSink *s = *it;
			s->Finish(&data_);
			sinks_.erase(it);
			return;
		}
	}
}

void Player::Init() {
	ConstructObjects();
	SetPropeties();
	LinkElements();
}

void Player::LinkElements() {
	g_assert(gst_element_link_many(data_.src_,
			data_.iddemux_,
			data_.parser_,
			data_.decoder_,
			data_.converter_,
			data_.pitch_,
			data_.tee_,
			NULL)
	);	//link chain
}

const AbstractSrc *Player::GetSrc() const {
	return src_;
}

void Player::SetPlaybackSpeed(float ratio) {
	g_object_set(G_OBJECT(data_.pitch_), "tempo", ratio, NULL);
}
