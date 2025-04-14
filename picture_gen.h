#pragma once

#include <gst/video/gstvideometa.h>

namespace gst {
class PictureGen
{
public:
    enum class Format {
        RGB,
    };

    virtual ~PictureGen() = default;
    virtual size_t get_ftrame_size() const = 0;
    virtual std::span<uint8_t> get_next_frame() = 0;
};

PictureGen* make_PictureGen(GstVideoInfo const& format);
} // namespace gst