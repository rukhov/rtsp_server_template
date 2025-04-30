#pragma once

#include <rtsp_streamer/RtspStreamer.h>

std::unique_ptr<rtsp_streamer::FrameSource> make_frame_source();
