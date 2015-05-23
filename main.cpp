#include "src/Player.h"
#include "src/FakeSink.h"
#include "src/FileSrc.h"
#include "src/RingSrc.h"
#include "src/PulseSink.h"
#include "src/FileSink.h"
#include "src/ring_buffer.h"

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

int main() {
	RingSrc *src = new RingSrc("./player_unittest_file.aac", 0.20);
	PulseSink *sink = new PulseSink();
	FileSink *new_sink = new FileSink("./test.raw");

	Player player(src);
	player.AddSink(sink);

	Data data;
	data.player = &player;
	data.sink = new_sink;

	g_timeout_add_seconds(1, AddCb, &data);
	g_timeout_add_seconds(5, RemCb, &data);
	player.Process();

	delete sink;
	delete src;
}
