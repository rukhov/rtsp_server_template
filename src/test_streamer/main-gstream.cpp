#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <iostream>
#include <string>

#include <rtsp_streamer/RtspStreamer.h>

#include "picture_gen.h"

#define DEFAULT_RTSP_PORT 8554
#define DEFAULT_MOUNT_POINT "/custom_video_src"

int main(int argc, char* argv[])
{
    rtsp_streamer::init(argc, argv);

    {
        auto streamer = rtsp_streamer::make_rtsp_streamer(
            DEFAULT_RTSP_PORT, DEFAULT_MOUNT_POINT, make_PictureGen());
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    {
        auto streamer = rtsp_streamer::make_rtsp_streamer(
            DEFAULT_RTSP_PORT + 1, DEFAULT_MOUNT_POINT, make_PictureGen());
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}
