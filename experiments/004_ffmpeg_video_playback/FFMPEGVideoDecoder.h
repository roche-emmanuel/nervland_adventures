#ifndef NV_FFMPEGVIDEODECODER_H_
#define NV_FFMPEGVIDEODECODER_H_

#include <gpu_common.h>

#ifdef _WIN32
#include <dx/DX11Engine.h>
#include <dx/DX12Engine.h>
#endif

#include <video/VideoDecoder.h>

namespace nv {

class NVGPU_EXPORT FFMPEGVideoDecoder : public VideoDecoder {

  public:
    explicit FFMPEGVideoDecoder(const VideoDecoderDesc& desc);
    ~FFMPEGVideoDecoder() override;

    // Load a video file
    auto open_input(const char* filename) -> bool override;

    // Decode next frame
    auto decode_next_frame() -> bool override;

    // Get current hardware frame (valid after successful decode_next_frame)
    // auto get_current_frame() -> AVFrame* { return _currentFrame; }
    auto get_current_frame(const wgpu::Texture& texture, const Vec3u& origin)
        -> bool override;

  protected:
    AVFormatContext* _formatCtx{nullptr};
    AVCodecContext* _codecCtx{nullptr};
    AVBufferRef* _hwDeviceCtx{nullptr};
    AVFrame* _currentFrame{nullptr};
    AVFrame* _swFrame{nullptr}; // For software fallback

    I32 _videoStreamIdx{};
    I32 _hwPixelFormat = -1;

    bool _isInitialized{false};

    /** Target shared texture interface. */
    wgpu::Texture _textureInterface;

    // Initialize hardware decoder context
    auto setup_hardware_decoder(const AVCodec* codec) -> bool;

    // Initialize the decoder for a specific file
    auto initialize_decoder(const char* filename) -> bool;

    // Cleanup resources
    void cleanup();

    // Decode a frame
    auto decode_frame() -> bool;

    // Platform-specific helpers
    auto update_texture_from_frame(const wgpu::Texture& texture,
                                   const Vec3u& origin, AVFrame* hw_frame)
        -> bool;

    /** Buffer for ffmpeg error strings */
    char _errBuf[64]{0};

    auto err2str(int ret) -> char*;

#if NV_FFMPEG_DX_VERSION == 11
    ComPtr<ID3D11Texture2D> _nv12Texture;
    ComPtr<ID3D11Texture2D> _rgbaTexture;
    wgpu::SharedTextureMemory _sharedTexMem;
    RefPtr<WGPUComputePass> _copyPass;
    ComPtr<ID3D11ShaderResourceView> _luminanceSRV;
    ComPtr<ID3D11ShaderResourceView> _chromaSRV;
    ComPtr<ID3D11UnorderedAccessView> _rgbaUAV;
    ComPtr<ID3D11Buffer> _layerBuffer; // Constant buffer for layer index
    DX11Program _nv12ToRgbaProgram;

    void init_dx11_texture_interface(ID3D11Texture2D* srcTex);
    void init_nv12_convert_prog(ID3D11Texture2D* srcTex);
    void convert_nv12_to_rgba(ID3D11Texture2D* srcTex, U32 layerIdx);
#endif
};

} // namespace nv

#endif
