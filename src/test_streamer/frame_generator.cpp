#include "frame_generator.h"

#include <lunasvg.h>

namespace {

class SimpleFrameGen : public rtsp_streamer::FrameSource
{
#pragma pack(push, 1)
    struct _RGB {
        uint8_t r, g, b;
    };
#pragma pack(pop)

    GstVideoInfo _format;
    uint64_t _frame_counter = 0;
    std::vector<_RGB> _image;
    std::vector<std::vector<_RGB>> _frames;

public:
    SimpleFrameGen()
    {
        _format.width = 480;
        _format.height = 360;
    }
    ~SimpleFrameGen() {}

    static void render_colors_frame(std::vector<_RGB>& frame,
                                    size_t width,
                                    size_t height,
                                    uint64_t parameter)
    {
        assert(frame.size() == width * height);

        auto shift = parameter % 255;

        // Fill the image with a gradient
        for (size_t x = 0; x < width; ++x) {

            auto fx = uint8_t((255 - shift + (x * 255 / width)) % 255);

            for (size_t y = 0; y < height; ++y) {

                auto fy = uint8_t((shift + (y * 255 / height)) % 255);

                auto& pixel = frame[x + y * width];
                pixel.r += fx;
                pixel.g += fy;
                pixel.b += uint8_t(255 - fy);
            }
        }
    }

    static void render_grid_frame(std::vector<_RGB>& frame,
                                  size_t width,
                                  size_t height,
                                  uint64_t parameter)
    {
        assert(frame.size() == width * height);

        auto const grid_step = 40;

        for (size_t x = 0; x < width; ++x) {

            for (size_t y = 0; y < height; ++y) {

                int grid =
                    (x + parameter) % grid_step == 0 || (y + parameter) % grid_step == 0
                        ? 0
                        : 1;

                auto& pixel = frame[x + y * width];

                pixel.r *= grid;
                pixel.g *= grid;
                pixel.b *= grid;
            }
        }
    }

    static void render_svg_frame(std::vector<_RGB>& frame,
                                 size_t width,
                                 size_t height,
                                 uint64_t parameter)
    {
        assert(frame.size() == width * height);

        auto doc = lunasvg::Document::loadFromData(
            R"(<svg width="256px" height="256px" viewBox="0 0 32 32" enable-background="new 0 0 32 32" version="1.1" xml:space="preserve" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" fill="#000000"><g id="SVGRepo_bgCarrier" stroke-width="0"></g><g id="SVGRepo_tracerCarrier" stroke-linecap="round" stroke-linejoin="round"></g><g id="SVGRepo_iconCarrier"> <g id="_x36_0"></g> <g id="_x35_9"></g> <g id="_x35_8"></g> <g id="_x35_7"></g> <g id="_x35_6"></g> <g id="_x35_5"></g> <g id="_x35_4"></g> <g id="_x35_3"></g> <g id="_x35_2"></g> <g id="_x35_1"></g> <g id="_x35_0"></g> <g id="_x34_9"></g> <g id="_x34_8"></g> <g id="_x34_7"></g> <g id="_x34_6"></g> <g id="_x34_5"></g> <g id="_x34_4"></g> <g id="_x34_3"></g> <g id="_x34_2"></g> <g id="_x34_1"></g> <g id="_x34_0"></g> <g id="_x33_9"></g> <g id="_x33_8"></g> <g id="_x33_7"></g> <g id="_x33_6"></g> <g id="_x33_5"></g> <g id="_x33_4"></g> <g id="_x33_3"></g> <g id="_x33_2"></g> <g id="_x33_1"></g> <g id="_x33_0"></g> <g id="_x32_9"></g> <g id="_x32_8"></g> <g id="_x32_7"></g> <g id="_x32_6"></g> <g id="_x32_5"></g> <g id="_x32_4_1_"></g> <g id="_x32_3"></g> <g id="_x32_2"></g> <g id="_x32_1"></g> <g id="_x32_0"></g> <g id="_x31_9"></g> <g id="_x31_8"></g> <g id="_x31_7"></g> <g id="_x31_6"></g> <g id="_x31_5"></g> <g id="_x31_4"></g> <g id="_x31_3"></g> <g id="_x31_2"></g> <g id="_x31_1"></g> <g id="_x31_0"></g> <g id="_x39_"></g> <g id="_x38_"></g> <g id="_x37_"></g> <g id="_x36_"></g> <g id="_x35_"></g> <g id="_x34_"></g> <g id="_x33_"></g> <g id="_x32_"></g> <g id="_x31_"> <g> <rect fill="#406AB5" height="5" width="10" x="7" y="7"></rect> <rect fill="#FCDF65" height="5" width="10" x="7" y="12"></rect> <rect fill="#406AB5" height="5" width="10" x="17" y="9"></rect> <rect fill="#FCDF65" height="5" width="10" x="17" y="14"></rect> </g> <path d="M27,8h-9V7c0-0.5522461-0.4477539-1-1-1H8V4c0-0.5522461-0.4477539-1-1-1S6,3.4477539,6,4v24 c0,0.5522461,0.4477539,1,1,1s1-0.4477539,1-1V18h8v1c0,0.5522461,0.4477539,1,1,1h6c0.5522461,0,1-0.4477539,1-1s-0.4477539-1-1-1 h-5v-3h8v4c0,0.5522461,0.4477539,1,1,1s1-0.4477539,1-1V9C28,8.4477539,27.5522461,8,27,8z M16,8v3H8V8H16z M8,16v-3h8v3H8z M18,10h8v3h-8V10z" fill="#4D4D4D"></path> </g> <g id="Guides"></g> </g></svg>)");

        lunasvg::Bitmap bitmap{ (int)width, (int)height };

        const auto stride = bitmap.stride();

        assert(stride == width * 4);

        doc->render(bitmap);

        for (size_t x = 0; x < width; ++x) {
            for (size_t y = 0; y < height; ++y) {

                auto& pixel = frame[x + y * width];

                auto const* src = bitmap.data() + y * stride + x * 4;

                if (src[3] > 0) {
                    pixel.b = src[0];
                    pixel.g = src[1];
                    pixel.r = src[2];
                }
            }
        }
    }

    void render_frames()
    {
        for (int i = 0; i < 10; ++i) {
            _frames.emplace_back(_format.width * _format.height);
            render_colors_frame(_frames.back(), _format.width, _format.height, i * 30);
            render_grid_frame(_frames.back(), _format.width, _format.height, i * 2);
            render_svg_frame(_frames.back(), _format.width, _format.height, i);
        }
    }

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

        render_frames();
    }


    bool is_eof() const override { return false; }

    std::tuple<size_t /*width*/, size_t /*height*/> get_fraem_size() const override
    {
        return { _format.width, _format.height };
    }

    double get_pixel_ratio() const override { return 1.0; }

    size_t get_ftrame_buffer_length() const override
    {
        return _format.width * _format.height * 3;
    }

    std::span<uint8_t> get_next_frame() override
    {
        assert(get_ftrame_buffer_length() > 0);

        auto frame_num = _frame_counter % _frames.size();

        ++_frame_counter;

        return { (uint8_t*)_frames[frame_num].data(), get_ftrame_buffer_length() };
    }
};

} // namespace

std::unique_ptr<rtsp_streamer::FrameSource> make_frame_source()
{
    return std::make_unique<SimpleFrameGen>();
}