/*
 * audio_decoder.h
 *
 *  Created on: May 23, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#ifndef AUDIO_DECODER_H_
#define AUDIO_DECODER_H_

#include "AudioDecoder/src/abstract_sink.h"
#include "AudioDecoder/src/player.h"
#include "AudioDecoder/src/fake_sink.h"
#include "AudioDecoder/src/ring_src.h"

/**
 * @class AudioDecoder
 * @brief Facade class for audio playing
 *
 * @author Kacper Patro patro.kacper@gmail.com
 * @copyright Public domain
 * @pre
 */
class AudioDecoder {
public:
	/**
	 * Constructor of AudioDecoder
	 * @param threshold Relative to 0.5, indicates values for which audio time-stretching will take place
	 * @param length Length of internal blocking ring buffer
	 */
	AudioDecoder(float, size_t);
	virtual ~AudioDecoder();

	/**
	 * Removes sink
	 * @param sink Pointer to sink object
	 */
	void RemoveSink(AbstractSink *);

	/**
	 * Adds new sink
	 * @return Added sink
	 */
	AbstractSink *AddSink(AbstractSink *);

	/**
	 * Writes into internal ring buffer, could override oldest data when full
	 * @param buffer Pointer to source buffer
	 * @param length Length of source buffer to read from
	 */
	void Write(uint8_t *, size_t);

	/**
	 * Should be called when current portion of data is last one
	 */
	void LastFrame();

	/**
	 * Registers tags map callback. cb_func will be called with cb_data passed when tags received
	 * @param cb_func Function to be called. Must match TagsMapCallback signature
	 * @param cb_data Data which will be passed to cb_func when called
	 */
	void RegisterTagsMapCallback(TagsMapCallback, void *);

	/**
	 * Starts audio processing
	 */
	void Process();

private:
	RingSrc *src_;
	FakeSink *sink_;
	Player *player_;

};

#endif /* AUDIO_DECODER_H_ */
