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

    // Build your pipeline programmatically
    GstElement* pipeline = gst_pipeline_new("pipeline");
    GstElement* src = gst_element_factory_make("videotestsrc", "src");
    GstElement* enc = gst_element_factory_make("x264enc", "enc");
    GstElement* pay = gst_element_factory_make("rtph264pay", "pay");

    // Configure elements
    g_object_set(pay, "pt", 96, "name", "pay0", NULL);

    // Add and link elements
    gst_bin_add_many(GST_BIN(pipeline), src, enc, pay, NULL);
    gst_element_link_many(src, enc, pay, NULL);

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
