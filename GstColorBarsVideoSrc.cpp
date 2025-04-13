
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>
#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>

#include "GstColorBarsVideoSrc.h"
#include "gst-color-bars.h"

GST_DEBUG_CATEGORY_STATIC(gst_gst_color_bars_video_src_debug);
#define GST_CAT_DEFAULT gst_gst_color_bars_video_src_debug

template <typename... _Args>
inline void log(std::format_string<_Args...> __fmt, _Args &&...__args)
{
    std::cout << ">>>>>>>> " << std::vformat(__fmt.get(), std::make_format_args(__args...));
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

#define BAYER_CAPS_GEN(mask, bits, endian) \
    " " #mask #bits #endian

#define BAYER_CAPS_ORD(bits, endian)   \
    BAYER_CAPS_GEN(bggr, bits, endian) \
    "," BAYER_CAPS_GEN(rggb, bits, endian) "," BAYER_CAPS_GEN(grbg, bits, endian) "," BAYER_CAPS_GEN(gbrg, bits, endian)

#define BAYER_CAPS_BITS(bits) \
    BAYER_CAPS_ORD(bits, le)  \
    "," BAYER_CAPS_ORD(bits, be)

#define BAYER_CAPS_ALL \
    BAYER_CAPS_ORD(, ) \
    "," BAYER_CAPS_BITS(10) "," BAYER_CAPS_BITS(12) "," BAYER_CAPS_BITS(14) "," BAYER_CAPS_BITS(16)

#define VTS_VIDEO_CAPS GST_VIDEO_CAPS_MAKE(GST_VIDEO_FORMATS_ALL) ","                                                     \
                                                                  "multiview-mode = { mono, left, right }"                \
                                                                  ";"                                                     \
                                                                  "video/x-bayer, format=(string) {" BAYER_CAPS_ALL " }," \
                                                                  "width = " GST_VIDEO_SIZE_RANGE ", "                    \
                                                                  "height = " GST_VIDEO_SIZE_RANGE ", "                   \
                                                                  "framerate = " GST_VIDEO_FPS_RANGE ", "                 \
                                                                  "multiview-mode = { mono, left, right }"

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
    log("GstColorBarsVideoSrc::_sink_event\n");
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
    log("GstColorBarsVideoSrc::_chain\n");
    GstColorBarsVideoSrc *filter;

    filter = GST_GSTCOLORBARSVIDEOSRC(parent);

    if (filter->silent == FALSE)
        g_print("I'm plugged, therefore I'm in.\n");

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
    log("GstColorBarsVideoSrc::_set_property\n");
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
    log("GstColorBarsVideoSrc::_get_property\n");
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
    log("GstColorBarsVideoSrc::_finalize\n");
    GstColorBarsVideoSrc *filter = GST_GSTCOLORBARSVIDEOSRC(object);
    delete filter->color_bars;
    filter->color_bars = nullptr;
    G_OBJECT_CLASS(parent_class)->finalize(object);
}

void gst_color_bars_video_src_register()
{
    // GST_TYPE_GSTCOLORBARSVIDEOSRC;
    log("GstColorBarsVideoSrc::_register\n");

    gboolean res = gst_element_register(NULL, "gst_color_bars_video_src", GST_RANK_NONE, GST_TYPE_GSTCOLORBARSVIDEOSRC);
}

/* initialize the gstcolorbarsvideosrc's class */
static void
gst_color_bars_video_src_class_init(GstColorBarsVideoSrcClass *klass)
{
    log("GstColorBarsVideoSrc::class_init\n");

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
    log("GstColorBarsVideoSrc::_init\n");

    // source->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
    // GST_PAD_SET_PROXY_CAPS (source->srcpad);
    // gst_element_add_pad (GST_ELEMENT (source), source->srcpad);

    source->silent = FALSE;

    source->color_bars = gst::make_GstColorBars();

    auto element = (GstElement *)source;

    // auto clock = gst_element_get_clock(element);
    // auto clickId = gst_clock_new_periodic_id(clock, 0, 500 * 1000000);
}

gboolean _GstColorBarsVideoSrc::_start(GstBaseSrc *object)
{
    log("GstColorBarsVideoSrc::_start\n");

    GstColorBarsVideoSrc *self = GST_GSTCOLORBARSVIDEOSRC(object);
    // ... your implementation ...
    return TRUE; // Return TRUE on success, FALSE on failure
}

GstFlowReturn _GstColorBarsVideoSrc::create(GstPushSrc *src, GstBuffer **buf)
{
    log("UNIMPLEMENTED: GstColorBarsVideoSrc::create\n");

    auto self = GST_GSTCOLORBARSVIDEOSRC(src);

    // end of stream
    if (0)
    {
        return GST_FLOW_EOS;
    }

    guint bpp = 0;
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

    gsize frame_size = 640 * 480 * 4; // self->frame_width * self->frame_height * bpp;
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
        return GST_FLOW_ERROR;
    }

    *buf = buffer;
    GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(self->frames_sent, GST_SECOND, 30); // Assuming 30 FPS

    return GST_FLOW_OK;
}

gboolean _GstColorBarsVideoSrc::setcaps(GstBaseSrc *bsrc, GstCaps *caps)
{

    log("GstColorBarsVideoSrc::setcaps\n");

    GstVideoInfo info;
    const GstStructure *structure;
    auto self = GST_GSTCOLORBARSVIDEOSRC(bsrc);

    structure = gst_caps_get_structure(caps, 0);

    if (gst_structure_has_name(structure, "video/x-raw"))
    {
        /* we can use the parsing code */
        if (!gst_video_info_from_caps(&info, caps))
            return FALSE;
    }
    else if (gst_structure_has_name(structure, "video/x-bayer"))
    {
        // if (!gst_video_test_src_parse_caps (caps, &info, videotestsrc)) return FALSE;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}