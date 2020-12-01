#include "gtest/gtest.h"

#include "engine/transport.cpp"
#include "library/rt_event_fifo.h"

using namespace sushi;
using namespace sushi::engine;

constexpr auto ZERO_TIMEOUT = std::chrono::milliseconds(0);
constexpr float TEST_SAMPLERATE = 48000;

class TestTransport : public ::testing::Test
{
protected:
    TestTransport()
    {
    }

    void SetUp()
    {}

    void TearDown()
    { }
    RtEventFifo<10> _rt_event_output;
    Transport _module_under_test{TEST_SAMPLERATE, &_rt_event_output};
};


TEST_F(TestTransport, TestBasicQuerying)
{
    _module_under_test.set_latency(std::chrono::microseconds(1500));
    _module_under_test.set_time(std::chrono::seconds(1), 44800);

    EXPECT_EQ(std::chrono::microseconds(1001500), _module_under_test.current_process_time());

    _module_under_test.set_tempo(130, false);
    _module_under_test.set_time(std::chrono::seconds(0), 0);
    EXPECT_FLOAT_EQ(130, _module_under_test.current_tempo());

    _module_under_test.set_time_signature({5, 8}, false);
    EXPECT_EQ(5, _module_under_test.time_signature().numerator);
    EXPECT_EQ(8, _module_under_test.time_signature().denominator);
}

TEST_F(TestTransport, TestTimeline44Time)
{
    constexpr int TEST_SAMPLERATE_X2 = 32768;
    /* Odd samplerate, but it's a convenient factor of 2 which makes testing easier
     * as bar boundaries end up on a power of 2 samplecount if AUDIO_CHUNK_SIZE is
     * a power of 2*/
    _module_under_test.set_sample_rate(TEST_SAMPLERATE_X2);
    _module_under_test.set_time_signature({4, 4}, false);
    _module_under_test.set_tempo(120, false);
    _module_under_test.set_playing_mode(PlayingMode::PLAYING, false);
    _module_under_test.set_time(std::chrono::seconds(0), 0);

    /* Check that the starting point is 0 */
    EXPECT_DOUBLE_EQ(0.0, _module_under_test.current_bar_beats());
    EXPECT_DOUBLE_EQ(0.0, _module_under_test.current_beats());
    EXPECT_DOUBLE_EQ(0.0, _module_under_test.current_bar_start_beats());
    EXPECT_DOUBLE_EQ(0.0, _module_under_test.current_bar_beats(0));
    EXPECT_DOUBLE_EQ(0.0, _module_under_test.current_beats(0));

    /* Advance time by 1 second equal to 1/2 bar at 120 bpm */
    _module_under_test.set_time(std::chrono::seconds(1), TEST_SAMPLERATE_X2 );
    EXPECT_DOUBLE_EQ(2.0, _module_under_test.current_bar_beats());
    EXPECT_DOUBLE_EQ(2.0, _module_under_test.current_beats());
    EXPECT_DOUBLE_EQ(0.0, _module_under_test.current_bar_start_beats());

    /* Test also that offset works correctly */
    EXPECT_DOUBLE_EQ(3.0, _module_under_test.current_bar_beats(TEST_SAMPLERATE_X2 / 2));
    EXPECT_DOUBLE_EQ(4.0, _module_under_test.current_beats(TEST_SAMPLERATE_X2));

    /* Advance time by 1.5 second equal to 3/4 bar at 120 bpm  which should bring us
     * in to the next bar*/
    _module_under_test.set_time(std::chrono::milliseconds(2500), 5 * TEST_SAMPLERATE_X2 / 2);
    EXPECT_DOUBLE_EQ(1.0, _module_under_test.current_bar_beats());
    EXPECT_DOUBLE_EQ(5.0, _module_under_test.current_beats());
    EXPECT_DOUBLE_EQ(4.0, _module_under_test.current_bar_start_beats());
}

TEST_F(TestTransport, TestTimeline68Time)
{
    /* Test the above but with different time signature and samplerate */
    _module_under_test.set_sample_rate(TEST_SAMPLERATE);
    _module_under_test.set_tempo(180, false);
    _module_under_test.set_time_signature({6, 8}, false);
    constexpr float precision = 4.0f * AUDIO_CHUNK_SIZE / TEST_SAMPLERATE;

    /* Check that the starting point is 0 */
    _module_under_test.set_playing_mode(PlayingMode::PLAYING, false);
    _module_under_test.set_time(std::chrono::seconds(0), 0);
    EXPECT_DOUBLE_EQ(0.0f, _module_under_test.current_bar_beats());
    EXPECT_DOUBLE_EQ(0.0f, _module_under_test.current_beats());
    EXPECT_DOUBLE_EQ(0.0f, _module_under_test.current_bar_start_beats());

    /* Advance time by 1/2 second equal to 1/2 bar at 180 bpm. Cant test exact
     * values here since 48000 is not an even multiple of AUDIO_CHUNK_SIZE */
    _module_under_test.set_time(std::chrono::milliseconds(500), TEST_SAMPLERATE / 2);
    EXPECT_NEAR(1.5, _module_under_test.current_bar_beats(), precision);
    EXPECT_NEAR(1.5, _module_under_test.current_beats(), precision);
    EXPECT_NEAR(0.0, _module_under_test.current_bar_start_beats(), precision);

    /* Advance time by 1 second equal to 1 bar at 180 bpm
     * which should bring us halfway  in to the next bar */
    _module_under_test.set_time(std::chrono::milliseconds(1500), 3 * TEST_SAMPLERATE / 2);
    EXPECT_NEAR(1.5, _module_under_test.current_bar_beats(), precision);
    EXPECT_NEAR(4.5, _module_under_test.current_beats(), precision);
    EXPECT_NEAR(3.0, _module_under_test.current_bar_start_beats(), precision);
}

TEST_F(TestTransport, TestPlayStateChange)
{
    _module_under_test.set_sample_rate(TEST_SAMPLERATE);
    _module_under_test.set_time_signature({4, 4}, false);
    _module_under_test.set_tempo(120, false);
    _module_under_test.set_playing_mode(PlayingMode::STOPPED, false);
    _module_under_test.set_sync_mode(SyncMode::INTERNAL, false);
    _module_under_test.set_time(std::chrono::seconds(0), 0);

    EXPECT_FALSE(_module_under_test.playing());
    EXPECT_EQ(PlayStateChange::UNCHANGED, _module_under_test.current_state_change());

    _module_under_test.set_time(std::chrono::seconds(1), 44000);
    EXPECT_FALSE(_module_under_test.playing());
    EXPECT_EQ(PlayStateChange::UNCHANGED, _module_under_test.current_state_change());

    _module_under_test.set_playing_mode(PlayingMode::PLAYING, false);
    _module_under_test.set_time(std::chrono::seconds(2), 88000);
    EXPECT_TRUE(_module_under_test.playing());
    EXPECT_EQ(PlayStateChange::STARTING, _module_under_test.current_state_change());

    _module_under_test.set_time(std::chrono::seconds(3), 132000);
    EXPECT_TRUE(_module_under_test.playing());
    EXPECT_EQ(PlayStateChange::UNCHANGED, _module_under_test.current_state_change());
}