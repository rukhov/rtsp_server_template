#include <rtsp_streamer/RtspStreamer.h>

#include "frame_generator.h"

int main(int argc, char* argv[])
{
    rtsp_streamer::init(argc, argv);

    auto frame_source = make_frame_source();

    auto streamer = rtsp_streamer::make_streamer(8554, "/atv", *frame_source);

    streamer->Join();

    return 0;
}
