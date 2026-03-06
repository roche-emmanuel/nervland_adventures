#ifndef NV_VIDEODECODER_H_
#define NV_VIDEODECODER_H_

#include <gpu_common.h>

namespace nv {

struct VideoDecoderDesc {
    bool enableHardwareAcceleration{true};
};

class NVGPU_EXPORT VideoDecoder : public RefObject {
  public:
    explicit VideoDecoder(const VideoDecoderDesc& desc);
    ~VideoDecoder() override;

    // Open a given input file.
    virtual auto open_input(const char* filename) -> bool = 0;

    // decode the next frame:
    virtual auto decode_next_frame() -> bool = 0;

    // Copy the current frame to a texture:
    virtual auto get_current_frame(const wgpu::Texture& texture,
                                   const Vec3u& origin) -> bool = 0;

    /** Get the frame width. */
    auto get_frame_width() const -> I32 { return _frameWidth; }

    /** Get the frame height. */
    auto get_frame_height() const -> I32 { return _frameHeight; }

    /** Get the framerate. */
    auto get_fps() const -> F64 { return _fps; }

    /** Check hardware acceleration. */
    auto is_hardware_accelerated() const -> bool { return _isHWAccelerated; }

  protected:
    VideoDecoderDesc _desc;
    I32 _frameWidth{-1};
    I32 _frameHeight{-1};
    F64 _fps{-1.0};
    bool _isHWAccelerated{false};
};

} // namespace nv

#endif
