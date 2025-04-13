
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/gst.h>
#include <gst/gstvalue.h>
#include <gst/base/gstpushsrc.h>
#include <gst/video/gstvideopool.h>

#include "GstColorBarsVideoSrc.h"
#include "gst-color-bars.h"

constexpr int MAX_SUPPORTED_WIDTH = 1920;
constexpr int MAX_SUPPORTED_HEIGHT = 1080;
constexpr double MAX_SUPPORTED_FRAMERATE = 320.0;

GST_DEBUG_CATEGORY_STATIC(gst_color_bars_video_src_debug);
#define GST_CAT_DEFAULT gst_color_bars_video_src_debug

template <typename... _Args>
inline void log(std::format_string<_Args...> __fmt, _Args &&...__args)
{
    g_log("GST_COLOR_BARS", GLogLevelFlags::G_LOG_LEVEL_MESSAGE, "%s", std::vformat(__fmt.get(), std::make_format_args(__args...)).c_str());
    // g_print(">>>>>>>>>>>: %s", std::vformat(__fmt.get(), std::make_format_args(__args...)).c_str());
}

/* Filter signals and args */
enum
{
    /* FILL ME */
    LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_SILENT
};

#define _VIDEO_FORMATS_STR "A444_16LE, A444_16BE, AYUV64, RGBA64_LE, "                                  \
                           "ARGB64, ARGB64_LE, BGRA64_LE, ABGR64_LE, RGBA64_BE, ARGB64_BE, BGRA64_BE, " \
                           "ABGR64_BE, A422_16LE, A422_16BE, A420_16LE, A420_16BE, A444_12LE, "         \
                           "GBRA_12LE, A444_12BE, GBRA_12BE, Y412_LE, Y412_BE, A422_12LE, A422_12BE, "  \
                           "A420_12LE, A420_12BE, A444_10LE, GBRA_10LE, A444_10BE, GBRA_10BE, "         \
                           "A422_10LE, A422_10BE, A420_10LE, A420_10BE, BGR10A2_LE, RGB10A2_LE, Y410, " \
                           "A444, GBRA, AYUV, VUYA, RGBA, RBGA, ARGB, BGRA, ABGR, A422, A420, AV12, "   \
                           "Y444_16LE, GBR_16LE, Y444_16BE, GBR_16BE, v216, P016_LE, P016_BE, "         \
                           "Y444_12LE, GBR_12LE, Y444_12BE, GBR_12BE, I422_12LE, I422_12BE, Y212_LE, "  \
                           "Y212_BE, I420_12LE, I420_12BE, P012_LE, P012_BE, Y444_10LE, GBR_10LE, "     \
                           "Y444_10BE, GBR_10BE, r210, I422_10LE, I422_10BE, NV16_10LE32, Y210, UYVP, " \
                           "v210, I420_10LE, I420_10BE, P010_10LE, NV12_10LE40, NV12_10LE32, "          \
                           "P010_10BE, MT2110R, MT2110T, NV12_10BE_8L128, NV12_10LE40_4L4, Y444, "      \
                           "BGRP, GBR, RGBP, NV24, v308, IYU2, RGBx, xRGB, BGRx, xBGR, RGB, BGR, "      \
                           "Y42B, NV16, NV61, YUY2, YVYU, UYVY, VYUY, I420, YV12, NV12, NV21, "         \
                           "NV12_16L32S, NV12_32L32, NV12_4L4, NV12_64Z32, NV12_8L128, Y41B, IYU1, "    \
                           "YUV9, YVU9, BGR16, RGB16, BGR15, RGB15, RGB8P, GRAY16_LE, GRAY16_BE, "      \
                           "GRAY10_LE32, GRAY8"

#define VIDEO_FORMATS_STR "I420, RGB"

#define VIDEO_FORMATS_ALL "{ " VIDEO_FORMATS_STR " }"

#define VTS_VIDEO_CAPS GST_VIDEO_CAPS_MAKE(VIDEO_FORMATS_ALL) "," \
                                                              "width=640,height=480,framerate=25/1"

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE("src",
                                                                  GST_PAD_SRC,
                                                                  GST_PAD_ALWAYS,
                                                                  GST_STATIC_CAPS(VTS_VIDEO_CAPS));

