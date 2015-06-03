/*
 * file_sink.h
 *
 *  Created on: May 21, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#ifndef SRC_FILE_SINK_H_
#define SRC_FILE_SINK_H_

#define FILE_SINK_CAST(X) (reinterpret_cast<FileSink *>(X))
#define FILE_SINK_DATA_CAST(X) (reinterpret_cast<FileSinkHelpers::Data *>(X))

#include "abstract_sink.h"
#include <gst/gst.h>

namespace FileSinkHelpers {

/**
 * @struct Data
 * @biref This struct contains specific for FileSink class elements
 */
struct Data {
	void *abstract_sink;	/**< Pointer to "this" sink element */

	GstElement *queue;	/**< Queue element for GStreamer */
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
 * @class FileSink
 * @brief Class used to save raw audio files
 *
 * @author Kacper Patro patro.kacper@gmail.com
 * @copyright Public domain
 * @pre
 */
class FileSink: public AbstractSink {
public:
	/**
	 * Constructor of FileSink
	 * @param path Path to output file
	 */
	FileSink(const char *);
	virtual ~FileSink();

	void InitSink(void *);
	const char *name() const;
	void Finish();
	bool linked() const;

private:
	AbstractSinkHelpers::Data *data_;

	const char *path_;

};

#endif /* SRC_FILE_SINK_H_ */
