/*
 * pulse_sink.cpp
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#include "pulse_sink.h"
#include "player.h"

#define QUEUE_BUFF_SIZE 1800	//1.8kB

static GstPadProbeReturn UnlinkCall(GstPad *pad, GstPadProbeInfo *info, gpointer container_ptr) {
	AbstractSinkHelpers::Data *container = ABSTRACT_SINK_DATA_CAST(container_ptr);
	PlayerHelpers::Data *data = PLAYER_DATA_CAST(container->other_data);
	PulseSinkHelpers::Data *sink_data = PULSE_SINK_DATA_CAST(container->sink_data);

	GstPad *sinkpad;

	if(!g_atomic_int_compare_and_exchange(&sink_data->removing, FALSE, TRUE))
		return GST_PAD_PROBE_OK;

	sinkpad = gst_element_get_static_pad(sink_data->queue, "sink");
	gst_pad_unlink(sink_data->teepad, sinkpad);
	gst_object_unref(sinkpad);

	gst_bin_remove(GST_BIN(data->pipeline), sink_data->queue);
	gst_bin_remove(GST_BIN(data->pipeline), sink_data->sink);

	gst_element_set_state(sink_data->sink, GST_STATE_NULL);
	gst_element_set_state(sink_data->queue, GST_STATE_NULL);

	gst_object_unref(sink_data->sink);
	gst_object_unref(sink_data->queue);

	gst_element_release_request_pad(data->tee, sink_data->teepad);
	gst_object_unref(sink_data->teepad);

	sink_data->linked = false;

	return GST_PAD_PROBE_REMOVE;
}

PulseSink::PulseSink() {
	data_ = new AbstractSinkHelpers::Data;

	PulseSinkHelpers::Data *temp = new PulseSinkHelpers::Data;
	temp->abstract_sink = this;
	temp->linked = false;

	data_->sink_data = temp;
	data_->other_data = NULL;
}

PulseSink::~PulseSink() {
	delete PULSE_SINK_DATA_CAST(data_->sink_data);
	delete data_;
}

void PulseSink::InitSink(void *other_data) {
	if(linked())
		return;

	data_->other_data = other_data;

	PlayerHelpers::Data *data = PLAYER_DATA_CAST(data_->other_data);
	PulseSinkHelpers::Data *sink_data = PULSE_SINK_DATA_CAST(data_->sink_data);

	GstPad *sinkpad;
	GstPadTemplate *templ;

	templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(data->tee), "src_%u");
	sink_data->teepad = gst_element_request_pad(data->tee, templ, NULL, NULL);

	char buff[100];

	strcpy(buff, get_name());
	strcat(buff, "_queue");

	sink_data->queue = gst_element_factory_make("queue", buff);
	g_assert(sink_data->queue);

	strcpy(buff, get_name());
	strcat(buff, "_sink");

	sink_data->sink = gst_element_factory_make(get_name(), buff);
	g_assert(sink_data->sink);

	g_object_set(sink_data->queue, "max-size-bytes", QUEUE_BUFF_SIZE, NULL);

	sink_data->removing = false;

	gst_object_ref(sink_data->queue);
	gst_object_ref(sink_data->sink);

	gst_bin_add_many(GST_BIN(data->pipeline),
			sink_data->queue,
			sink_data->sink,
			NULL);

	g_assert(gst_element_link_many(
			sink_data->queue,
			sink_data->sink,
			NULL)
	);

	gst_element_sync_state_with_parent(sink_data->queue);
	gst_element_sync_state_with_parent(sink_data->sink);

	sinkpad = gst_element_get_static_pad(sink_data->queue, "sink");
	gst_pad_link(sink_data->teepad, sinkpad);
	gst_object_unref(sinkpad);

	sink_data->linked = true;
}

const char *PulseSink::get_name() const {
	return "pulsesink";
}

void PulseSink::Finish() {
	if(!linked()) {
		return;
	}

	gst_pad_add_probe(PULSE_SINK_DATA_CAST(data_->sink_data)->teepad, GST_PAD_PROBE_TYPE_IDLE, UnlinkCall, data_, NULL);
}

bool PulseSink::linked() const {
	return PULSE_SINK_DATA_CAST(data_->sink_data)->linked;
}
