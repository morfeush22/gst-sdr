/*
 * audio_decoder.cpp
 *
 *  Created on: May 23, 2015
 *      Author: morfeush22
 */

#include "audio_decoder.h"

AudioDecoder::AudioDecoder(float threshold, size_t length) {
	src_ = new RingSrc(threshold, length);
	sink_ = new PulseSink();

	player_ = new Player(src_);
	player_->AddSink(sink_);
}

AudioDecoder::~AudioDecoder() {
	delete player_;
	delete sink_;
	delete src_;
}

void AudioDecoder::RemoveSink(AbstractSink *sink) {
	player_->RemoveSink(sink);
}

AbstractSink *AudioDecoder::AddSink(AbstractSink *sink) {
	return player_->AddSink(sink);
}

void AudioDecoder::Write(uint8_t *buffer, size_t length) {
	src_->Write(buffer, length);
}

void AudioDecoder::LastFrame() {
	src_->set_last_frame(true);
}

const std::map<const std::string, std::string> *AudioDecoder::tags_map() const {
	return player_->tags_map();
}

void AudioDecoder::Process() {
	player_->Process();
}
