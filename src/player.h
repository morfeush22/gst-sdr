/*
 * player.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_PLAYER_H_
#define SRC_PLAYER_H_

#include "abstract_sink.h"
#include "abstract_src.h"

#include <gst/gst.h>

#include <stdint.h>
#include <string.h>
#include <map>
#include <list>

namespace PlayerHelpers {

struct CmpStr {
	bool operator()(char const *a, char const *b) const {
		return strcmp(a, b) < 0;
	}
};

struct Data {
	void *player_;

	GstElement *pipeline_;

	GstElement *src_;
	GstElement *iddemux_;
	GstElement *decoder_;
	GstElement *parser_;
	GstElement *pitch_;
	GstElement *converter_;
	GstElement *tee_;

	GMainLoop *loop_;

	std::map<const char *, char *, PlayerHelpers::CmpStr> tags_map_;
	gboolean ready_;
};

}

class Player {
public:
	/**
	 * Play audio
	 * @param *src pointer to AbstractSrc object
	 * @param *sink pointer to AbstractSink object
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
	const AbstractSrc *GetSrc() const;

	/**
	 * Remove sink
	 */
	void RemoveSink(AbstractSink *);

	/**
	 * Set playback speed
	 */
	void SetPlaybackSpeed(float);

	/**
	 * Add new sink
	 * @return added sink
	 */
	AbstractSink *AddSink(AbstractSink *);

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
