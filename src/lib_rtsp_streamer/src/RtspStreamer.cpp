#include "RtspStreamer.h"

#include "CustomMediaFactory.h"
#include "GstCustomVideoSrc.h"

namespace {

template <typename... _Args>
inline void log(std::format_string<_Args...> __fmt, _Args&&... __args)
{
    g_log("CUSTOM_VIDEO_SRC",
          GLogLevelFlags::G_LOG_LEVEL_MESSAGE,
          "%s",
          std::vformat(__fmt.get(), std::make_format_args(__args...)).c_str());
}

using namespace rtsp_streamer;

std::tuple<GstElement*, GstCustomVideoSrc*>
create_custom_rtsp_pipeline(rtsp_streamer::FrameSource* frame_source, uint32_t bitrate)
{
    // Create the pipeline
    GstElement* pipeline = gst_pipeline_new("rtsp-pipeline");
    if (!pipeline) {
        g_printerr("Failed to create pipeline\n");
        return std::make_tuple(nullptr, nullptr);
    }

    GstCustomVideoSrc* video_src = nullptr;

    // Create elements
    auto src = gst_element_factory_make("gst_custom_video_src", "source");
    {
        video_src = GST_CUSTOMVIDEOSRC(src);
        video_src->_frame_source = frame_source;
    }

    GstElement* capsfilter1 = gst_element_factory_make("capsfilter", "raw-caps");
    GstElement* videoconvert = gst_element_factory_make("videoconvert", "converter");
    GstElement* capsfilter2 = gst_element_factory_make("capsfilter", "i420-caps");
    GstElement* encoder = gst_element_factory_make("x264enc", "encoder");
    GstElement* rtppay = gst_element_factory_make("rtph264pay", "pay0");

    // Check all elements were created
    if (!src || !capsfilter1 || !videoconvert || !capsfilter2 || !encoder || !rtppay) {
        g_printerr("Failed to create one or more elements\n");
        if (pipeline)
            gst_object_unref(pipeline);
        if (src)
            gst_object_unref(src);
        if (capsfilter1)
            gst_object_unref(capsfilter1);
        if (videoconvert)
            gst_object_unref(videoconvert);
        if (capsfilter2)
            gst_object_unref(capsfilter2);
        if (encoder)
            gst_object_unref(encoder);
        if (rtppay)
            gst_object_unref(rtppay);
        return std::make_tuple(nullptr, nullptr);
    }

    auto [width, height] = frame_source->get_frame_size();

    // Set properties
    // First capsfilter: video/x-raw,width=640,height=480,framerate=25/1,format=RGB
    GstCaps* caps1 = gst_caps_new_simple("video/x-raw",
                                         "width",
                                         G_TYPE_INT,
                                         width,
                                         "height",
                                         G_TYPE_INT,
                                         height,
                                         "framerate",
                                         GST_TYPE_FRACTION,
                                         25,
                                         1,
                                         "format",
                                         G_TYPE_STRING,
                                         "RGB",
                                         NULL);
    g_object_set(capsfilter1, "caps", caps1, NULL);
    gst_caps_unref(caps1);

    // Second capsfilter: video/x-raw,format=I420
    GstCaps* caps2 =
        gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", NULL);
    g_object_set(capsfilter2, "caps", caps2, NULL);
    gst_caps_unref(caps2);

    // x264enc properties
    g_object_set(encoder, "key-int-max", 1, NULL);

    /*tune: stillimage (0x00000001), fastdecode (0x00000002), zerolatency (0x00000004)*/
    // g_object_set(encoder, "tune", 1 NULL);

    //  Subpixel motion estimation and
    //  partition decision quality : 1 = fast, 10 = best */
    //  g_object_set(encoder, "subme", 10, NULL);

    /*
    "speed-preset":
    None (0) – No preset
    ultrafast (1) – ultrafast
    superfast (2) – superfast
    veryfast (3) – veryfast
    faster (4) – faster
    fast (5) – fast
    medium (6) – medium
    slow (7) – slow
    slower (8) – slower
    veryslow (9) – veryslow
    placebo (10) – placebo*/
    // g_object_set(encoder, "speed-preset", 6, NULL);

    /* Number of threads used by the codec (0 for automatic)*/
    // g_object_set(encoder, "threads", 0, NULL);


    // g_object_set(encoder, "pass", "qual", NULL);
    // g_object_set(encoder, "quantizer", "1", NULL);
    // g_object_set(encoder, "profile", "high", NULL);
    // g_object_set(encoder, "speed-preset", "veryslow", NULL);
    // g_object_set(encoder, "preset", "high-quality", NULL);
    // g_object_set(encoder, "rc-mode", "constqp", NULL);
    g_object_set(encoder, "bitrate", bitrate, NULL);
    // g_object_set(encoder, "qp-min", 18, NULL);


    // rtph264pay properties
    g_object_set(rtppay, "pt", 96, "name", "pay0", NULL);

    // Add all elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline),
                     src,
                     capsfilter1,
                     videoconvert,
                     capsfilter2,
                     encoder,
                     rtppay,
                     NULL);

