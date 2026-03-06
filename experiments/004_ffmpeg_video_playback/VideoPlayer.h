#ifndef NV_VIDEOPLAYER_H_
#define NV_VIDEOPLAYER_H_

#include <gpu_common.h>

namespace nv {

struct VideoPlayerDesc {
    String videoFile;
    bool playOnOpen{false};
};

class NVGPU_EXPORT VideoPlayer : public RefObject {
  public:
    explicit VideoPlayer(const VideoPlayerDesc& desc);
    ~VideoPlayer() override;

    auto open_file(const char* filename) -> bool;
    void play();
    void pause();
    void stop();

    static auto create(const VideoPlayerDesc& desc) -> RefPtr<VideoPlayer>;

    /** Get the frame width. */
    auto get_width() const -> I32;

    /** Get the frame height. */
    auto get_height() const -> I32;

    /** Get the framerate. */
    auto get_fps() const -> F64;

    /** Assign the target texture */
    void set_target_texture(wgpu::Texture texture, const Vec3u& orig);

  protected:
    VideoPlayerDesc _desc;
    RefPtr<VideoDecoder> _decoder;
    String _filename;
    bool _isPlaying{false};
    I64 _playbackStartTick{-1};
    I64 _lastUpdateTick{-1};
    F64 _videoSpeed{1.0};
    F64 _playTime{0.0};
    I32 _currentFrameIndex{0};
    RefPtr<WGPUEngine::RenderCallback> _updateCb;

    wgpu::Texture _texture;
    Vec3u _origin;

    // Update this player state.
    void update();
};

} // namespace nv

#endif
