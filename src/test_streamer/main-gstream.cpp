#include <rtsp_streamer/RtspStreamer.h>

#include "simple_frame_gen.h"

int main(int argc, char* argv[])
{
    rtsp_streamer::init(argc, argv);

    auto streamer = rtsp_streamer::make_rtsp_streamer(
        8554, "/custom_video_src", std::make_unique<SimpleFrameGen>());

    streamer->Join();

    return 0;
}
