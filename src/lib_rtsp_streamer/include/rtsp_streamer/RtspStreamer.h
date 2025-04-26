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
    virtual std::tuple<size_t /*width*/, size_t /*height*/> get_fraem_size() const = 0;
    virtual double /* width / heigth */ get_pixel_ratio() const = 0;
    virtual size_t get_ftrame_buffer_length() const = 0;
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
                                            std::shared_ptr<FrameSource> frame_source);

} // namespace rtsp_streamer
