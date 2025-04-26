#include <rtsp_streamer/RtspStreamer.h>

#include "simple_frame_gen.h"

int main(int argc, char* argv[])
{
    rtsp_streamer::init(argc, argv);

    SimpleFrameGen frame_source;

    auto streamer = rtsp_streamer::make_streamer(8554, "/custom_video_src", frame_source);

    streamer->Join();

    return 0;
}
