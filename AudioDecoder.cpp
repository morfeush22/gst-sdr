/*
 * AudioDecoder.cpp
 *
 *  Created on: May 23, 2015
 *      Author: morfeush22
 */

#include "AudioDecoder.h"

AudioDecoder::AudioDecoder():
src_(NULL),
sink_(NULL),
player_(NULL) {
}

AudioDecoder::~AudioDecoder() {
	delete player_;
	delete sink_;
	delete src_;
}

void AudioDecoder::RemoveSink(AbstractSink *sink) {
	player_->RemoveSink(sink);
}

AbstractSink* AudioDecoder::AddSink(AbstractSink *sink) {
	return player_->AddSink(sink);
}

void AudioDecoder::Write(float *buff, size_t length) {
}
