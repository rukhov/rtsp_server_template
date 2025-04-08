#include <opencv2/opencv.hpp>
#include <liveMedia.hh>
#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>

// OpenCV Encoder Wrapper ======================================================
class OpenCVEncoder {
public:
    OpenCVEncoder(int width, int height, int fps) 
        : width(width), height(height), fps(fps), frameCount(0) {
        
        // Initialize OpenCV VideoWriter with H.264 codec
        encoder.open("appsrc ! videoconvert ! x264enc tune=zerolatency ! h264parse ! video/x-h264,stream-format=byte-stream",
                   cv::CAP_GSTREAMER, 0, fps, cv::Size(width, height), true);
        
        if (!encoder.isOpened()) {
            throw std::runtime_error("Failed to initialize OpenCV encoder");
        }
    }

    bool encodeFrame(const cv::Mat& frame, std::vector<uchar>& encodedData) {
        if (frame.empty()) return false;
        
        encoder.write(frame);
        frameCount++;
        
        // In a real application, you'd get encoded packets from GStreamer pipeline
        // For this example, we'll simulate encoded data
        simulateH264Data(encodedData);
        
        return true;
    }

private:
    void simulateH264Data(std::vector<uchar>& data) {
        // This simulates H.264 NAL units - replace with actual encoder output
        data.clear();
        
        // SPS/PPS (simplified)
        if (frameCount % (fps * 2) == 0) { // Every 2 seconds
            const uchar sps[] = {0x00,0x00,0x00,0x01,0x67,0x42,0x00,0x0a,0xf8,0x41,0xa2};
            const uchar pps[] = {0x00,0x00,0x00,0x01,0x68,0xce,0x38,0x80};
            data.insert(data.end(), sps, sps + sizeof(sps));
            data.insert(data.end(), pps, pps + sizeof(pps));
        }
        
        // Frame data (simulated)
        const uchar header[] = {0x00,0x00,0x00,0x01, (frameCount % (fps * 2) == 0) ? 0x65 : 0x61};
        data.insert(data.end(), header, header + sizeof(header));
        
        // Add some dummy payload
        for (int i = 0; i < 10000; i++) {
            data.push_back((i + frameCount) % 256);
        }
    }

    cv::VideoWriter encoder;
    int width, height, fps;
    int frameCount;
};

// Frame Source ================================================================
class H264OpenCVSource : public FramedSource {
public:
    static H264OpenCVSource* createNew(UsageEnvironment& env, 
                                     OpenCVEncoder* encoder,
                                     int width, int height, int fps) {
        return new H264OpenCVSource(env, encoder, width, height, fps);
    }

protected:
    H264OpenCVSource(UsageEnvironment& env, 
                    OpenCVEncoder* enc,
                    int width, int height, int fps)
        : FramedSource(env), 
          encoder(enc),
          width(width), height(height), fps(fps),
          frameCount(0) {
        // Create test pattern image buffer
        testPattern = cv::Mat::zeros(height, width, CV_8UC3);
    }

private:
    virtual void doGetNextFrame() {
        // Generate test pattern
        generateTestPattern();
        
        // Encode frame
        std::vector<uchar> encodedData;
        if (!encoder->encodeFrame(testPattern, encodedData)) {
            handleClosure();
            return;
        }
        
        // Copy to output buffer
        fFrameSize = std::min((unsigned)encodedData.size(), fMaxSize);
        memcpy(fTo, encodedData.data(), fFrameSize);
        fNumTruncatedBytes = encodedData.size() - fFrameSize;
        
        // Set presentation time
        gettimeofday(&fPresentationTime, NULL);
        fPresentationTime.tv_usec += (1000000 / fps) * frameCount++;
        
        // Inform the reader that the frame is available
        afterGetting(this);
    }

    void generateTestPattern() {
        // Create moving color pattern
        cv::Scalar color(
            128 + 127 * sin(frameCount * 0.1),
            128 + 127 * sin(frameCount * 0.2),
            128 + 127 * sin(frameCount * 0.3)
        );
        testPattern = color;
        
        // Add moving circle
        int radius = height/4;
        int centerX = width/2 + (width/3) * sin(frameCount * 0.05);
        int centerY = height/2 + (height/3) * cos(frameCount * 0.05);
        cv::circle(testPattern, cv::Point(centerX, centerY), radius, 
                 cv::Scalar(255, 255, 255) - color, -1);
    }

    OpenCVEncoder* encoder;
    cv::Mat testPattern;
    int width, height, fps;
    int frameCount;
};

// Server Setup ================================================================
class H264OpenCVMediaSubsession : public OnDemandServerMediaSubsession {
public:
    static H264OpenCVMediaSubsession* createNew(UsageEnvironment& env,
                                              int width, int height, int fps) {
        return new H264OpenCVMediaSubsession(env, width, height, fps);
    }

protected:
    H264OpenCVMediaSubsession(UsageEnvironment& env,
                             int width, int height, int fps)
        : OnDemandServerMediaSubsession(env, true),
          width(width), height(height), fps(fps),
          encoder(nullptr) {
        try {
            encoder = new OpenCVEncoder(width, height, fps);
        } catch (const std::exception& e) {
            env << "Encoder creation failed: " << e.what() << "\n";
        }
    }

    virtual ~H264OpenCVMediaSubsession() {
        delete encoder;
    }

    virtual FramedSource* createNewStreamSource(unsigned /*clientSessionId*/,
                                              unsigned& estBitrate) {
        estBitrate = 5000; // kbps estimate
        return H264OpenCVSource::createNew(envir(), encoder, width, height, fps);
    }

    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                     unsigned char rtpPayloadTypeIfDynamic,
                                     FramedSource* /*inputSource*/) {
        OutPacketBuffer::maxSize = 2000000; // 2MB buffer
        return H264VideoRTPSink::createNew(envir(), rtpGroupsock, 
                                         rtpPayloadTypeIfDynamic);
    }

private:
    int width, height, fps;
    OpenCVEncoder* encoder;
};

// Main Program ================================================================
int main(int argc, char** argv) {
    // Setup Live555 environment
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

    // Create RTSP server
    RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554);
    if (!rtspServer) {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }

    // Create media session (640x480 @ 30fps)
    ServerMediaSession* sms = ServerMediaSession::createNew(*env, "test");
    sms->addSubsession(H264OpenCVMediaSubsession::createNew(*env, 640, 480, 30));
    rtspServer->addServerMediaSession(sms);

    // Announce stream
    char* url = rtspServer->rtspURL(sms);
    *env << "OpenCV H.264 stream is available at: " << url << "\n";
    delete[] url;

    // Start event loop
    env->taskScheduler().doEventLoop();

    return 0;
}
