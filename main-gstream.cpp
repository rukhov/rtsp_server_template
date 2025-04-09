#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <iostream>
#include <string>

#define DEFAULT_RTSP_PORT "8554"

static char *port = (char *)DEFAULT_RTSP_PORT;
static char *file_path = NULL;

// Structure to hold our pipeline information
struct CustomData {
    GstRTSPServer *server;
    GstRTSPMediaFactory *factory;
};

// Callback when media is configured - we can add the file source here
static void media_configured(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data) {
    GstElement *element, *filesrc;
    CustomData *data = (CustomData *)user_data;

    // Get the pipeline from the media
    element = gst_rtsp_media_get_element(media);

    // Create a filesrc element
    filesrc = gst_element_factory_make("filesrc", "file-source");
    g_object_set(filesrc, "location", file_path, NULL);

    // Get the payloader element from the pipeline
    GstElement *payloader = gst_bin_get_by_name(GST_BIN(element), "pay0");

    // Create a queue element
    GstElement *queue = gst_element_factory_make("queue", "queue");

    // Add the filesrc and queue to the pipeline before the payloader
    gst_bin_add_many(GST_BIN(element), filesrc, queue, NULL);

    // Link the filesrc to the queue and then to the payloader
    if (!gst_element_link_many(filesrc, queue, payloader, NULL)) {
        g_printerr("Elements could not be linked.\n");
    }

    gst_object_unref(payloader);
    gst_object_unref(element);
}

int main(int argc, char *argv[]) {
    GMainLoop *loop;
    CustomData data;

    if (argc < 2) {
        g_printerr("Usage: %s <H264 file> [port]\n", argv[0]);
        return -1;
    }

    file_path = argv[1];
    if (argc > 2) {
        port = argv[2];
    }

    // Initialize GStreamer
    gst_init(&argc, &argv);

    // Create main loop
    loop = g_main_loop_new(NULL, FALSE);

    // Create RTSP server
    data.server = gst_rtsp_server_new();
    g_object_set(data.server, "service", port, NULL);

    // Create a media factory for H264 streams
    data.factory = gst_rtsp_media_factory_new();
    
    // Set the launch pipeline - this will be enhanced in the media_configured callback
    gst_rtsp_media_factory_set_launch(data.factory, 
        "( appsrc name=src ! h264parse ! rtph264pay name=pay0 pt=96 )");

    // Set the shared property so multiple clients can connect
    gst_rtsp_media_factory_set_shared(data.factory, TRUE);

    // Connect the media configured signal
    g_signal_connect(data.factory, "media-configured", (GCallback)media_configured, &data);

    // Get the mount points from the server and add our factory to it
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(data.server);
    gst_rtsp_mount_points_add_factory(mounts, "/test", data.factory);
    g_object_unref(mounts);

    // Attach the server to the main context
    gst_rtsp_server_attach(data.server, NULL);

    // Print the stream URL
    g_print("Stream ready at rtsp://127.0.0.1:%s/test\n", port);

    // Start the main loop
    g_main_loop_run(loop);

    // Clean up
    g_main_loop_unref(loop);
    g_object_unref(data.server);

    return 0;
}