#define gst_color_bars_video_src_parent_class parent_class
G_DEFINE_TYPE(GstColorBarsVideoSrc, gst_color_bars_video_src, GST_TYPE_PUSH_SRC);

GST_ELEMENT_REGISTER_DEFINE(gst_color_bars_video_src, "gst_color_bars_video_src", GST_RANK_NONE,
                            GST_TYPE_GSTCOLORBARSVIDEOSRC);

static void gst_color_bars_video_src_set_property(GObject *object,
                                                  guint prop_id, const GValue *value, GParamSpec *pspec);
static void gst_color_bars_video_src_get_property(GObject *object,
                                                  guint prop_id, GValue *value, GParamSpec *pspec);

static gboolean gst_color_bars_video_src_sink_event(GstPad *pad,
                                                    GstObject *parent, GstEvent *event);
static GstFlowReturn gst_color_bars_video_src_chain(GstPad *pad,
                                                    GstObject *parent, GstBuffer *buf);

/* GObject vmethod implementations */

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_color_bars_video_src_sink_event(GstPad *pad, GstObject *parent,
                                    GstEvent *event)
{
    log("GstColorBarsVideoSrc::_sink_event");
    GstColorBarsVideoSrc *filter;
    gboolean ret;

    filter = GST_GSTCOLORBARSVIDEOSRC(parent);

    GST_LOG_OBJECT(filter, "Received %s event: %" GST_PTR_FORMAT,
                   GST_EVENT_TYPE_NAME(event), event);

    switch (GST_EVENT_TYPE(event))
    {
    case GST_EVENT_CAPS:
    {
        GstCaps *caps;

        gst_event_parse_caps(event, &caps);
        /* do something with the caps */

        /* and forward */
        ret = gst_pad_event_default(pad, parent, event);
        break;
    }
    default:
        ret = gst_pad_event_default(pad, parent, event);
        break;
    }
    return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_color_bars_video_src_chain(GstPad *pad, GstObject *parent, GstBuffer *buf)
{
    log("GstColorBarsVideoSrc::_chain");
    GstColorBarsVideoSrc *filter;

    filter = GST_GSTCOLORBARSVIDEOSRC(parent);

    if (filter->silent == FALSE)
        log("I'm plugged, therefore I'm in.");

    /* just push out the incoming buffer without touching it */
    return gst_pad_push(filter->srcpad, buf);
}

/* gstreamer looks for this structure to register gstcolorbarsvideosrcs
 *
 * exchange the string 'Template gstcolorbarsvideosrc' with your gstcolorbarsvideosrc description
 */
/*
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    gstcolorbarsvideosrc,
    "gst_color_bars_video_src",
    gstcolorbarsvideosrc_init,
    PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
*/

void _GstColorBarsVideoSrc::set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    log("GstColorBarsVideoSrc::_set_property");
    GstColorBarsVideoSrc *filter = GST_GSTCOLORBARSVIDEOSRC(object);

    switch (prop_id)
    {
    case PROP_SILENT:
        filter->silent = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

void _GstColorBarsVideoSrc::get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    log("GstColorBarsVideoSrc::_get_property");
    GstColorBarsVideoSrc *filter = GST_GSTCOLORBARSVIDEOSRC(object);

    switch (prop_id)
    {
    case PROP_SILENT:
        g_value_set_boolean(value, filter->silent);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

void _GstColorBarsVideoSrc::finalize(GObject *object)
{
    log("GstColorBarsVideoSrc::_finalize");
    GstColorBarsVideoSrc *filter = GST_GSTCOLORBARSVIDEOSRC(object);
    delete filter->color_bars;
    filter->color_bars = nullptr;
    G_OBJECT_CLASS(parent_class)->finalize(object);
}

void gst_color_bars_video_src_register()
{
    // GST_TYPE_GSTCOLORBARSVIDEOSRC;
    log("GstColorBarsVideoSrc::_register");

    gboolean res = gst_element_register(NULL, "gst_color_bars_video_src", GST_RANK_NONE, GST_TYPE_GSTCOLORBARSVIDEOSRC);
}

/* initialize the gstcolorbarsvideosrc's class */
static void
gst_color_bars_video_src_class_init(GstColorBarsVideoSrcClass *klass)
{
    log("GstColorBarsVideoSrc::class_init");

    G_TYPE_CHECK_CLASS_CAST(klass, GST_TYPE_GSTCOLORBARSVIDEOSRC, GstColorBarsVideoSrc);

    auto gobject_class = G_OBJECT_CLASS(klass);
    auto gstelement_class = GST_ELEMENT_CLASS(klass);
    auto gstpushsrc_class = GST_PUSH_SRC_CLASS(klass);
    auto basesrc_class = GST_BASE_SRC_CLASS(klass);

    gobject_class->set_property = GST_DEBUG_FUNCPTR(_GstColorBarsVideoSrc::set_property);
    gobject_class->get_property = GST_DEBUG_FUNCPTR(_GstColorBarsVideoSrc::get_property);
    gobject_class->finalize = GST_DEBUG_FUNCPTR(_GstColorBarsVideoSrc::finalize);
    gstpushsrc_class->create = GST_DEBUG_FUNCPTR(_GstColorBarsVideoSrc::create);
    basesrc_class->start = GST_DEBUG_FUNCPTR(_GstColorBarsVideoSrc::_start);
    basesrc_class->set_caps = GST_DEBUG_FUNCPTR(_GstColorBarsVideoSrc::setcaps);

    g_object_class_install_property(gobject_class, PROP_SILENT,
                                    g_param_spec_boolean("silent", "Silent", "Produce verbose output ?",
                                                         FALSE, G_PARAM_READWRITE));

    gst_element_class_set_details_simple(gstelement_class,
                                         "GstColorBarsVideoSrc",
                                         "source",
                                         "In memory Video Buffer Test Source", "Roman Ukhov ukhov.roman@gmail.com>>");

    gst_element_class_add_static_pad_template(gstelement_class, &src_factory);
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void gst_color_bars_video_src_init(GstColorBarsVideoSrc *source)
{
    log("GstColorBarsVideoSrc::_init");

    // source->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
    // GST_PAD_SET_PROXY_CAPS (source->srcpad);
    // gst_element_add_pad (GST_ELEMENT (source), source->srcpad);

    source->silent = FALSE;

    source->color_bars = gst::make_GstColorBars();

    auto element = (GstElement *)source;

    // auto clock = gst_element_get_clock(element);
    // auto clickId = gst_clock_new_periodic_id(clock, 0, 500 * 1000000);

    /* Configure basesrc to operate in push mode */
    gst_base_src_set_format(GST_BASE_SRC(source), GST_FORMAT_TIME);
    gst_base_src_set_live(GST_BASE_SRC(source), FALSE);
    gst_base_src_set_async(GST_BASE_SRC(source), FALSE);

    GstState state;
    GstState pending;

    auto retSCh =
        gst_element_get_state(&source->element.parent.element,
                              &state,
                              &pending,
                              GST_CLOCK_TIME_NONE);
}

gboolean _GstColorBarsVideoSrc::_start(GstBaseSrc *object)
{
    log("GstColorBarsVideoSrc::_start");

    GstColorBarsVideoSrc *self = GST_GSTCOLORBARSVIDEOSRC(object);

    return TRUE; // Return TRUE on success, FALSE on failure
}

GstFlowReturn _GstColorBarsVideoSrc::create(GstPushSrc *src, GstBuffer **buf)
{
    log("UNIMPLEMENTED: GstColorBarsVideoSrc::create");

    auto self = GST_GSTCOLORBARSVIDEOSRC(src);

    // end of stream
    if (0)
    {
        return GST_FLOW_EOS;
    }

    gsize y_size = self->_video_info.width * self->_video_info.height;
    gsize u_size = y_size / 4;
    gsize v_size = y_size / 4;
    gsize frame_size = y_size + u_size + v_size;

    /*
    if (g_strcmp0(self->format_string, "RGB") == 0) {
      bpp = 3;
    } else if (g_strcmp0(self->format_string, "YUY2") == 0) {
      bpp = 2;
    } else {
      GST_ERROR_OBJECT(self, "Unsupported format: %s", self->format_string);
      return GST_FLOW_ERROR;
    }
      */

    /*
    if (self->buffer_size - self->current_offset < frame_size) {
      GST_WARNING_OBJECT(self, "Not enough data in buffer for a full frame");
      return GST_FLOW_EOS; // Or handle partial frame differently
    }
      */

    GstBuffer *buffer = gst_buffer_new_allocate(nullptr, frame_size, nullptr);
    GstMapInfo map;

    /*
    if (gst_buffer_map(buffer, GST_MAP_WRITE, &map)) {
      std::memcpy(map.data, self->video_buffer.data() + self->current_offset, frame_size);
      gst_buffer_unmap(buffer, &map);
    } else {
      gst_buffer_unref(buffer);
      GST_ERROR_OBJECT(self, "Failed to map buffer for writing");
      return GST_FLOW_ERROR;
    }

    GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(self->frames_sent, GST_SECOND, 30); // Assuming 30 FPS
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(1, GST_SECOND, 30);
    GST_BUFFER_OFFSET(buffer) = GST_BUFFER_PTS(buffer);
    GST_BUFFER_OFFSET_END(buffer) = GST_BUFFER_PTS(buffer) + GST_BUFFER_DURATION(buffer);

    self->current_offset += frame_size;
    self->frames_sent++;
    *out_buf = buffer;

    */

    if (gst_buffer_map(buffer, &map, GST_MAP_WRITE))
    {

        auto dst = map.data;

        for (int i = 0; i < frame_size; ++i)
        {
            dst[i] = 0xaa;
        }

        gst_buffer_unmap(buffer, &map);
    }
    else
    {
        gst_buffer_unref(buffer);
        GST_ERROR_OBJECT(self, "Failed to map buffer for writing");
        log("Failed to map buffer for writing");
        return GST_FLOW_ERROR;
    }

    GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(self->_frames_sent, GST_SECOND * self->_video_info.fps_d, self->_video_info.fps_n); // Assuming 30 FPS
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(1, GST_SECOND * self->_video_info.fps_d, self->_video_info.fps_n);
    GST_BUFFER_OFFSET(buffer) = GST_BUFFER_PTS(buffer);
    GST_BUFFER_OFFSET_END(buffer) = GST_BUFFER_PTS(buffer) + GST_BUFFER_DURATION(buffer);

    *buf = buffer;

    return GST_FLOW_OK;
}

std::string Caps2String(GstCaps &caps)
{
    auto pchar = gst_caps_serialize(&caps, GstSerializeFlags::GST_SERIALIZE_FLAG_NONE);
    std::string str(pchar);
    g_free(pchar);
    return str;
}

gboolean _GstColorBarsVideoSrc::setcaps(GstBaseSrc *bsrc, GstCaps *caps)
{
    auto caps_str = Caps2String(*caps);
    log("GstColorBarsVideoSrc::setcaps");
    log("Proposed caps: {}\n", caps_str);

    gboolean ret = TRUE;
    GstVideoInfo info;
    auto self = GST_GSTCOLORBARSVIDEOSRC(bsrc);

    for (guint i = 0; i < gst_caps_get_size(caps); ++i)
    {
        auto structure = gst_caps_get_structure(caps, i);
        const gchar *name = gst_structure_get_name(structure);

        log("Caps structure {}: {}", i, gst_structure_to_string(structure));

        if (gst_structure_has_name(structure, "video/x-raw"))
        {
            /* we can use the parsing code */
            if (!gst_video_info_from_caps(&info, caps))
            {
                log("Failed to parse video info from caps");
                ret = FALSE;
                continue;
            }

            self->_video_info = info;
        }
        else if (gst_structure_has_name(structure, "video/x-bayer"))
        {
            log("Unsupported Bayer format");
            ret = FALSE;
            continue;
        }
        else
        {
            ret = FALSE;
            continue;
        }
    }

    // If you found a compatible format, store the negotiated caps
    if (ret)
    {
        self->_negotiated_caps = caps_str;
    }

    return ret;
}