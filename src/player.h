/*
 * player.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_PLAYER_H_
#define SRC_PLAYER_H_

#include <gst/gst.h>

#include <stdint.h>
#include <string.h>
#include <map>
#include <list>
#include "abstract_sink.h"
#include "abstract_src.h"

namespace PlayerHelpers {

struct CmpStr {
	bool operator()(char const *a, char const *b) const {
		return strcmp(a, b) < 0;
	}
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
	 * Get received tags
	 * @return current tags map
	 */
	std::map<const char *, char *, PlayerHelpers::CmpStr> *GetTagsMap();

	/**
	 * Set playback speed
	 */
	void SetPlaybackSpeed(float);

	/**
	 * Add new sink
	 * @return added sink
	 */
	AbstractSink *AddSink(AbstractSink *);

	GstElement *pipeline_;

	GstElement *src_;
	GstElement *iddemux_;
	GstElement *decoder_;
	GstElement *parser_;
	GstElement *pitch_;
	GstElement *converter_;
	GstElement *tee_;

	GMainLoop *loop_;

	gboolean ready_;

private:
	AbstractSrc *abstract_src_;
	std::list<AbstractSink *> abstract_sinks_;

	std::map<const char *, char *, PlayerHelpers::CmpStr> tags_map_;

	void SetTagsFilters();

	void ConstructObjects();
	void SetPropeties();
	void LinkElements();

	void Init();

};

#endif /* SRC_PLAYER_H_ */
