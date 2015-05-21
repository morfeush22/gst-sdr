#include "src/Player.h"
#include "src/FakeSink.h"
#include "src/FileSrc.h"
#include "src/RingSrc.h"
#include "src/PulseSink.h"
#include "src/ring_buffer.h"

#include <iostream>

using namespace std;

int main() {
	RingSrc *src = new RingSrc("./player_unittest_file.aac", 0.2);

	PulseSink *sink = new PulseSink();

	Player player(src, 44100);
	player.AddSink(sink);
	player.Process();

	delete sink;
	delete src;
}
