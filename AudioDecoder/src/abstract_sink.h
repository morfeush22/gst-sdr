/*
 * abstract_sink.h
 *
 *  Created on: May 24, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#ifndef SRC_ABSTRACT_SINK_H_
#define SRC_ABSTRACT_SINK_H_

#define ABSTRACT_SINK_CAST(X) (reinterpret_cast<AbstractSink *>(X))
#define ABSTRACT_SINK_DATA_CAST(X) (reinterpret_cast<AbstractSinkHelpers::Data *>(X))

namespace AbstractSinkHelpers {

/**
 * @struct Data
 * @biref This struct contains pointers to data passed when calling GStreamer functions
 */
struct Data {
	void *sink_data;	/**< Sink data pointer */
	void *other_data;	/**< Other data pointer */
};

}

/**
 * @class AbstractSink
 * @brief Abstract class for sinks
 *
 * @author Kacper Patro patro.kacper@gmail.com
 * @copyright Public domain
 * @pre
 */
class AbstractSink {
public:
	AbstractSink();
	virtual ~AbstractSink();

	/**
	 * Initialize sink structures
	 * @param other_data Other data pointer, which should be assigned to other_data in AbstractSinkHelpers::Data structure
	 * @pure
	 */
	virtual void InitSink(void *other_data) = 0;

	/**
	 * Returns element name
	 * @return Element name
	 * @pure
	 */
	virtual const char *name() const = 0;

	/**
	 * This one should be called when unlinking sink
	 * @pure
	 */
	virtual void Finish() = 0;

	/**
	 * Checks if sink is already linked
	 * @return Current linkage status
	 * @pure
	 */
	virtual bool linked() const = 0;

	/**
	 * Equality operator overload, compares sinks names
	 * @return True when elements equal, false otherwise
	 * @pure
	 */
	bool operator ==(const AbstractSink &other) const;

};

#endif /* SRC_ABSTRACT_SINK_H_ */
