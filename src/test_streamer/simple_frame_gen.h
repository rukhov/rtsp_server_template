#pragma once

#include <rtsp_streamer/RtspStreamer.h>

class SimpleFrameGen : public rtsp_streamer::FrameSource
{
#pragma pack(push, 1)
    struct _RGB {

        uint8_t r;
        uint8_t g;
        uint8_t b;
    };
#pragma pack(pop)

    GstVideoInfo _format;
    size_t _frame_counter = 0;
    std::vector<_RGB> _image;

public:
    SimpleFrameGen() {}
    ~SimpleFrameGen() {}

    // PictureGen
private:
    void set_format(GstVideoInfo const& format) override
    {
        assert(format.finfo->format == GST_VIDEO_FORMAT_RGB);
        assert(format.finfo->bits == 8);
        assert(format.finfo->n_components == 3);
        assert(format.finfo->depth[0] == 8 && format.finfo->depth[1] == 8 &&
               format.finfo->depth[2] == 8 && format.finfo->depth[3] == 0);

        _format = format;

        _image.resize(_format.width * _format.height);
    }

    bool is_eof() const override { return false; }

    size_t get_ftrame_size() const override { return _format.width * _format.height * 3; }

    std::span<uint8_t> get_next_frame() override
    {
        assert(get_ftrame_size() > 0);

        auto shift = _frame_counter % 255;

        // Fill the image with a gradient
        for (size_t x = 0; x < _format.width; ++x) {

            auto fx = uint8_t((255 - shift + (x * 255 / _format.width)) % 255);

            for (size_t y = 0; y < _format.height; ++y) {

                auto fy = uint8_t((shift + (y * 255 / _format.height)) % 255);

                _image[x + y * _format.width] = _RGB{ fx, fy, uint8_t(255 - fy) };
            }
        }

        ++_frame_counter;

        return { (uint8_t*)_image.data(), get_ftrame_size() };
    }
};
