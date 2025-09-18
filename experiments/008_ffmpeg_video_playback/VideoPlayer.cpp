#include <video/VideoDecoder.h>
#include <video/VideoPlayer.h>

#if NV_USE_FFMPEG
#include <ffmpeg/FFMPEGVideoDecoder.h>
#endif

namespace nv {

VideoPlayer::VideoPlayer(const VideoPlayerDesc& desc) : _desc(desc) {
    logDEBUG("VideoPlayer initialized.");

    VideoDecoderDesc ddesc{};

#if NV_USE_FFMPEG
    _decoder = nv::create<FFMPEGVideoDecoder>(ddesc);
#endif

    // Check the decoder is valid:
    NVCHK(_decoder != nullptr, "VideoDecoder was not assigned.");

    if (!desc.videoFile.empty()) {
        open_file(desc.videoFile.c_str());
    }
};

VideoPlayer::~VideoPlayer() { stop(); };

auto VideoPlayer::open_file(const char* filename) -> bool {
    if (_decoder == nullptr) {
        logERROR("VideoPlayer: No decoder, cannot open file.");
        return false;
    }

    if (!system_file_exists(filename)) {
        logERROR("VideoPlayer: video file '{}' doesn't exists.", filename);
        return false;
    }

    logDEBUG("Opening video file {}...", filename);
    if (!_decoder->open_input(filename)) {
        logERROR("VideoPlayer: decoder cannot open input.");
        return false;
    }

    _filename = filename;

    if (_desc.playOnOpen) {
        play();
    }

    return true;
};

void VideoPlayer::play() {
    NVCHK(_decoder != nullptr, "Invalid decoder.");

    _isPlaying = true;
    _playbackStartTick = SystemTime::tick();
    _lastUpdateTick = -1;
    _playTime = 0.0;
    _currentFrameIndex = 0;
    logDEBUG("Started playing video {}", _filename);

    auto* eng = WGPUEngine::instance();
    _updateCb = eng->add_pre_render_func([this] { update(); });
};

void VideoPlayer::update() {
    if (!_isPlaying)
        return;

    auto curTick = SystemTime::tick();
    if (_lastUpdateTick == -1) {
        _lastUpdateTick = curTick;
    }
    auto elapsed = SystemTime::delta_s(_lastUpdateTick, curTick) * _videoSpeed;
    _lastUpdateTick = curTick;
    _playTime += elapsed;

    F64 videoFps = _decoder->get_fps();
    I32 expectedFrameIndex = static_cast<I32>(_playTime * videoFps);

    // Only decode if we need to advance to the next frame:
    if (expectedFrameIndex > _currentFrameIndex) {
        I32 delta = expectedFrameIndex - _currentFrameIndex;
        if (delta > 1) {
            logWARN("Jumping over {} video frames.", delta - 1);
        }

        if (_decoder->decode_next_frame()) {
            _currentFrameIndex = expectedFrameIndex;
            // logDEBUG("Current video frame: {}", _currentFrameIndex);
            // get the texture data:
            _decoder->get_current_frame(_texture, _origin);
        } else {
            // End of video reached
            logDEBUG("No additional frame, stopping playback.");
            stop();
        }
    }
};

void VideoPlayer::pause() { logDEBUG("Should pause the video {}", _filename); };

void VideoPlayer::stop() {
    if (!_updateCb) {
        return;
    }

    logDEBUG("Stopping playback of the video {}", _filename);
    auto* eng = WGPUEngine::instance();
    eng->remove_pre_render_callback(_updateCb);
    _isPlaying = false;
    _updateCb = nullptr;
};

auto VideoPlayer::create(const VideoPlayerDesc& desc) -> RefPtr<VideoPlayer> {
    return nv::create<VideoPlayer>(desc);
}

auto VideoPlayer::get_fps() const -> F64 {
    return _decoder != nullptr ? _decoder->get_fps() : -1;
}
auto VideoPlayer::get_height() const -> I32 {
    return _decoder != nullptr ? _decoder->get_frame_height() : -1;
}
auto VideoPlayer::get_width() const -> I32 {
    return _decoder != nullptr ? _decoder->get_frame_width() : -1;
}

void VideoPlayer::set_target_texture(wgpu::Texture texture, const Vec3u& orig) {
    _texture = std::move(texture);
    _origin = orig;
}

} // namespace nv
