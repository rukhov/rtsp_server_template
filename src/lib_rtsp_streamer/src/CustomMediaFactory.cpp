#include "CustomMediaFactory.h"

G_DEFINE_TYPE(GstCustomVideoSrcMediaFactory,
              gst_custom_video_src_media_factory,
              GST_TYPE_RTSP_MEDIA_FACTORY);
// G_DEFINE_TYPE(MyMediaFactory, my_media_factory, GST_TYPE_RTSP_MEDIA_FACTORY)

static GstElement*
gst_custom_video_src_media_factory_create_element(GstRTSPMediaFactory* factory,
                                                  const GstRTSPUrl* url)
{
    auto self = (GstCustomVideoSrcMediaFactory*)(factory);
    return self->_the_piplene;
}

static void
gst_custom_video_src_media_factory_class_init(GstCustomVideoSrcMediaFactoryClass* klass)
{
    auto factory_class = (GstRTSPMediaFactoryClass*)(klass);
    auto rtsp_factory_class = GST_RTSP_MEDIA_FACTORY_CLASS(klass);
    rtsp_factory_class->create_element =
        gst_custom_video_src_media_factory_create_element;
}

static void
gst_custom_video_src_media_factory_init(GstCustomVideoSrcMediaFactory* factory)
{
    // Initialize any custom data
}

GstRTSPMediaFactory* gst_custom_video_src_media_factory_new(GstElement* pipeline)
{
    auto factory = (GstCustomVideoSrcMediaFactory*)GST_RTSP_MEDIA_FACTORY(
        g_object_new(gst_custom_video_src_media_factory_get_type(), NULL));
    factory->_the_piplene = pipeline;
    return &factory->parent_instance;
}
