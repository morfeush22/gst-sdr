#include "src/ring_buffer.h"
#include "src/fake_sink.h"
#include "src/file_sink.h"
#include "src/file_src.h"
#include "src/player.h"
#include "src/pulse_sink.h"
#include "src/ring_src.h"
#include "data_dumper.h"
#include "src/blocking_ring_buffer.h"

#include <iostream>

using namespace std;

struct Data {
	void *player;
	void *sink;
};

static gboolean AddCb(gpointer data) {
	cout << "###################adding sink###################" << endl;

	Player *player = (Player *)((Data *)data)->player;
	FileSink *sink = (FileSink *)((Data *)data)->sink;
	player->AddSink(sink);

	cout << "###################sink added###################" << endl;
	return FALSE;
}

static gboolean RemCb(gpointer data) {
	cout << "###################removing sink###################" << endl;

	Player *player = (Player *)((Data *)data)->player;
	FileSink *sink = (FileSink *)((Data *)data)->sink;
	player->RemoveSink(sink);

	cout << "###################sink removed###################" << endl;

	return FALSE;
}

static void *ReadingThread(void *context) {
	DataDumper *dumper = (DataDumper *)context;
	return dumper->StartDumping();
}

int main() {
	pthread_t read_thread;
	pthread_attr_t attr;

	RingSrc *src = new RingSrc(0.20);
	PulseSink *sink = new PulseSink();
	FileSink *new_sink = new FileSink("./test.raw");

	DataDumper *dumper = new DataDumper(100000, 1000000000, "./player_unittest_file.aac", src);

	pthread_attr_init(&attr);
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&read_thread, &attr, &ReadingThread, dumper);

	Player player(src);
	player.AddSink(sink);

	Data data;
	data.player = &player;
	data.sink = new_sink;

	//g_timeout_add_seconds(1, AddCb, &data);
	//g_timeout_add_seconds(5, RemCb, &data);

	player.Process();

	delete dumper;
	delete sink;
	delete src;
}
