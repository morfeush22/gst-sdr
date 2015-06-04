/*
 * player.h
 *
 *  Created on: May 13, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
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

typedef void (*TagsMapCallback)(const std::map<const std::string, std::string> *, void *);

namespace PlayerHelpers {

/**
 * @struct Data
 * @biref This struct contains specific for Player class elements
 */
struct Data {
	void *player;	/**< Pointer to "this" Player element */

	GstElement *pipeline;	/**< Pipeline element for GStreamer */

	GstElement *iddemux;	/**< ID3 demuxer element for GStreamer */
	GstElement *decoder;	/**< FAAD decoder element for GStreamer */
	GstElement *parser;	/**< AAC parser element for GStreamer */
	GstElement *pitch;	/**< SoundTouch pitch element for GStreamer */
	GstElement *converter;	/**< Converter element for GStreamer */
	GstElement *tee;	/**< Tee element for GStreamer */

	GMainLoop *loop;	/**< Loop element for GStreamer */

	bool ready;	/**< Ready flag */
};

extern "C" {
/**
 * GStreamer callback called when got tags in pipeline. Check GStreamer documentation for more
 */
void SaveTags(const GstTagList *, const gchar *, gpointer);

/**
 * GStreamer callback called when bus event caught. Check GStreamer documentation for more
 */
gboolean BusCall(GstBus *, GstMessage *, gpointer);

}

}

/**
 * @class Player
 * @brief Class used to manage GStreaner pipeline
 *
 * @author Kacper Patro patro.kacper@gmail.com
 * @copyright Public domain
 * @pre
 */
class Player {
public:
	/**
	 * Constructor of Player
	 * @param *src Pointer to AbstractSrc object
	 */
	Player(AbstractSrc *);
	virtual ~Player();

	/**
	 * Starts processing
	 */
	void Process();

	/**
	 * Gets source
	 * @return Current AbstractSrc object
	 */
	const AbstractSrc *abstract_src() const;

	/**
	 * Removes sink
	 * @param sink Pointer to sink object
	 */
	void RemoveSink(AbstractSink *);

	/**
	 * Sets playback speed
	 */
	void set_playback_speed(float);

	/**
	 * Adds new sink
	 * @return Added sink
	 */
	AbstractSink *AddSink(AbstractSink *);

	/**
	 * Internal ready state
	 * @return True if Player ready to change playback speed
	 */
	bool ready() const;

	/**
	 * Registers tags map callback. cb_func will be called with cb_data passed when tags received
	 * @param cb_func Function to be called. Must match TagsMapCallback signature
	 * @param cb_data Data which will be passed to cb_func when called
	 */
	void RegisterTagsMapCallback(TagsMapCallback, void *);

	friend void PlayerHelpers::SaveTags(const GstTagList *, const gchar *, gpointer);
	friend gboolean PlayerHelpers::BusCall(GstBus *, GstMessage *, gpointer);

private:
	PlayerHelpers::Data data_;

	std::map<const std::string, std::string> *tags_map_;

	AbstractSrc *abstract_src_;
	std::list<AbstractSink *> abstract_sinks_;

	TagsMapCallback tags_map_cb_;
	void *tags_map_cb_data_;

	void SetTagsFilters();

	void ConstructObjects();
	void SetPropeties();
	void LinkElements();

	void Init();

};

#endif /* SRC_PLAYER_H_ */
