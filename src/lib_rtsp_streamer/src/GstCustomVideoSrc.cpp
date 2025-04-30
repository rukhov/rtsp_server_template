#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/base/gstpushsrc.h>
#include <gst/gst.h>
#include <gst/gstvalue.h>
#include <gst/video/gstvideopool.h>

#include "GstCustomVideoSrc.h"

GST_DEBUG_CATEGORY_STATIC(gst_custom_video_src_debug);
#define GST_CAT_DEFAULT gst_custom_video_src_debug

template <typename... _Args>
inline void log(std::format_string<_Args...> __fmt, _Args&&... __args)
{
    g_log("CUSTOM_VIDEO_SRC",
          GLogLevelFlags::G_LOG_LEVEL_MESSAGE,
          "%s",
          std::vformat(__fmt.get(), std::make_format_args(__args...)).c_str());
}

#define VIDEO_FORMATS_STR "I420, RGB"

#define VIDEO_FORMATS_ALL "{ " VIDEO_FORMATS_STR " }"

#define VTS_VIDEO_CAPS                                            \
    GST_VIDEO_CAPS_MAKE(VIDEO_FORMATS_ALL)                        \
    ","                                                           \
    "width=" GST_VIDEO_SIZE_RANGE ",height=" GST_VIDEO_SIZE_RANGE \
    ",framerate=" GST_VIDEO_FPS_RANGE ","

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE(
    "src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS(VTS_VIDEO_CAPS));

#define gst_custom_video_src_parent_class parent_class

G_DEFINE_TYPE(GstCustomVideoSrc, gst_custom_video_src, GST_TYPE_PUSH_SRC);

GST_ELEMENT_REGISTER_DEFINE(gst_custom_video_src,
                            "gst_custom_video_src",
                            GST_RANK_NONE,
                            GST_TYPE_GSTCUSTOMVIDEOSRC);

static void gst_custom_video_src_set_property(GObject* object,
                                              guint prop_id,
                                              const GValue* value,
                                              GParamSpec* pspec);
static void gst_custom_video_src_get_property(GObject* object,
                                              guint prop_id,
                                              GValue* value,
                                              GParamSpec* pspec);

static gboolean
gst_custom_video_src_sink_event(GstPad* pad, GstObject* parent, GstEvent* event);
static GstFlowReturn
gst_custom_video_src_chain(GstPad* pad, GstObject* parent, GstBuffer* buf);

/* GObject vmethod implementations */

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_custom_video_src_sink_event(GstPad* pad, GstObject* parent, GstEvent* event)
{
    GstCustomVideoSrc* filter;
    gboolean ret;

    filter = GST_CUSTOMVIDEOSRC(parent);

    GST_LOG_OBJECT(
        filter, "Received %s event: %" GST_PTR_FORMAT, GST_EVENT_TYPE_NAME(event), event);

    switch (GST_EVENT_TYPE(event)) {
    case GST_EVENT_CAPS: {
        GstCaps* caps;

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
gst_custom_video_src_chain(GstPad* pad, GstObject* parent, GstBuffer* buf)
{
    GstCustomVideoSrc* filter;

    filter = GST_CUSTOMVIDEOSRC(parent);

    /* just push out the incoming buffer without touching it */
    return gst_pad_push(filter->srcpad, buf);
}

void _GstCustomVideoSrc::set_property(GObject* object,
                                      guint prop_id,
                                      const GValue* value,
                                      GParamSpec* pspec)
{
    GstCustomVideoSrc* filter = GST_CUSTOMVIDEOSRC(object);

    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

void _GstCustomVideoSrc::get_property(GObject* object,
                                      guint prop_id,
                                      GValue* value,
                                      GParamSpec* pspec)
{
    GstCustomVideoSrc* filter = GST_CUSTOMVIDEOSRC(object);

    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

void _GstCustomVideoSrc::finalize(GObject* object)
{
    GstCustomVideoSrc* filter = GST_CUSTOMVIDEOSRC(object);
    filter->_frame_source = nullptr;
    G_OBJECT_CLASS(parent_class)->finalize(object);
}

void gst_custom_video_src_register()
{
    gboolean res = gst_element_register(
        NULL, "gst_custom_video_src", GST_RANK_NONE, GST_TYPE_GSTCUSTOMVIDEOSRC);
}

/* initialize the gstcolorbarsvideosrc's class */
static void gst_custom_video_src_class_init(GstCustomVideoSrcClass* klass)
{
    G_TYPE_CHECK_CLASS_CAST(klass, GST_TYPE_GSTCUSTOMVIDEOSRC, GstCustomVideoSrc);

    auto gobject_class = G_OBJECT_CLASS(klass);
    auto gstelement_class = GST_ELEMENT_CLASS(klass);
    auto gstpushsrc_class = GST_PUSH_SRC_CLASS(klass);
    auto basesrc_class = GST_BASE_SRC_CLASS(klass);

    gobject_class->set_property = GST_DEBUG_FUNCPTR(_GstCustomVideoSrc::set_property);
    gobject_class->get_property = GST_DEBUG_FUNCPTR(_GstCustomVideoSrc::get_property);
    gobject_class->finalize = GST_DEBUG_FUNCPTR(_GstCustomVideoSrc::finalize);
    gstpushsrc_class->create = GST_DEBUG_FUNCPTR(_GstCustomVideoSrc::create);
    basesrc_class->start = GST_DEBUG_FUNCPTR(_GstCustomVideoSrc::_start);
    basesrc_class->set_caps = GST_DEBUG_FUNCPTR(_GstCustomVideoSrc::setcaps);

    gst_element_class_set_details_simple(gstelement_class,
                                         "GstCustomVideoSrc",
                                         "source",
                                         "In memory Video Buffer Test Source",
                                         "Roman Ukhov ukhov.roman@gmail.com>>");

    gst_element_class_add_static_pad_template(gstelement_class, &src_factory);
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void gst_custom_video_src_init(GstCustomVideoSrc* source)
{
    auto element = (GstElement*)source;

    /* Configure basesrc to operate in push mode */
    gst_base_src_set_format(GST_BASE_SRC(source), GST_FORMAT_TIME);
    gst_base_src_set_live(GST_BASE_SRC(source), FALSE);
    gst_base_src_set_async(GST_BASE_SRC(source), FALSE);

    GstState state;
    GstState pending;

    auto retSCh = gst_element_get_state(
        &source->element.parent.element, &state, &pending, GST_CLOCK_TIME_NONE);
}

gboolean _GstCustomVideoSrc::_start(GstBaseSrc* object)
{
    GstCustomVideoSrc* self = GST_CUSTOMVIDEOSRC(object);

    self->_next_frame = std::chrono::high_resolution_clock::now();

    return TRUE;
}

GstFlowReturn _GstCustomVideoSrc::create(GstPushSrc* src, GstBuffer** buf)
{
    auto self = GST_CUSTOMVIDEOSRC(src);

    std::this_thread::sleep_until(self->_next_frame);

    if (self->_eos.test() || self->_frame_source->is_eof()) {
        return GST_FLOW_EOS;
    }

    auto [frame_width, frame_height] = self->_frame_source->get_frame_size();

    GstBuffer* buffer = gst_buffer_new_allocate(
        nullptr, self->_frame_source->stride() * frame_height, nullptr);
    GstMapInfo map;

    if (gst_buffer_map(buffer, &map, GST_MAP_WRITE)) {

        auto src = self->_frame_source->get_next_frame();
        auto dst = map.data;

        std::copy(src.begin(), src.end(), dst);

        gst_buffer_unmap(buffer, &map);
    } else {
        gst_buffer_unref(buffer);
        GST_ERROR_OBJECT(self, "Failed to map buffer for writing");
        log("Failed to map buffer for writing");
        return GST_FLOW_ERROR;
    }

    GST_BUFFER_PTS(buffer) = gst_util_uint64_scale(self->_frame_counter,
                                                   GST_SECOND * self->_video_info.fps_d,
                                                   self->_video_info.fps_n);
    GST_BUFFER_DTS(buffer) = GST_CLOCK_TIME_NONE;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale(
        1, GST_SECOND * self->_video_info.fps_d, self->_video_info.fps_n);
    GST_BUFFER_OFFSET(buffer) = GST_BUFFER_PTS(buffer);
    GST_BUFFER_OFFSET_END(buffer) = GST_BUFFER_PTS(buffer) + GST_BUFFER_DURATION(buffer);

    *buf = buffer;

    self->_next_frame += std::chrono::microseconds(1000000 * self->_video_info.fps_d /
                                                   self->_video_info.fps_n);

    ++self->_frame_counter;
    return GST_FLOW_OK;
}

std::string Caps2String(GstCaps& caps)
{
    auto pchar = gst_caps_serialize(&caps, GstSerializeFlags::GST_SERIALIZE_FLAG_NONE);
    std::string str(pchar);
    g_free(pchar);
    return str;
}

gboolean _GstCustomVideoSrc::setcaps(GstBaseSrc* bsrc, GstCaps* caps)
{
    auto caps_str = Caps2String(*caps);
    gboolean ret = TRUE;
    GstVideoInfo info;
    auto self = GST_CUSTOMVIDEOSRC(bsrc);

    for (guint i = 0; i < gst_caps_get_size(caps); ++i) {
        auto structure = gst_caps_get_structure(caps, i);
        const gchar* name = gst_structure_get_name(structure);

        // log("Caps structure {}: {}", i, gst_structure_to_string(structure));

        if (gst_structure_has_name(structure, "video/x-raw")) {
            /* we can use the parsing code */
            if (!gst_video_info_from_caps(&info, caps)) {
                log("Failed to parse video info from caps");
                ret = FALSE;
                continue;
            }

            self->_video_info = info;
        } else if (gst_structure_has_name(structure, "video/x-bayer")) {
            log("Unsupported Bayer format");
            ret = FALSE;
            continue;
        } else {
            ret = FALSE;
            continue;
        }
    }

    // If I found a compatible format, store the negotiated caps
    if (ret) {

        self->_negotiated_caps = caps_str;
        assert(self->_frame_source);
        self->_eos.clear();
        self->_frame_source->set_format(self->_video_info);
    }

    return ret;
}