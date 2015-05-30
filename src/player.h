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

	GstElement *src;
	GstElement *iddemux;
	GstElement *decoder;
	GstElement *parser;
	GstElement *pitch;
	GstElement *converter;
	GstElement *tee;

	GMainLoop *loop;

	std::map<const std::string, std::string> *tags_map;
	gboolean ready;
};

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

	const std::map<const std::string, std::string> *tags_map() const;

private:
	PlayerHelpers::Data data_;

	AbstractSrc *abstract_src_;
	std::list<AbstractSink *> abstract_sinks_;

	void SetTagsFilters();

	void ConstructObjects();
	void SetPropeties();
	void LinkElements();

	void Init();

};

#endif /* SRC_PLAYER_H_ */
