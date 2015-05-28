#include "src/player.h"
#include "src/fake_sink.h"
#include "src/file_src.h"
#include "src/ring_src.h"
#include "gtest/gtest.h"

#include <fstream>
#include <string>

using namespace std;

uint16_t FakeSink::count_ = 0;

TEST(PlayerTestBytesTest, number_of_processed_bytes) {
	uint32_t size;
	ifstream in_file("./player_unittest_file.raw", ifstream::binary);
	ASSERT_TRUE(in_file.good()) << "TESTING CODE FAILED... could not load data";

	in_file.seekg(0, ios::end);
	size = in_file.tellg();

	in_file.close();

	FileSrc *src = new FileSrc("./player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	Player player(src);
	player.AddSink(sink);
	player.Process();

	EXPECT_EQ(size, sink->bytes_returned());

	delete sink;
	delete src;
}

TEST(PlayerTestTagsTest, tags_returned) {
	ifstream in_file("./player_unittest_tags_expected.txt");
	ASSERT_TRUE(in_file.good()) << "TESTING CODE FAILED... could not load data";

	FileSrc *src = new FileSrc("./player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	Player player(src);
	player.AddSink(sink);
	player.Process();

	std::map<const char *, char *, PlayerHelpers::CmpStr>::const_iterator it;
	string line;

	while(getline(in_file, line)) {
		it = player.tags_map()->find(line.c_str());
		ASSERT_TRUE(it != player.tags_map()->end());
		getline(in_file, line);
		EXPECT_FALSE(strcmp(it->second, line.c_str()));
	}

	delete sink;
	delete src;
}

TEST(PlayerTestTeeTest, number_of_src_pads) {
	FileSrc *src = new FileSrc("./player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	Player player(src);
	player.AddSink(sink);

	EXPECT_EQ(1, sink->num_src_pads());

	delete sink;
	delete src;
}

TEST(PlayerTestAddSink, incrementation_of_sink_pads) {
	FileSrc *src = new FileSrc("./player_unittest_file.aac");
	FakeSink *sink1 = new FakeSink();
	FakeSink *sink2 = new FakeSink();
	FakeSink *sink3 = new FakeSink();

	Player player(src);
	EXPECT_EQ(sink1, player.AddSink(sink1));
	EXPECT_EQ(sink2, player.AddSink(sink2));
	EXPECT_EQ(sink3, player.AddSink(sink3));

	EXPECT_EQ(3, sink1->num_src_pads());

	delete sink1;
	delete sink2;
	delete sink3;
	delete src;
}

TEST(PlayerTestRemoveSink, decrementation_of_sink_pads) {
	FileSrc *src = new FileSrc("./player_unittest_file.aac");
	FakeSink *sink1 = new FakeSink();
	FakeSink *sink2 = new FakeSink();
	FakeSink *sink3 = new FakeSink();

	Player player(src);
	player.AddSink(sink1);
	player.AddSink(sink2);
	player.AddSink(sink3);

	player.RemoveSink(sink3);

	EXPECT_EQ(2, sink1->num_src_pads());

	delete sink1;
	delete sink2;
	delete sink3;
	delete src;
}

TEST(PlayerTestGetSrc, src_getter) {
	FileSrc *src = new FileSrc("./player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	Player player(src);
	player.AddSink(sink);

	EXPECT_EQ(src, player.abstract_src());

	delete sink;
	delete src;
}

TEST(PlayerTestPlaybackSpeed, setting_playback_speed) {
	FileSrc *src = new FileSrc("./player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	Player player(src);
	player.AddSink(sink);
	player.set_playback_speed(0.9);

	EXPECT_NEAR(0.9, sink->playback_speed(), 1e-3);

	delete sink;
	delete src;
}

TEST(PulseSinkTestLinkage, test_of_linkage) {
	FileSrc *src = new FileSrc("./player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	EXPECT_FALSE(sink->linked());

	Player player(src);
	player.AddSink(sink);

	EXPECT_TRUE(sink->linked());

	player.RemoveSink(sink);

	EXPECT_FALSE(sink->linked());

	delete sink;
	delete src;
}

TEST(RingSrcTestDecrementRatio, decrementation_of_ratio) {
	RingSrc *src = new RingSrc(0.2);
	FakeSink *sink = new FakeSink();

	Player player(src);
	player.AddSink(sink);
	src->DecrementRatio(&player);

	EXPECT_LT(sink->playback_speed(), 1.0);

	delete sink;
	delete src;
}


TEST(RingSrcTestIncrementRatio, incrementation_of_ratio) {
	RingSrc *src = new RingSrc(0.2);
	FakeSink *sink = new FakeSink();

	Player player(src);
	player.AddSink(sink);
	src->IncrementRatio(&player);

	EXPECT_GT(sink->playback_speed(), 1.0);

	delete sink;
	delete src;
}

