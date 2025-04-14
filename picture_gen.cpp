#include <gst/gst.h>
#include <format>
#include <iostream>
#include <string>

#include "picture_gen.h"


namespace {

template <typename... _Args>
inline void log(std::format_string<_Args...> __fmt, _Args&&... __args)
{
    g_log("GST_COLOR_BARS",
          GLogLevelFlags::G_LOG_LEVEL_MESSAGE,
          "%s",
          std::vformat(__fmt.get(), std::make_format_args(__args...)).c_str());
    // g_print(">>>>>>>>>>>: %s", std::vformat(__fmt.get(),
    // std::make_format_args(__args...)).c_str());
}

#pragma pack(push, 1)
struct _RGB {

    uint8_t r;
    uint8_t g;
    uint8_t b;
};
#pragma pack(pop)

using namespace gst;

class PictureGenImpl : public PictureGen
{
    const GstVideoInfo _format;
    std::vector<uint8_t> _farme_buffer;
    size_t _frame_counter = 0;

public:
    PictureGenImpl(GstVideoInfo const& format) : _format(format)
    {
        log("GstColorBarsImpl::GstColorBarsImpl()");

        assert(_format.finfo->format == GST_VIDEO_FORMAT_RGB);
        assert(_format.finfo->bits == 8);
        assert(_format.finfo->n_components == 3);
        assert(_format.finfo->depth[0] == 8 && _format.finfo->depth[1] == 8 &&
               _format.finfo->depth[2] == 8 && _format.finfo->depth[3] == 0);

        _farme_buffer.resize(get_ftrame_size());
    }

    ~PictureGenImpl() override { log("PictureGenImpl::~PictureGenImpl()"); }

    // PictureGen
private:
    size_t get_ftrame_size() const override { return _format.width * _format.height * 3; }

    std::span<uint8_t> get_next_frame() override
    {
        // if (_frame_counter % 10 == 0) log("PictureGenImpl::get_next_frame() frame: {}",
        // _frame_counter);

        auto shift = _frame_counter % 255;
        auto frame = (_RGB*)_farme_buffer.data();

        for (size_t x = 0; x < _format.width; ++x) {

            auto fx = uint8_t((shift + (x * 255 / _format.width)) % 255);

            for (size_t y = 0; y < _format.height; ++y) {

                auto fy = uint8_t((shift + (y * 255 / _format.height)) % 255);

                frame[x + y * _format.width] = _RGB{ fx, fy, uint8_t(255 - fy) };
            }
        }

        ++_frame_counter;

        return { _farme_buffer.data(), _farme_buffer.size() };
    }
};
} // namespace

namespace gst {

PictureGen* make_PictureGen(GstVideoInfo const& format)
{
    return new PictureGenImpl(format);
}
} // namespace gst