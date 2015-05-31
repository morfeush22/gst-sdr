/*
 * player.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_PLAYER_H_
#define SRC_PLAYER_H_

#define PLAYER_CAST(X) (reinterpret_cast<Player *>(X))
#define PLAYER_DATA_CAST(X) (reinterpret_cast<PlayerHelpers::Data *>(X))

#include "abstract_sink.h"
#include "abstract_src.h"
#include <gst/gst.h>
#include <list>
#include <map>
#include <string>
#include <stdint.h>
#include <string.h>

namespace PlayerHelpers {

struct Data {
	void *player;

	GstElement *pipeline;

	GstElement *iddemux;
	GstElement *decoder;
	GstElement *parser;
	GstElement *pitch;
	GstElement *converter;
	GstElement *tee;

	GMainLoop *loop;

	bool ready;
};

void SaveTags(const GstTagList *, const gchar *, gpointer);
gboolean BusCall(GstBus *, GstMessage *, gpointer);

}

class Player {
public:
	/**
	 * Play audio
	 * @param *src pointer to AbstractSrc object
	 */
	Player(AbstractSrc *);
	virtual ~Player();

	/**
	 * Start processing
	 */
	void Process();

	/**
	 * Get source
	 * @return current AbstractSrc object
	 */
	const AbstractSrc *abstract_src() const;

	/**
	 * Remove sink
	 */
	void RemoveSink(AbstractSink *);

	/**
	 * Set playback speed
	 */
	void set_playback_speed(float);

	/**
	 * Add new sink
	 * @return added sink
	 */
	AbstractSink *AddSink(AbstractSink *);

	/**
	 * Get tags map
	 * @return constant pointer to tags map
	 */
	const std::map<const std::string, std::string> *tags_map() const;

	/**
	 * Internal ready state
	 * @return true if player ready to change playback speed
	 */
	const bool ready() const;

	friend void PlayerHelpers::SaveTags(const GstTagList *, const gchar *, gpointer);
	friend gboolean PlayerHelpers::BusCall(GstBus *, GstMessage *, gpointer);

private:
	PlayerHelpers::Data data_;

	std::map<const std::string, std::string> *tags_map_;

	AbstractSrc *abstract_src_;
	std::list<AbstractSink *> abstract_sinks_;

	void SetTagsFilters();

	void ConstructObjects();
	void SetPropeties();
	void LinkElements();

	void Init();

};

#endif /* SRC_PLAYER_H_ */