    // Link elements
    if (!gst_element_link_many(
            src, capsfilter1, videoconvert, capsfilter2, encoder, rtppay, NULL)) {
        g_printerr("Failed to link elements\n");
        gst_object_unref(pipeline);
        return std::make_tuple(nullptr, nullptr);
    }

    return std::make_tuple(pipeline, video_src);
}

template <typename _T>
using gst_counted_ptr =
    std::unique_ptr<_T, decltype([](_T* ptr) { gst_object_unref(ptr); })>;

template <typename _T>
using gst_uncounted_ptr = std::unique_ptr<_T, decltype([](_T* ptr) { g_free(ptr); })>;


class RtspStreamerImpl : public RtspStreamer
{
    std::jthread _thread;
    rtsp_streamer::FrameSource* _frame_source;
    gst_counted_ptr<GstRTSPServer> _server;
    gst_counted_ptr<GstCustomVideoSrc> _source;
    gst_counted_ptr<GstElement> _pipeline;
    gst_uncounted_ptr<GMainLoop> _loop;
    gst_counted_ptr<GstRTSPMediaFactory> _factory;

public:
    RtspStreamerImpl() {}
    ~RtspStreamerImpl() override { stop(); }

private:
    void Join() override
    {
        if (_thread.joinable()) {
            _thread.join();
        }
    }

public:
    void start(uint16_t port,
               std::string const& mount_point,
               FrameSource* frame_source,
               uint32_t bitrate)
    {
        if (_thread.joinable()) {
            throw std::runtime_error("RTSP server is already running");
        }

        // Implementation of starting the RTSP server

        _frame_source = frame_source;


        /* create a server instance */
        _server.reset(gst_rtsp_server_new());
        g_object_ref(_server.get());

        g_object_set(_server.get(), "service", std::to_string(port).c_str(), NULL);

        /* get the mount points for this server, every server has a default object
         * that be used to map uri mount points to media factories */
        gst_counted_ptr<GstRTSPMountPoints> mounts(
            gst_rtsp_server_get_mount_points(_server.get()));

        /* make a media factory for a test stream. The default media factory can use
         * gst-launch syntax to create pipelines.
         * any launch line works as long as it contains elements named pay%d. Each
         * element with pay%d names will be a stream */

        if constexpr (0) {
            // For testing purposes
            _factory.reset(gst_rtsp_media_factory_new());
            g_object_ref(_factory.get());
            gst_rtsp_media_factory_set_launch(
                _factory.get(),
                "( videotestsrc pattern=smpte ! "
                "video/x-raw,width=640,height=480,framerate=30/1 "
                "! x264enc key-int-max=30 tune=zerolatency ! h264parse "
                "! rtph264pay config-interval=-1 pt=96 name=pay0 )");
        } else {

            auto [pipeline_, source] =
                create_custom_rtsp_pipeline(_frame_source, bitrate);
            if (!pipeline_ || !source) {
                throw std::runtime_error("Failed to create pipeline");
            }
            _pipeline.reset(pipeline_);
            g_object_ref(_pipeline.get());
            _source.reset(source);
            g_object_ref(_source.get());
            _source->_eos.clear();

            _factory.reset(gst_custom_video_src_media_factory_new(pipeline_));

            g_object_ref(_factory.get());
        }

        gst_rtsp_media_factory_set_shared(_factory.get(), TRUE);

        /* attach the factory to the mount_point url */
        gst_rtsp_mount_points_add_factory(
            mounts.get(), mount_point.c_str(), _factory.get());

        /* don't need the ref to the mapper anymore */
        mounts.reset();

        /* attach the server to the default maincontext */
        gst_rtsp_server_attach(_server.get(), NULL);

        _loop.reset(g_main_loop_new(NULL, FALSE));

        /* start serving */
        log("stream ready at rtsp://127.0.0.1:{}{}\n", port, mount_point.c_str());

        _thread = std::jthread([this]() {
            // This is where you would set up the server and start streaming
            run();
        });
    }

