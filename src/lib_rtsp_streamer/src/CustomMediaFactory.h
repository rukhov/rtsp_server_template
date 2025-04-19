#include <gst/rtsp-server/rtsp-media-factory.h>

typedef struct {
    GstRTSPMediaFactory parent;
    // Add any custom data you need
} MyMediaFactory;

typedef struct {
    GstRTSPMediaFactoryClass parent_class;
    GstElement* the_piplene;
} MyMediaFactoryClass;

G_DEFINE_TYPE(MyMediaFactory, my_media_factory, GST_TYPE_RTSP_MEDIA_FACTORY);

static GstElement* my_media_factory_create_element(GstRTSPMediaFactory* factory,
                                                   const GstRTSPUrl* url)
{
    auto self = (MyMediaFactoryClass*)(factory);
    return self->the_piplene;
}

static void my_media_factory_class_init(MyMediaFactoryClass* klass)
{
    GstRTSPMediaFactoryClass* factory_class = GST_RTSP_MEDIA_FACTORY_CLASS(klass);
    factory_class->create_element = my_media_factory_create_element;
}

static void my_media_factory_init(MyMediaFactory* factory)
{
    // Initialize any custom data
}

GstRTSPMediaFactory* my_media_factory_new(GstElement* the_piplene)
{
    auto factory =
        GST_RTSP_MEDIA_FACTORY(g_object_new(my_media_factory_get_type(), NULL));
    auto myFactory = (MyMediaFactoryClass*)factory;
    myFactory->the_piplene = the_piplene;
    return factory;
}
