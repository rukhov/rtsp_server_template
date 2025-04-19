#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <iostream>
#include <string>

#include <rtsp_streamer/RtspStreamer.h>

#include "picture_gen.h"

int main(int argc, char* argv[])
{
    rtsp_streamer::init(argc, argv);

    auto streamer =
        rtsp_streamer::make_rtsp_streamer(8554, "/custom_video_src", make_PictureGen());

    // std::this_thread::sleep_for(std::chrono::seconds(10));

    streamer->Join();

    return 0;
}
