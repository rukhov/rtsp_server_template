#pragma once

#include <gst/video/gstvideometa.h>

#include <rtsp_streamer/RtspStreamer.h>

std::unique_ptr<rtsp_streamer::FrameSource> make_PictureGen();
