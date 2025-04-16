#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <iostream>
#include <string>

#include "GstColorBarsVideoSrc.h"

#define DEFAULT_RTSP_PORT "8554"
#define DEFAULT_MOUNT_POINT "/test"
#define DEFAULT_H264_FILE "test.h264"

static char* port = (char*)DEFAULT_RTSP_PORT;
static char* mount_point = (char*)DEFAULT_MOUNT_POINT;
static char* file_location = (char*)DEFAULT_H264_FILE;

int main(int argc, char* argv[])
{
    GMainLoop* loop;
    GstRTSPServer* server;
    GstRTSPMountPoints* mounts;
    GstRTSPMediaFactory* factory;

    gst_init(&argc, &argv);

    gst_color_bars_video_src_register();

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
    factory = gst_rtsp_media_factory_new();

    if constexpr (0) {

        gst_rtsp_media_factory_set_launch(
            factory,
            "( videotestsrc is-live=1 pattern=smpte ! "
            "video/x-raw,width=640,height=480,framerate=30/1 "
            "! x264enc key-int-max=30 tune=zerolatency ! h264parse "
            "! rtph264pay config-interval=-1 pt=96 name=pay0 )");
    } else {
        gst_rtsp_media_factory_set_launch(
            factory,
            R"( gst_color_bars_video_src ! video/x-raw,width=640,height=480,framerate=25/1,format=RGB ! 
                    videoconvert ! video/x-raw,format=I420 ! x264enc key-int-max=1 ! 
                    rtph264pay pt=96 name=pay0)");
    }
    //  gst_rtsp_media_factory_set_launch(factory, "( gst_color_bars_video_src !
    //  x264enc ! rtph264pay name=pay0 pt=96 )");

    gst_rtsp_media_factory_set_shared(factory, TRUE);

    /* attach the test factory to the /test url */
    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);

    /* don't need the ref to the mapper anymore */
    g_object_unref(mounts);

    /* attach the server to the default maincontext */
    gst_rtsp_server_attach(server, NULL);

    /* start serving */
    g_print("stream ready at rtsp://127.0.0.1:8554/test\n");
    g_main_loop_run(loop);

    return 0;
}
