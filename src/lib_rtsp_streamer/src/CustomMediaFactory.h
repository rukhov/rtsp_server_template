#include <gst/rtsp-server/rtsp-media-factory.h>

#define GST_CUSTOM_VIDEO_SRC_MEDIA_FACTORY_GET_TYPE \
    (gst_custom_video_src_media_factory_get_type())

G_DECLARE_FINAL_TYPE(GstCustomVideoSrcMediaFactory,
                     gst_custom_video_src_media_factory,
                     GST,
                     CUSTOM_VIDEO_SRC_MEDIA_FACTORY,
                     GstRTSPMediaFactory)

typedef struct _GstCustomVideoSrcMediaFactory {
    GstRTSPMediaFactory parent_instance;
    // Add any custom data you need
    GstElement* _the_piplene;
} GstCustomVideoSrcMediaFactory;

GstRTSPMediaFactory* gst_custom_video_src_media_factory_new(GstElement* pipeline);
