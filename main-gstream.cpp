#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <iostream>
#include <string>

#include "CustomMediaFactory.h"
#include "GstCustomVideoSrc.h"

#define DEFAULT_RTSP_PORT "8554"
#define DEFAULT_MOUNT_POINT "/custom_video_src"

static char* port = (char*)DEFAULT_RTSP_PORT;
static char* mount_point = (char*)DEFAULT_MOUNT_POINT;

GstElement* create_custom_rtsp_pipeline()
{
    // Create the pipeline
    GstElement* pipeline = gst_pipeline_new("rtsp-pipeline");
    if (!pipeline) {
        g_printerr("Failed to create pipeline\n");
        return NULL;
    }

    // Create elements
    GstElement* src = gst_element_factory_make("gst_custom_video_src", "source");
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
        return NULL;
    }

    // Set properties
    // First capsfilter: video/x-raw,width=640,height=480,framerate=25/1,format=RGB
    GstCaps* caps1 = gst_caps_new_simple("video/x-raw",
                                         "width",
                                         G_TYPE_INT,
                                         640,
                                         "height",
                                         G_TYPE_INT,
                                         480,
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
        return NULL;
    }

    return pipeline;
}

int main(int argc, char* argv[])
{
    GMainLoop* loop;
    GstRTSPServer* server;
    GstRTSPMountPoints* mounts;
    GstRTSPMediaFactory* factory;

    gst_init(&argc, &argv);

    gst_custom_video_src_register();

    // Create the pipeline
    auto pipeline = gst_pipeline_new("my-pipeline");

    loop = g_main_loop_new(NULL, FALSE);

    /* create a server instance */
    server = gst_rtsp_server_new();

    g_object_set(server, "service", port, NULL);

    /* get the mount points for this server, every server has a default object
     * that be used to map uri mount points to media factories */
    mounts = gst_rtsp_server_get_mount_points(server);

    /* make a media factory for a test stream. The default media factory can use
     * gst-launch syntax to create pipelines.
     * any launch line works as long as it contains elements named pay%d. Each
     * element with pay%d names will be a stream */

    if constexpr (0) {
        // For testing purposes
        factory = gst_rtsp_media_factory_new();
        gst_rtsp_media_factory_set_launch(
            factory,
            "( videotestsrc is-live=1 pattern=smpte ! "
            "video/x-raw,width=640,height=480,framerate=30/1 "
            "! x264enc key-int-max=30 tune=zerolatency ! h264parse "
            "! rtph264pay config-interval=-1 pt=96 name=pay0 )");
    } else {
        factory = my_media_factory_new(create_custom_rtsp_pipeline());
    }

    gst_rtsp_media_factory_set_shared(factory, TRUE);

    /* attach the test factory to the /test url */
    gst_rtsp_mount_points_add_factory(mounts, mount_point, factory);

    /* don't need the ref to the mapper anymore */
    g_object_unref(mounts);

    /* attach the server to the default maincontext */
    gst_rtsp_server_attach(server, NULL);

    /* start serving */
    g_print("stream ready at rtsp://127.0.0.1:%s%s\n", port, mount_point);
    g_main_loop_run(loop);

    return 0;
}
