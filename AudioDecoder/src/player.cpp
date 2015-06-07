/*
 * player.cpp
 *
 *  Created on: May 13, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#include "player.h"
#include <sstream>

void PlayerHelpers::SaveTags(const GstTagList *list, const gchar *tag, gpointer data_ptr) {
	Player *player = PLAYER_CAST(data_ptr);
	std::map<const std::string, std::string>::iterator t_it;
	std::map<const std::string, TagsMapParser>::iterator f_it;

	gint count = gst_tag_list_get_tag_size(list, tag);

	for(gint i = 0; i < count; i++) {
		if((t_it = player->tags_map_->find(gst_tag_get_nick(tag))) != player->tags_map_->end()) {	//if we are interested in this tag
			if((f_it = player->tags_map_parsers_->find(gst_tag_get_nick(tag))) != player->tags_map_parsers_->end())	//if this tag has parser function
				 f_it->second(list, tag, i, &t_it->second);
		}
	}

	if(player->tags_map_cb_ != NULL) {
		player->tags_map_cb_(player->tags_map_, player->tags_map_cb_data_);
		//for(it = player->tags_map_->begin(); it != player->tags_map_->end(); it++)
		//	it->second = "";
	}
}

gboolean PlayerHelpers::BusCall(GstBus *bus, GstMessage *message, gpointer player_data_ptr) {
	PlayerHelpers::Data *player_data = PLAYER_DATA_CAST(player_data_ptr);

	switch(GST_MESSAGE_TYPE(message)) {

	case GST_MESSAGE_ERROR: {
		gchar *debug;
		GError *err;

		gst_message_parse_error(message, &err, &debug);
		g_print("GStreamer error: %s\n", err->message);
		g_error_free(err);

		g_free(debug);
		g_main_loop_quit(player_data->loop);
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
		//g_print ("GStreamer: %s prerolled, lock'n'load\n", GST_OBJECT_NAME(message->src));
		player_data->ready = true;
		break;
	}

	case GST_MESSAGE_TAG: {
		GstTagList *tags = NULL;

		gst_message_parse_tag(message, &tags);

		gst_tag_list_foreach(tags, SaveTags, player_data->player);

		gst_tag_list_free(tags);
		break;
	}

	case GST_MESSAGE_EOS:
		g_main_loop_quit(player_data->loop);
		break;

	default:
		break;
	}

	return TRUE;
}

Player::Player(AbstractSrc *src):
tags_map_(new std::map<const std::string, std::string>),
tags_map_parsers_(new std::map<const std::string, TagsMapParser>),
abstract_src_(src),
abstract_sinks_(new std::list<AbstractSink *>),
tags_map_cb_(NULL),
tags_map_cb_data_(NULL) {
	gst_init (NULL, NULL);

	data_.player = this;
	data_.ready = false;

	Init();
}

Player::~Player() {
	std::list<AbstractSink *>::iterator it;
	it = abstract_sinks_->begin();

	while(it != abstract_sinks_->end()) {
		(*it)->Finish();
		it = abstract_sinks_->erase(it);
	}
	delete abstract_sinks_;

	gst_object_unref(data_.pipeline);
	delete tags_map_parsers_;
	delete tags_map_;
}

void Player::Process() {
	if(!abstract_sinks_->size()) {
		g_print("GStreamer: no sinks\n");
		return;
	}

	gst_element_set_state(data_.pipeline, GST_STATE_PLAYING);
	data_.loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(data_.loop);
	g_main_loop_unref(data_.loop);

	gst_element_set_state(data_.pipeline, GST_STATE_NULL);

	data_.ready = false;
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

	abstract_src_->SetSrc(&data_);

	gst_bin_add_many(GST_BIN(data_.pipeline),
			data_.iddemux,
			data_.decoder,
			data_.parser,
			data_.pitch,
			data_.converter,
			data_.tee,
			NULL);	//add elements to bin

	bus = gst_element_get_bus(data_.pipeline);
	gst_bus_add_watch(bus, PlayerHelpers::BusCall, &data_);

	SetTagsFilters();

	gst_object_unref(bus);
}

void Player::SetTagsFilters() {
	tags_map_->insert(std::make_pair(GST_TAG_TITLE, ""));
	tags_map_->insert(std::make_pair(GST_TAG_ARTIST, ""));
	tags_map_->insert(std::make_pair(GST_TAG_ALBUM, ""));
	tags_map_->insert(std::make_pair(GST_TAG_GENRE, ""));
	tags_map_->insert(std::make_pair(GST_TAG_DATE_TIME, ""));

	tags_map_parsers_->insert(std::make_pair(GST_TAG_TITLE, PlayerHelpers::ParseStringTag));
	tags_map_parsers_->insert(std::make_pair(GST_TAG_ARTIST, PlayerHelpers::ParseStringTag));
	tags_map_parsers_->insert(std::make_pair(GST_TAG_ALBUM, PlayerHelpers::ParseStringTag));
	tags_map_parsers_->insert(std::make_pair(GST_TAG_GENRE, PlayerHelpers::ParseStringTag));
	tags_map_parsers_->insert(std::make_pair(GST_TAG_DATE_TIME, PlayerHelpers::ParseDataTag));
}

AbstractSink *Player::AddSink(AbstractSink *sink) {
	sink->InitSink(&data_);
	abstract_sinks_->push_back(sink);

	return sink;
}

void Player::RemoveSink(AbstractSink *sink) {
	std::list<AbstractSink *>::iterator it;
	it = abstract_sinks_->begin();
	while(it != abstract_sinks_->end()) {
		if(*it == sink) {
			(*it)->Finish();
			abstract_sinks_->erase(it);

			if(!abstract_sinks_->size())
				gst_element_set_state(data_.pipeline, GST_STATE_NULL);
			return;
		}
		it++;
	}
}

bool Player::ready() const {
	return data_.ready;
}

void Player::Init() {
	ConstructObjects();
	SetPropeties();
	LinkElements();
}

void Player::LinkElements() {
	abstract_src_->LinkSrc();

	g_assert(gst_element_link_many(
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

void Player::RegisterTagsMapCallback(TagsMapCallback cb_func, void *cb_data) {
	tags_map_cb_ = cb_func;
	tags_map_cb_data_ = cb_data;
}

void PlayerHelpers::ParseStringTag(const GstTagList *list, const gchar *tag, gint index, void *ptr_to_elem) {
	std::string *elem_data = reinterpret_cast<std::string *>(ptr_to_elem);
	gchar *string;

	gst_tag_list_get_string_index(list, tag, index, &string);

	std::string temp = std::string(string);
	*(elem_data) = temp;

	g_free(string);
}

void PlayerHelpers::ParseDataTag(const GstTagList *list, const gchar *tag, gint index, void *ptr_to_elem) {
	std::string *elem_data = reinterpret_cast<std::string *>(ptr_to_elem);
	GstDateTime *time;

	gst_tag_list_get_date_time_index(list, tag, index, &time);
	gint year = gst_date_time_get_year(time);

	std::ostringstream t;
	t << year;
	std::string temp = t.str();
	*(elem_data) = temp;

	gst_date_time_unref(time);
}
