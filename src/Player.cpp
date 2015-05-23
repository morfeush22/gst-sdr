/*
 * Player.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "Player.h"
#include <stdio.h>

static void SaveTags(const GstTagList *list, const gchar *tag, gpointer ptr) {
	Player *player = (Player *)ptr;
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
	Player *player = (Player *)ptr;

	switch(GST_MESSAGE_TYPE(message)) {

	case GST_MESSAGE_ERROR: {
		gchar *debug;
		GError *err;

		gst_message_parse_error(message, &err, &debug);
		g_print("GStreamer error: %s\n", err->message);
		g_error_free(err);

		g_free(debug);
		g_main_loop_quit(player->loop_);
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
		player->ready_ = TRUE;
		break;
	}

	case GST_MESSAGE_TAG: {
		GstTagList *tags = NULL;

		gst_message_parse_tag(message, &tags);

		gst_tag_list_foreach(tags, SaveTags, player);

		gst_tag_list_free (tags);
		break;
	}

	case GST_MESSAGE_EOS:
		g_main_loop_quit(player->loop_);
		break;

	default:
		break;
	}

	return TRUE;
}

Player::Player(AbstractSrc *src):
abstract_src_(src) {
	gst_init (NULL, NULL);
	Init();
}

Player::~Player() {
}

void Player::Process() {
	if(!abstract_sinks_.size()) {
		g_print("GStreamer: no sinks\n");
		return;
	}

	gst_element_set_state((GstElement*)pipeline_, GST_STATE_PLAYING);
	loop_ = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(loop_);

	gst_element_set_state((GstElement*)pipeline_, GST_STATE_NULL);
	gst_object_unref(pipeline_);

	std::list<AbstractSink *>::iterator it;
	it = abstract_sinks_.begin();
	while(it != abstract_sinks_.end()) {
		(*it)->UnlinkFinished();
		it = abstract_sinks_.erase(it);
	}
}

void Player::ConstructObjects() {
	pipeline_ = gst_pipeline_new("player");

	iddemux_ = gst_element_factory_make("id3demux", "tag_demuxer");
	decoder_ = gst_element_factory_make("faad", "decoder");
	parser_ = gst_element_factory_make("aacparse", "parser");

	pitch_ = gst_element_factory_make("pitch", "pitch");
	converter_ = gst_element_factory_make("audioconvert", "converter");

	tee_ = gst_element_factory_make("tee", "tee");

	g_assert(pipeline_
			&& iddemux_
			&& decoder_
			&& parser_
			&& pitch_
			&& converter_
			&& tee_);	//check if all elements created
}

void Player::SetPropeties() {
	GstBus *bus;

	abstract_src_->InitSrc(static_cast<void *>(this));

	gst_bin_add_many(GST_BIN(pipeline_),
			src_,
			iddemux_,
			decoder_,
			parser_,
			pitch_,
			converter_,
			tee_,
			NULL);	//add elements to bin

	bus = gst_element_get_bus(pipeline_);
	gst_bus_add_watch(bus, BusCall, this);

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

AbstractSink *Player::AddSink(AbstractSink *sink) {
	sink->InitSink(static_cast<void *>(this));
	abstract_sinks_.push_back(sink);
	return sink;
}

void Player::RemoveSink(AbstractSink *sink) {
	std::list<AbstractSink *>::iterator it;
	it = abstract_sinks_.begin();
	while(it != abstract_sinks_.end()) {
		if(*it == sink) {
			AbstractSink *s = *it;
			s->FinishEarly(this);
			abstract_sinks_.erase(it);
			if(!abstract_sinks_.size())
				gst_element_set_state(pipeline_, GST_STATE_NULL);
			return;
		}
		it++;
	}
}

void Player::Init() {
	ConstructObjects();
	SetPropeties();
	LinkElements();
}

void Player::LinkElements() {
	g_assert(gst_element_link_many(src_,
			iddemux_,
			parser_,
			decoder_,
			converter_,
			pitch_,
			tee_,
			NULL)
	);	//link chain
}

const AbstractSrc *Player::GetSrc() const {
	return abstract_src_;
}

void Player::SetPlaybackSpeed(float ratio) {
	g_object_set(G_OBJECT(pitch_), "tempo", ratio, NULL);
}
