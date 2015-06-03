/*
 * ogg_sink.h
 *
 *  Created on: May 29, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#ifndef SRC_OGG_SINK_H_
#define SRC_OGG_SINK_H_

#define OGG_SINK_CAST(X) (reinterpret_cast<OggSink *>(X))
#define OGG_SINK_DATA_CAST(X) (reinterpret_cast<OggSinkHelpers::Data *>(X))

#include "abstract_sink.h"
#include <gst/gst.h>

namespace OggSinkHelpers {

/**
 * @struct Data
 * @biref This struct contains specific for OggSink class elements
 */
struct Data {
	void *abstract_sink;	/**< Pointer to "this" sink element */

	GstElement *queue;	/**< Queue element for GStreamer */
	GstElement *encoder;	/**< Vorbis encoder element for GStreamer */
	GstElement *muxer;	/**< Ogg muxer element for GStreamer */
	GstElement *sink;	/**< Sink element for GStreamer */
	GstPad *teepad;	/**< TeePad element for GStreamer */

	gboolean removing;	/**< True, when sink is being removed from pipeline */
	bool linked;	/**< True, when sink is linked in pipeline */
};

/**
 * GStreamer callback called when unlinking sink from pipeline. Check GStreamer documentation for more
 */
GstPadProbeReturn UnlinkCall(GstPad *, GstPadProbeInfo *, gpointer);

}

/**
 * @class OggSink
 * @brief Class used to save ogg audio files
 *
 * @author Kacper Patro patro.kacper@gmail.com
 * @copyright Public domain
 * @pre
 */
class OggSink: public AbstractSink {
public:
	/**
	 * Constructor of OggSink
	 * @param path Path to output file
	 */
	OggSink(const char *);
	virtual ~OggSink();

	void InitSink(void *);
	const char *name() const;
	void Finish();
	bool linked() const;

private:
	AbstractSinkHelpers::Data *data_;

	const char *path_;

};

#endif /* SRC_OGG_SINK_H_ */