    void stop()
    {
        assert(_thread.joinable());
        // Implementation of stopping the RTSP server
        assert(_source);
        _source->_eos.test_and_set();

        gst_element_send_event(_pipeline.get(), gst_event_new_eos());
        gst_element_set_state(_pipeline.get(), GST_STATE_NULL);
        g_main_loop_quit(_loop.get());

        _thread.join();
        _source->_eos.clear();
    }

private:
    void run()
    {
        // Implementation of the RTSP server loop
        // This is where you would set up the server and start streaming
        g_main_loop_run(_loop.get());
    }
};
} // namespace

extern "C" void gst_init_static_plugins();


// #pragma message "GStreamer static linking : " GSTREAMER_STATIC_LINKING

#if GSTREAMER_STATIC_LINKING == 1
extern "C" {
#include "/path_to/config.h"
#include "/path_to/gstinitstaticplugins.c"
}
#endif

using void_fn_t = void (*)(void);
using gst_init_fn_t = void (*)(int* argc, char** argv[]);

namespace rtsp_streamer {

void init(int argc, char** argv)
{
    if (!gst_is_initialized()) {
        gst_init(&argc, &argv);


#if GSTREAMER_STATIC_LINKING == 1
        gst_init_static_plugins();
#endif

        if constexpr (0) {

            if constexpr (0) {

                GError* err = nullptr;

                // Load a plugin from a specific .so file
                GstPlugin* plugin = gst_plugin_load_file(
                    //"/home/rukhov/projects/github/vcpkg/buildtrees/gstreamer/x64-linux-dbg/"
                    //"/home/rukhov/projects/github/vcpkg/buildtrees/gstreamer/x64-linux-rel/"
                    "./libgstreamer-full-1.0.so", // Path to the .so file
                    &err                          // Optional error tracking
                );

                if (!plugin || err) {
                    log("Failed to load plugin!: {}", err->message);
                }

                auto filename = "./libgstreamer-full-1.0.so";

                auto module = g_module_open(filename, G_MODULE_BIND_LOCAL);

                {
                    auto symname = "gst_init";

                    gst_init_fn_t gst_init_ptr = nullptr;

                    auto ret = g_module_symbol(module, symname, (void**)&gst_init_ptr);

                    gst_init_ptr(&argc, &argv);
                }
                {
                    auto symname = "gst_init_static_plugins";

                    void_fn_t gst_init_static_plugins_ptr = nullptr;

                    auto ret = g_module_symbol(
                        module, symname, (void**)&gst_init_static_plugins_ptr);

                    gst_init_static_plugins_ptr();
                }
            }

            auto vtst_factory = gst_element_factory_find("videotestsrc");

            auto vtst_element =
                gst_element_factory_make("videotestsrc", "my_videotestsrc");
        }
    }
    gst_custom_video_src_register();
}

std::unique_ptr<RtspStreamer> make_streamer(uint16_t port,
                                            std::string const& mount_point,
                                            FrameSource& frame_source,
                                            uint32_t bitrate)
{
    auto streamer = std::make_unique<RtspStreamerImpl>();
    streamer->start(port, mount_point, &frame_source, bitrate);
    return streamer;
}
} // namespace rtsp_streamer