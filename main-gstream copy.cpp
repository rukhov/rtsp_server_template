#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <iostream>
#include <cstring>
#include <format>

#define DEFAULT_RTSP_PORT "8554"
#define DEFAULT_MOUNT_POINT "/test"
#define WIDTH 640
#define HEIGHT 480
#define FPS 30

static char *port = (char *)DEFAULT_RTSP_PORT;
static char *mount_point = (char *)DEFAULT_MOUNT_POINT;

typedef struct {
    GstElement *appsrc;
    guint sourceid;
    guint64 num_frames;
    GMainLoop *loop;
} AppData;

static void push_data(AppData *data) {
    GstBuffer *buffer;
    GstFlowReturn ret;
    GstMapInfo map;
    guint size;
    static guint8 *frame_data = NULL;
    static gboolean white = FALSE;

    if (frame_data == NULL) {
        frame_data = new guint8[WIDTH * HEIGHT * 3];
    }

    white = !white;
    if (white) {
        memset(frame_data, 255, WIDTH * HEIGHT * 3);
    } else {
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                int section = x * 6 / WIDTH;
                int offset = (y * WIDTH + x) * 3;
                
                switch (section) {
                    case 0: frame_data[offset] = 255; frame_data[offset+1] = 0; frame_data[offset+2] = 0; break;
                    case 1: frame_data[offset] = 255; frame_data[offset+1] = 255; frame_data[offset+2] = 0; break;
                    case 2: frame_data[offset] = 0; frame_data[offset+1] = 255; frame_data[offset+2] = 0; break;
                    case 3: frame_data[offset] = 0; frame_data[offset+1] = 255; frame_data[offset+2] = 255; break;
                    case 4: frame_data[offset] = 0; frame_data[offset+1] = 0; frame_data[offset+2] = 255; break;
                    case 5: frame_data[offset] = 255; frame_data[offset+1] = 0; frame_data[offset+2] = 255; break;
                }
            }
        }
    }

    size = WIDTH * HEIGHT * 3;
    buffer = gst_buffer_new_and_alloc(size);
    gst_buffer_map(buffer, &map, GST_MAP_WRITE);
    memcpy(map.data, frame_data, size);
    gst_buffer_unmap(buffer, &map);

    GST_BUFFER_PTS(buffer) = data->num_frames * GST_SECOND / FPS;
    GST_BUFFER_DURATION(buffer) = GST_SECOND / FPS;
    data->num_frames++;

    g_signal_emit_by_name(data->appsrc, "push-buffer", buffer, &ret);
    gst_buffer_unref(buffer);

    if (ret != GST_FLOW_OK) {
        std::cerr << "Push buffer error: " << ret << std::endl;
        g_main_loop_quit(data->loop);
    }
}

static void start_feed(GstElement *source, guint size, AppData *data) {
  std::cout << std::format("Start feeding\n");
    if (data->sourceid == 0) {
        data->sourceid = g_timeout_add(1000 / FPS, (GSourceFunc)push_data, data);
    }
}

static void stop_feed(GstElement *source, AppData *data) {
  std::cout << std::format("Stop feeding\n");
    if (data->sourceid != 0) {
        g_source_remove(data->sourceid);
        data->sourceid = 0;
    }
}

// Media constructed callback (must be C-compatible)
static void media_constructed(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data) {
    AppData *data = (AppData *)user_data;
    GstElement *element = gst_rtsp_media_get_element(media);
    data->appsrc = gst_bin_get_by_name(GST_BIN(element), "mysrc");
    
    g_object_set(data->appsrc,
        "stream-type", 0,
        "format", GST_FORMAT_TIME,
        "is-live", TRUE,
        "do-timestamp", TRUE,
        NULL);
    
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
        "format", G_TYPE_STRING, "RGB",
        "width", G_TYPE_INT, WIDTH,
        "height", G_TYPE_INT, HEIGHT,
        "framerate", GST_TYPE_FRACTION, FPS, 1,
        NULL);
    g_object_set(data->appsrc, "caps", caps, NULL);
    gst_caps_unref(caps);
    
    g_signal_connect(data->appsrc, "need-data", G_CALLBACK(start_feed), data);
    g_signal_connect(data->appsrc, "enough-data", G_CALLBACK(stop_feed), data);
    
    gst_object_unref(element);
}

int main(int argc, char *argv[]) {
    GMainLoop *loop;
    GstRTSPServer *server;
    GstRTSPMountPoints *mounts;
    GstRTSPMediaFactory *factory;
    AppData data = {0};

    gst_init(&argc, &argv);
    
    loop = g_main_loop_new(NULL, FALSE);
    data.loop = loop;
    
    server = gst_rtsp_server_new();
    g_object_set(server, "service", port, NULL);
    
    mounts = gst_rtsp_server_get_mount_points(server);
    factory = gst_rtsp_media_factory_new();
    
    std::string launch = "( appsrc name=mysrc ! videoconvert ! video/x-raw,format=I420 ! "
                         "x264enc tune=zerolatency speed-preset=ultrafast ! "
                         "h264parse ! rtph264pay name=pay0 pt=96 )";
    
    gst_rtsp_media_factory_set_launch(factory, launch.c_str());
    gst_rtsp_media_factory_set_shared(factory, TRUE);
    
    // Connect the C-compatible callback function
    g_signal_connect(factory, "media-constructed", G_CALLBACK(media_constructed), &data);
    
    gst_rtsp_mount_points_add_factory(mounts, mount_point, factory);
    g_object_unref(mounts);
    
    gst_rtsp_server_attach(server, NULL);
    
    std::cout << "RTSP server streaming at rtsp://127.0.0.1:" << port << mount_point << std::endl;
    std::cout << "Streaming from memory buffer at " << WIDTH << "x" << HEIGHT << " " << FPS << "fps" << std::endl;
    
    g_main_loop_run(loop);
    
    if (data.sourceid != 0) {
        g_source_remove(data.sourceid);
    }
    g_main_loop_unref(loop);
    g_object_unref(server);
    
    return 0;
}