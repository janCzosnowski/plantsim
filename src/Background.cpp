#ifdef __linux__ 
#include <string>
#include <raylib.h>
#include "Globals.h"

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/avutil.h>
}

// FFmpeg Contexts
static AVFormatContext* fmtCtx = nullptr;
static AVCodecContext* codecCtx = nullptr;
static AVStream* videoStream = nullptr;
static int videoStreamIndex = -1;
static AVFrame* frame = nullptr;
static AVFrame* frameRGBA = nullptr;
static uint8_t* buffer = nullptr;
static struct SwsContext* swsCtx = nullptr;
static Texture2D videoTexture;
static bool videoLoaded = false;
#define VIDEO_FPS 10.0f


bool InitVideoBackground(const std::string& path) {
    // 1. Open Input
    if (avformat_open_input(&fmtCtx, path.c_str(), nullptr, nullptr) != 0) {
        TraceLog(LOG_WARNING, "FFMPEG: Failed to open %s", path.c_str());
        return false;
    }

    // 2. Retrieve stream information
    if (avformat_find_stream_info(fmtCtx, nullptr) < 0) {
        avformat_close_input(&fmtCtx);
        return false;
    }

    // 3. Find video stream and decoder
    const AVCodec* codec = nullptr;
    videoStreamIndex = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);

    if (videoStreamIndex < 0 || !codec) {
        avformat_close_input(&fmtCtx);
        return false;
    }

    videoStream = fmtCtx->streams[videoStreamIndex];

    // 4. Setup Codec Context
    codecCtx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecCtx, videoStream->codecpar);
    
    if (avcodec_open2(codecCtx, codec, nullptr) < 0) {
        avcodec_free_context(&codecCtx);
        avformat_close_input(&fmtCtx);
        return false;
    }

    // 5. Allocate Frames & Buffer
    frame = av_frame_alloc();
    frameRGBA = av_frame_alloc();
    
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, codecCtx->width, codecCtx->height, 1);
    buffer = (uint8_t*)av_malloc(numBytes);
    av_image_fill_arrays(frameRGBA->data, frameRGBA->linesize, buffer, AV_PIX_FMT_RGBA, codecCtx->width, codecCtx->height, 1);

    // 6. Initialize SwsContext
    swsCtx = sws_getContext(codecCtx->width, codecCtx->height, codecCtx->pix_fmt,
                            codecCtx->width, codecCtx->height, AV_PIX_FMT_RGBA,
                            SWS_BILINEAR, nullptr, nullptr, nullptr);

    // 7. Create Raylib Texture (GPU side)
    // Initialize with a blank texture of correct size
    Image img = GenImageColor(codecCtx->width, codecCtx->height, BLANK);
    videoTexture = LoadTextureFromImage(img);
    UnloadImage(img); 

    videoLoaded = true;
    return true;
} 

void UpdateVideoBackground() {
    if (!videoLoaded) return;

    // 1. Timing Logic
    static float timeAccumulator = 0.0f;
    float frameDuration = 1.0f / VIDEO_FPS;
    timeAccumulator += GetFrameTime();

    // Only proceed if enough time has passed for a new video frame
    if (timeAccumulator < frameDuration) return;
    timeAccumulator -= frameDuration;

    // 2. Processing Logic
    AVPacket packet;
    // We use a loop here because av_read_frame might return audio or metadata 
    // packets even if we only asked for video; we need to skip those.
    while (av_read_frame(fmtCtx, &packet) >= 0) {
        if (packet.stream_index == videoStreamIndex) {
            if (avcodec_send_packet(codecCtx, &packet) == 0) {
                if (avcodec_receive_frame(codecCtx, frame) == 0) {
                    // Convert YUV to RGBA
                    sws_scale(swsCtx, frame->data, frame->linesize, 0, codecCtx->height,
                              frameRGBA->data, frameRGBA->linesize);

                    // Upload to GPU
                    UpdateTexture(videoTexture, frameRGBA->data[0]);
                    
                    av_packet_unref(&packet);
                    return; // Successfully displayed a frame
                }
            }
        }
        av_packet_unref(&packet);
    }

    // 3. Looping Logic (End of file reached)
    // Seek back to the beginning
    av_seek_frame(fmtCtx, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
    
    // IMPORTANT: Clear the decoder's memory so the next frame isn't corrupted
    avcodec_flush_buffers(codecCtx);
}

void DrawVideoBackground() {
    if (!videoLoaded) return;
    
    DrawTexturePro(
        videoTexture,
        { 0, 0, (float)videoTexture.width, (float)videoTexture.height },
        { 0, 0, (float)screenWidth, (float)screenHeight },
        { 0, 0 }, 0.0f, WHITE
    );
}

void ShutdownVideoBackground() {
    if (!videoLoaded) return;

    UnloadTexture(videoTexture);
    av_free(buffer);
    av_frame_free(&frame);
    av_frame_free(&frameRGBA);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&fmtCtx);
    sws_freeContext(swsCtx);

    videoLoaded = false;
}

#endif