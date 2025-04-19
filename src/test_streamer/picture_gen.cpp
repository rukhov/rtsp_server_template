
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "picture_gen.h"


namespace {

template <typename... _Args>
inline void log(std::format_string<_Args...> __fmt, _Args&&... __args)
{
    g_log("GST_COLOR_BARS",
          GLogLevelFlags::G_LOG_LEVEL_MESSAGE,
          "%s",
          std::vformat(__fmt.get(), std::make_format_args(__args...)).c_str());
}

#pragma pack(push, 1)
struct _RGB {

    uint8_t r;
    uint8_t g;
    uint8_t b;
};
#pragma pack(pop)

class PictureGenImpl : public rtsp_streamer::FrameSource
{
    GstVideoInfo _format;
    size_t _frame_counter = 0;
    cv::Mat _image;

public:
    PictureGenImpl() {}
    ~PictureGenImpl() {}

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

        _image = cv::Mat(
            _format.height, _format.width, CV_8UC3, cv::Scalar_<uint8_t>(255, 255, 255));
    }

    bool is_eof() const override { return false; }

    size_t get_ftrame_size() const override { return _format.width * _format.height * 3; }

    std::span<uint8_t> get_next_frame() override
    {
        assert(get_ftrame_size() > 0);

        auto shift = _frame_counter % 255;

        for (size_t x = 0; x < _format.width; ++x) {

            auto fx = uint8_t((255 - shift + (x * 255 / _format.width)) % 255);

            for (size_t y = 0; y < _format.height; ++y) {

                auto fy = uint8_t((shift + (y * 255 / _format.height)) % 255);

                _image.at<cv::Vec3b>(y, x) = cv::Vec3b{ fx, fy, uint8_t(255 - fy) };
            }
        }

        if constexpr (1) {
            // Draw text
            std::string text = std::format("{}", std::chrono::system_clock::now());
            cv::Point textOrg(10, _format.height - 10);
            int fontFace = cv::FONT_HERSHEY_SCRIPT_COMPLEX;
            double fontScale = 1.0;
            int thickness = 2;
            cv::putText(_image,
                        text,
                        textOrg,
                        fontFace,
                        fontScale,
                        cv::Vec3b{ 0, 0, 0 },
                        thickness,
                        cv::LINE_AA);
        }

        ++_frame_counter;

        return { _image.data, get_ftrame_size() };
    }
};
} // namespace

std::unique_ptr<rtsp_streamer::FrameSource> make_PictureGen()
{
    return std::make_unique<PictureGenImpl>();
}
