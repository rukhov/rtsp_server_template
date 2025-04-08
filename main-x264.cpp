#include <iostream>
#include <cstring>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>

extern "C" {
#include <x264.h>
}

#include <BasicUsageEnvironment.hh>
#include <GroupsockHelper.hh>
#include <liveMedia.hh>
#include <H264VideoStreamDiscreteFramer.hh>
#include <H264VideoRTPSink.hh>

#define WIDTH 640
#define HEIGHT 480
#define FPS 30
#define BITRATE 500000

UsageEnvironment* env;
char const* streamName = "test";
portNumBits rtspServerPortNum = 8554;

class MemoryBufferSource : public FramedSource {
public:
    static MemoryBufferSource* createNew(UsageEnvironment& env) {
        return new MemoryBufferSource(env);
    }

protected:
    MemoryBufferSource(UsageEnvironment& env)
        : FramedSource(env), encoder(nullptr), isFirstFrame(true), timestamp(0) {
        x264_param_default_preset(&param, "veryfast", "zerolatency");
        param.i_width = WIDTH;
        param.i_height = HEIGHT;
        param.i_fps_num = FPS;
        param.i_fps_den = 1;
        param.i_csp = X264_CSP_I420;
        param.b_vfr_input = 0;
        param.b_repeat_headers = 1;
        param.b_annexb = 1;
        
        x264_param_apply_profile(&param, "baseline");
        param.rc.i_bitrate = BITRATE / 1000;
        
        encoder = x264_encoder_open(&param);
        if (!encoder) {
            env << "Failed to open x264 encoder\n";
            exit(1);
        }
        
        x264_picture_alloc(&pic_in, X264_CSP_I420, WIDTH, HEIGHT);
        x264_picture_init(&pic_out);
    }

    ~MemoryBufferSource() override {
        x264_picture_clean(&pic_in);
        if (encoder) {
            x264_encoder_close(encoder);
        }
    }

private:
    void doGetNextFrame() override {
        generateTestFrame();
        
        x264_nal_t* nals;
        int i_nals;
        int frame_size = x264_encoder_encode(encoder, &nals, &i_nals, &pic_in, &pic_out);
        
        if (frame_size > 0) {
            // Calculate total frame size including start codes
            unsigned total_size = 0;
            for (int i = 0; i < i_nals; ++i) {
                total_size += nals[i].i_payload;
                // Add 4 bytes for start code if not already present
                if (nals[i].i_payload < 4 || 
                    nals[i].p_payload[0] != 0 || 
                    nals[i].p_payload[1] != 0 || 
                    nals[i].p_payload[2] != 0 || 
                    nals[i].p_payload[3] != 1) {
                    total_size += 4;
                }
            }
            
            // Ensure our buffer is large enough
            if (fMaxSize < total_size) {
                fFrameSize = 0;
                fNumTruncatedBytes = 0;
                handleClosure();
                return;
            }
            
            // Copy NAL units into the buffer with proper start codes
            unsigned offset = 0;
            for (int i = 0; i < i_nals; ++i) {
                // Check if start code is already present
                bool has_start_code = (nals[i].i_payload >= 4 && 
                                      nals[i].p_payload[0] == 0 && 
                                      nals[i].p_payload[1] == 0 && 
                                      nals[i].p_payload[2] == 0 && 
                                      nals[i].p_payload[3] == 1);
                
                if (!has_start_code) {
                    // Add start code
                    static const uint8_t start_code[4] = {0, 0, 0, 1};
                    memcpy(fTo + offset, start_code, 4);
                    offset += 4;
                }
                
                // Copy NAL unit data
                memcpy(fTo + offset, nals[i].p_payload, nals[i].i_payload);
                offset += nals[i].i_payload;
            }
            
            fFrameSize = offset;
            fNumTruncatedBytes = 0;
            
            // Set presentation time
            if (isFirstFrame) {
                gettimeofday(&fPresentationTime, nullptr);
                isFirstFrame = false;
            } else {
                fPresentationTime.tv_usec += 1000000 / FPS;
                if (fPresentationTime.tv_usec >= 1000000) {
                    fPresentationTime.tv_sec += fPresentationTime.tv_usec / 1000000;
                    fPresentationTime.tv_usec %= 1000000;
                }
            }
            
            afterGetting(this);
        } else {
            nextTask() = env->taskScheduler().scheduleDelayedTask(
                1000 / FPS, (TaskFunc*)FramedSource::afterGetting, this);
        }
    }
    
    void generateTestFrame() {
        static int counter = 0;
        counter++;
        
        uint8_t* y = pic_in.img.plane[0];
        uint8_t* u = pic_in.img.plane[1];
        uint8_t* v = pic_in.img.plane[2];
        
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                y[i * WIDTH + j] = (i + counter) % 256;
            }
        }
        
        for (int i = 0; i < HEIGHT/2; i++) {
            for (int j = 0; j < WIDTH/2; j++) {
                u[i * WIDTH/2 + j] = 128 + (j + counter) % 128;
                v[i * WIDTH/2 + j] = 128 + (i + counter*2) % 128;
            }
        }
        
        pic_in.i_pts = timestamp++;
    }

    x264_param_t param;
    x264_t* encoder;
    x264_picture_t pic_in;
    x264_picture_t pic_out;
    bool isFirstFrame;
    int64_t timestamp;
};

int main(int argc, char** argv) {
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);
    
    RTSPServer* rtspServer = RTSPServer::createNew(*env, rtspServerPortNum, NULL);
    if (rtspServer == nullptr) {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }
    
    // Create proper address structure
    struct sockaddr_storage destinationAddress;
    memset(&destinationAddress, 0, sizeof(destinationAddress));
    struct sockaddr_in* destAddr = (struct sockaddr_in*)&destinationAddress;
    destAddr->sin_family = AF_INET;
    destAddr->sin_addr.s_addr = INADDR_ANY;
    destAddr->sin_port = htons(18888);
    
    // Create RTP groupsock
    const unsigned char ttl = 255;
    Groupsock rtpGroupsock(*env, destinationAddress, Port(destAddr->sin_port), ttl);
    
    // Create in-memory source
    FramedSource* videoSource = MemoryBufferSource::createNew(*env);
    
    // Create framer
    H264VideoStreamDiscreteFramer* videoFramer = H264VideoStreamDiscreteFramer::createNew(*env, videoSource);
    
    // Create RTP sink
    RTPSink* videoSink = H264VideoRTPSink::createNew(*env, &rtpGroupsock, 96);
    
    // Create media session
    ServerMediaSession* sms = ServerMediaSession::createNew(*env, streamName);
    sms->addSubsession(PassiveServerMediaSubsession::createNew(*videoSink, NULL));
    rtspServer->addServerMediaSession(sms);
    
    char* url = rtspServer->rtspURL(sms);
    *env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;
    
    // Start playing
    videoSink->startPlaying(*videoFramer, NULL, NULL);
    
    env->taskScheduler().doEventLoop();
    return 0;
}