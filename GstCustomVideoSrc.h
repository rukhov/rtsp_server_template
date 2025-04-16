#ifndef __GST_GSTCUSTOMVIDEOSRC_H__
#define __GST_GSTCUSTOMVIDEOSRC_H__

#include <gst/gst.h>
#include <gst/video/gstvideometa.h>

namespace gst {
class PictureGen;
}


#define GST_TYPE_GSTCUSTOMVIDEOSRC (gst_custom_video_src_get_type())

G_DECLARE_FINAL_TYPE(
    GstCustomVideoSrc, gst_custom_video_src, GST, CUSTOMVIDEOSRC, GstPushSrc)

struct _GstCustomVideoSrc {
    GstPushSrc element;

    GstPad *sinkpad, *srcpad;

    std::string _negotiated_caps;
    GstVideoInfo _video_info;
    gst::PictureGen* _picture_gen;
    size_t _frame_counter = 0;
    std::chrono::high_resolution_clock::time_point _next_frame;

    static void
    set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec);
    static void
    get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec);
    static void finalize(GObject* object);
    static gboolean _start(GstBaseSrc* object);
    static GstFlowReturn create(GstPushSrc* src, GstBuffer** buf);
    static gboolean setcaps(GstBaseSrc* bsrc, GstCaps* caps);
};

void gst_custom_video_src_register();


#endif /* __GST_GSTCUSTOMVIDEOSRC_H__ */
