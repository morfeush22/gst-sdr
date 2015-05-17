/*
 * Player.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_PLAYER_H_
#define SRC_PLAYER_H_

#define AUDIO_CAPS "audio/x-raw, format=S16LE, channels=2, layout=interleaved, rate=%d"

#include "AbstractSink.h"
#include "AbstractSrc.h"
#include <gst/gst.h>

#include <stdint.h>
#include <string.h>
#include <map>

namespace PlayerHelpers {

struct Data {
	void *player_;

	GstElement *pipeline_;

	GstElement *src_;
	GstElement *sink_;

	GstElement *iddemux_;
	GstElement *decoder_;
	GstElement *parser_;

	GstElement *pitch_;
	GstElement *converter_;

	GMainLoop *loop_;
	gboolean ready_;
};

struct CmpStr {
	bool operator()(char const *a, char const *b) {
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
	 * @param sample_rate starting sample rate
	 */
	Player(AbstractSrc *, AbstractSink *, uint32_t);
	virtual ~Player();

	/**
	 * Start processing
	 */
	void Process();

	/**
	 * Get source
	 * @return current AbstractSrc object
	 */
	AbstractSrc *GetSrc();

	/**
	 * Get sink
	 * @return current AbstractSink object
	 */
	AbstractSink *GetSink();

	/**
	 * Get received tags
	 * @return current tags map
	 */
	std::map<const char *, char *, PlayerHelpers::CmpStr> *GetTagsMap();

	/**
	 * Get sample rate
	 * @return init sample rate
	 */
	uint32_t GetSampleRate();

	/**
	 * Set playback speed
	 */
	void SetPlaybackSpeed(float);

private:
	PlayerHelpers::Data data_;
	const uint32_t sample_rate_;
	AbstractSrc *src_;
	AbstractSink *sink_;
	std::map<const char *, char *, PlayerHelpers::CmpStr> tags_map_;

	void SetTagsFilters();
	void ConstructObjects();
	void SetPropeties();
	void LinkElements();

};

#endif /* SRC_PLAYER_H_ */
