/*
 * player.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "player.h"
#include <stdio.h>

static void SaveTags(const GstTagList *list, const gchar *tag, gpointer data_ptr) {
	PlayerHelpers::Data *data = PLAYER_DATA_CAST(data_ptr);
	std::map<const char *, char *, PlayerHelpers::CmpStr>::iterator it;

	gint count = gst_tag_list_get_tag_size(list, tag);

	for(gint i = 0; i < count; i++) {
		if((it = data->tags_map->find(gst_tag_get_nick(tag))) != data->tags_map->end()) {
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

static gboolean BusCall(GstBus *bus, GstMessage *message, gpointer data_ptr) {
	PlayerHelpers::Data *data = PLAYER_DATA_CAST(data_ptr);

	switch(GST_MESSAGE_TYPE(message)) {

	case GST_MESSAGE_ERROR: {
		gchar *debug;
		GError *err;

		gst_message_parse_error(message, &err, &debug);
		g_print("GStreamer error: %s\n", err->message);
		g_error_free(err);

		g_free(debug);
		g_main_loop_quit(data->loop);
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
		data->ready = TRUE;
		break;
	}

	case GST_MESSAGE_TAG: {
		GstTagList *tags = NULL;

		gst_message_parse_tag(message, &tags);

		gst_tag_list_foreach(tags, SaveTags, data);

		gst_tag_list_free(tags);
		break;
	}

	case GST_MESSAGE_EOS:
		g_main_loop_quit(data->loop);
		break;

	default:
		break;
	}

	return TRUE;
}

Player::Player(AbstractSrc *src):
abstract_src_(src) {
	gst_init (NULL, NULL);

	data_.tags_map = new std::map<const char *, char *, PlayerHelpers::CmpStr>;
	data_.player = this;
	data_.ready = FALSE;

	Init();
}

Player::~Player() {
	delete data_.tags_map;
}

void Player::Process() {
	if(!abstract_sinks_.size()) {
		g_print("GStreamer: no sinks\n");
		return;
	}

	gst_element_set_state(reinterpret_cast<GstElement *>(data_.pipeline), GST_STATE_PLAYING);
	data_.loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(data_.loop);

	gst_element_set_state(reinterpret_cast<GstElement *>(data_.pipeline), GST_STATE_NULL);
	gst_object_unref(data_.pipeline);

	std::list<AbstractSink *>::iterator it;
	it = abstract_sinks_.begin();

	while(it != abstract_sinks_.end()) {
		//(*it)->Finish();
		it = abstract_sinks_.erase(it);
	}

	data_.ready = FALSE;
}

void Player::ConstructObjects() {
	data_.pipeline = gst_pipeline_new("player");

	data_.iddemux = gst_element_factory_make("id3demux", "tag_demuxer");
	data_.decoder = gst_element_factory_make("faad", "decoder");
	data_.parser = gst_element_factory_make("aacparse", "parser");

	data_.pitch = gst_element_factory_make("pitch", "pitch");
	data_.converter = gst_element_factory_make("audioconvert", "converter");

	data_.tee = gst_element_factory_make("tee", "tee");

	g_assert(data_.pipeline
			&& data_.iddemux
			&& data_.decoder
			&& data_.parser
			&& data_.pitch
			&& data_.converter
			&& data_.tee);	//check if all elements created
}

void Player::SetPropeties() {
	GstBus *bus;

	abstract_src_->InitSrc(&data_);

	gst_bin_add_many(GST_BIN(data_.pipeline),
			data_.src,
			data_.iddemux,
			data_.decoder,
			data_.parser,
			data_.pitch,
			data_.converter,
			data_.tee,
			NULL);	//add elements to bin

	bus = gst_element_get_bus(data_.pipeline);
	gst_bus_add_watch(bus, BusCall, &data_);

	SetTagsFilters();

	gst_object_unref(bus);
}

void Player::SetTagsFilters() {
	(*data_.tags_map)[GST_TAG_TITLE] = NULL;
	(*data_.tags_map)[GST_TAG_ARTIST] = NULL;
	(*data_.tags_map)[GST_TAG_ALBUM] = NULL;
	(*data_.tags_map)[GST_TAG_GENRE] = NULL;
	(*data_.tags_map)[GST_TAG_DATE_TIME] = NULL;
}

AbstractSink *Player::AddSink(AbstractSink *sink) {
	sink->InitSink(&data_);
	abstract_sinks_.push_back(sink);

	return sink;
}

void Player::RemoveSink(AbstractSink *sink) {
	std::list<AbstractSink *>::iterator it;
	it = abstract_sinks_.begin();
	while(it != abstract_sinks_.end()) {
		if(*it == sink) {
			(*it)->Finish();
			abstract_sinks_.erase(it);

			if(!abstract_sinks_.size())
				gst_element_set_state(data_.pipeline, GST_STATE_NULL);
			return;
		}
		it++;
	}
}

const std::map<const char*, char*, PlayerHelpers::CmpStr> *Player::tags_map() const {
	return data_.tags_map;
}

void Player::Init() {
	ConstructObjects();
	SetPropeties();
	LinkElements();
}

void Player::LinkElements() {
	g_assert(gst_element_link_many(data_.src,
			data_.iddemux,
			data_.parser,
			data_.decoder,
			data_.converter,
			data_.pitch,
			data_.tee,
			NULL)
	);	//link chain
}

const AbstractSrc *Player::abstract_src() const {
	return abstract_src_;
}

void Player::set_playback_speed(float ratio) {
	g_object_set(G_OBJECT(data_.pitch), "tempo", ratio, NULL);
}
