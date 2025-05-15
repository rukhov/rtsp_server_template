#pragma once

#include <gst/video/video-info.h>

namespace rtsp_streamer {

void init(int argc, char** argv);

class FrameSource
{
public:
    enum class Format {
        RGB,
    };

    virtual ~FrameSource() = default;
    virtual void set_format(GstVideoInfo const& format) = 0;
    virtual std::tuple<size_t /*width*/, size_t /*height*/> get_frame_size() const = 0;
    virtual double /* width / heigth */ get_pixel_ratio() const = 0;
    virtual size_t stride() const = 0; // stride size in bytes
    virtual bool is_eof() const = 0;
    virtual std::span<uint8_t> get_next_frame() = 0;
};

class RtspStreamer
{
public:
    virtual ~RtspStreamer() = default;
    virtual void Join() = 0;
};

std::unique_ptr<RtspStreamer> make_streamer(uint16_t port,
                                            std::string const& mount_point,
                                            FrameSource& frame_source,
                                            uint32_t bitrate = 2048);

} // namespace rtsp_streamer
