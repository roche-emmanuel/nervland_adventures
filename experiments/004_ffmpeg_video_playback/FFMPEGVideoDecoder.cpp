#include <ffmpeg/FFMPEGVideoDecoder.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}

#ifdef DAWN_ENABLE_BACKEND_D3D11
#include <libavutil/hwcontext_d3d11va.h>
#endif

#ifdef DAWN_ENABLE_BACKEND_D3D12
#include <dawn/native/D3D12Backend.h>
#include <libavutil/hwcontext_d3d11va.h>
#include <libavutil/hwcontext_d3d12va.h>
#endif

using namespace wgpu;

namespace nv {

FFMPEGVideoDecoder::FFMPEGVideoDecoder(const VideoDecoderDesc& desc)
    : VideoDecoder(desc) {
    // Initialize FFmpeg (call once globally, but safe to call multiple times)
    avformat_network_init();

    logDEBUG("FFMPEGVideoDecoder initialized");
};

FFMPEGVideoDecoder::~FFMPEGVideoDecoder() { cleanup(); }

void FFMPEGVideoDecoder::cleanup() {
    logDEBUG("Cleaning up VideoDecoder.");

    if (_currentFrame != nullptr) {
        av_frame_free(&_currentFrame);
        _currentFrame = nullptr;
    }

    if (_swFrame != nullptr) {
        av_frame_free(&_swFrame);
        _swFrame = nullptr;
    }

    if (_codecCtx != nullptr) {
        avcodec_free_context(&_codecCtx);
        _codecCtx = nullptr;
    }

    if (_formatCtx != nullptr) {
        avformat_close_input(&_formatCtx);
        _formatCtx = nullptr;
    }

    if (_hwDeviceCtx != nullptr) {
        av_buffer_unref(&_hwDeviceCtx);
        _hwDeviceCtx = nullptr;
    }

    _isInitialized = false;
    _isHWAccelerated = false;
    _videoStreamIdx = -1;
}

auto FFMPEGVideoDecoder::open_input(const char* filename) -> bool {
    logDEBUG("FFMPEGVideoDecoder: Opening video file: {}", filename);

    // Clean up any previous state
    cleanup();

    // Initialize decoder for this file
    if (!initialize_decoder(filename)) {
        logERROR("Failed to initialize decoder");
        return false;
    }

    // Allocate frames
    _currentFrame = av_frame_alloc();
    _swFrame = av_frame_alloc();

    NVCHK(_currentFrame != nullptr && _swFrame != nullptr,
          "Failed to allocate AVFrame structures.");

    _isInitialized = true;
    logDEBUG("Video decoder ready - Resolution: {}x{}, FPS: {}, HW Accel: {}",
             _frameWidth, _frameHeight, _fps, _isHWAccelerated);
    return true;
}

auto FFMPEGVideoDecoder::initialize_decoder(const char* filename) -> bool {

    // Open input file
    _formatCtx = avformat_alloc_context();
    if (!_formatCtx) {
        logERROR("Failed to allocate format context");
        return false;
    }

    I32 ret = avformat_open_input(&_formatCtx, filename, nullptr, nullptr);
    if (ret < 0) {
        logDEBUG("Failed to open input file: {}", err2str(ret));
        return false;
    }

    // Retrieve stream information
    ret = avformat_find_stream_info(_formatCtx, nullptr);
    if (ret < 0) {
        logDEBUG("Failed to find stream info: {}", err2str(ret));
        return false;
    }

    // Find video stream
    _videoStreamIdx =
        av_find_best_stream(_formatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (_videoStreamIdx < 0) {
        logDEBUG("No video stream found");
        return false;
    }

    AVStream* video_stream = _formatCtx->streams[_videoStreamIdx];
    const AVCodec* codec =
        avcodec_find_decoder(video_stream->codecpar->codec_id);
    if (codec == nullptr) {
        logDEBUG("Unsupported codec");
        return false;
    }

    // Calculate FPS
    if (video_stream->r_frame_rate.den != 0) {
        _fps = av_q2d(video_stream->r_frame_rate);
    } else if (video_stream->avg_frame_rate.den != 0) {
        _fps = av_q2d(video_stream->avg_frame_rate);
    } else {
        _fps = 25.0; // Default fallback
    }

    // Store frame dimensions
    _frameWidth = video_stream->codecpar->width;
    _frameHeight = video_stream->codecpar->height;

    // Try to setup hardware decoder first
    _isHWAccelerated = false;
    if (_desc.enableHardwareAcceleration && setup_hardware_decoder(codec)) {
        logDEBUG("Hardware acceleration enabled");
        _isHWAccelerated = true;
    } else {
        logDEBUG("Using software decoding");
        // Fallback to software decoder
        _codecCtx = avcodec_alloc_context3(codec);
        if (_codecCtx == nullptr) {
            logDEBUG("Failed to allocate codec context");
            return false;
        }
    }

    // Copy codec parameters
    ret = avcodec_parameters_to_context(_codecCtx, video_stream->codecpar);
    if (ret < 0) {
        logDEBUG("Failed to copy codec parameters: {}", err2str(ret));
        return false;
    }

    // Open codec
    ret = avcodec_open2(_codecCtx, codec, nullptr);
    if (ret < 0) {
        logDEBUG("Failed to open codec: {}", err2str(ret));
        return false;
    }

    return true;
}

auto FFMPEGVideoDecoder::setup_hardware_decoder(const AVCodec* codec) -> bool {
#ifdef DAWN_ENABLE_BACKEND_D3D12
#if NV_FFMPEG_DX_VERSION == 11
    AVHWDeviceType hw_type = AV_HWDEVICE_TYPE_D3D11VA;
    AVPixelFormat px_fmt = AV_PIX_FMT_D3D11;
#else
    AVHWDeviceType hw_type = AV_HWDEVICE_TYPE_D3D12VA;
    AVPixelFormat px_fmt = AV_PIX_FMT_D3D12;
#endif

    AVBufferRef* hwDeviceCtx = av_hwdevice_ctx_alloc(hw_type);
    NVCHK(hwDeviceCtx != nullptr, "Cannot allocate HW Device context for {}.",
          av_hwdevice_get_type_name(hw_type));

    auto* hw_device_ctx = (AVHWDeviceContext*)hwDeviceCtx->data;

// Set the custom device here:
#if NV_FFMPEG_DX_VERSION == 11
    auto* d3d_ctx = (AVD3D11VADeviceContext*)hw_device_ctx->hwctx;
    d3d_ctx->device = DX11Engine::instance().device();
#else
    auto* d3d_ctx = (AVD3D12VADeviceContext*)hw_device_ctx->hwctx;
    d3d_ctx->device = DX12Engine::instance().device();
#endif
    NVCHK(d3d_ctx->device != nullptr,
          "Invalid D3D Device for FFMPEG hw context.");
    d3d_ctx->device->AddRef();
#else
#error "No implementation for FFMPEGVideoDecoder::setup_hardware_decoder yet."
#endif

    // Initialize the context
    int ret = av_hwdevice_ctx_init(hwDeviceCtx);
    NVCHK(ret >= 0, "FFMPEGVideoDecoder: Cannot initialize HW Device context.");
    _hwDeviceCtx = hwDeviceCtx;

    _codecCtx = avcodec_alloc_context3(codec);
    if (_codecCtx != nullptr) {
        _codecCtx->hw_device_ctx = av_buffer_ref(_hwDeviceCtx);
        _hwPixelFormat = px_fmt;
        logDEBUG("Hardware decoder setup successful: {}",
                 av_hwdevice_get_type_name(hw_type));
        return true;
    }

    logDEBUG("No suitable hardware acceleration found.");
    return false;

#ifdef DAWN_ENABLE_BACKEND_D3D11
#error "No support yet for wgpu D3D11 backend with FFMPEG."
    // Could try with AV_HWDEVICE_TYPE_D3D11VA here.
#endif

#if 0
    // Check if codec supports hardware acceleration
    for (int i = 0;; i++) {
        const AVCodecHWConfig* config = avcodec_get_hw_config(codec, i);
        if (config == nullptr) {
            break;
        }

        if (!(config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX)) {
            continue;
        }

        // Try different hardware acceleration types
        AVHWDeviceType hw_types[] = {
#ifdef _WIN32
            win_hw_type,
        // AV_HWDEVICE_TYPE_D3D12VA, // Windows primary
        // AV_HWDEVICE_TYPE_D3D11VA, // Windows secondary
        // AV_HWDEVICE_TYPE_DXVA2,   // Windows fallback
#endif
#ifdef __linux__
            AV_HWDEVICE_TYPE_VAAPI, // Linux
            AV_HWDEVICE_TYPE_VDPAU, // Linux alternative
#endif
#ifdef __APPLE__
            AV_HWDEVICE_TYPE_VIDEOTOOLBOX // macOS
#endif
        };

        for (auto hw_type : hw_types) {
            if (hw_type == config->device_type) {
                I32 ret = av_hwdevice_ctx_create(&_hwDeviceCtx, hw_type,
                                                 nullptr, nullptr, 0);
                if (ret >= 0) {
                    _codecCtx = avcodec_alloc_context3(codec);
                    if (_codecCtx != nullptr) {
                        _codecCtx->hw_device_ctx = av_buffer_ref(_hwDeviceCtx);
                        _hwPixelFormat = config->pix_fmt;
                        logDEBUG("Hardware decoder setup successful: {}",
                                 av_hwdevice_get_type_name(hw_type));
                        return true;
                    }
                }
            }
        }
    }

    logDEBUG("No suitable hardware acceleration found.");
    return false;
#endif
}

auto FFMPEGVideoDecoder::decode_next_frame() -> bool {
    if (!_isInitialized) {
        logERROR("Decoder not initialized.");
        return false;
    }

    return decode_frame();
}

auto FFMPEGVideoDecoder::decode_frame() -> bool {
    AVPacket* packet = av_packet_alloc();
    if (!packet) {
        logDEBUG("Failed to allocate packet.");
        return false;
    }

    I32 ret = 0;
    while ((ret = av_read_frame(_formatCtx, packet)) >= 0) {
        if (packet->stream_index == _videoStreamIdx) {
            ret = avcodec_send_packet(_codecCtx, packet);
            if (ret < 0) {
                if (ret != AVERROR(EAGAIN)) {
                    logERROR("Error sending packet: ", err2str(ret));
                    break;
                }
            }

            ret = avcodec_receive_frame(_codecCtx, _currentFrame);
            if (ret == 0) {
                // Frame decoded successfully
                if (_isHWAccelerated &&
                    _currentFrame->format == _hwPixelFormat) {
                    // Hardware decoded frame ready
                    av_packet_free(&packet);
                    return true;
                }

                if (!_isHWAccelerated) {
                    // Software decoded frame ready
                    av_packet_free(&packet);
                    return true;
                }

                if (_isHWAccelerated &&
                    _currentFrame->format != _hwPixelFormat) {
                    logDEBUG(
                        "Need to convert pixel format with software decoding.");

                    // Need to transfer from hardware to software frame
                    ret = av_hwframe_transfer_data(_swFrame, _currentFrame, 0);
                    if (ret < 0) {
                        logERROR("Failed to transfer hardware frame: {}",
                                 err2str(ret));
                        continue;
                    }

                    // Use software frame as current
                    av_frame_unref(_currentFrame);
                    av_frame_ref(_currentFrame, _swFrame);
                    av_packet_free(&packet);
                    return true;
                }
            } else if (ret != AVERROR(EAGAIN)) {
                logERROR("Error receiving frame: {}", err2str(ret));
                break;
            }
        }

        av_packet_unref(packet);
    }

    av_packet_free(&packet);

    if (ret == AVERROR_EOF) {
        logDEBUG("End of file reached.");
    }

    return false;
}

auto FFMPEGVideoDecoder::get_current_frame(const Texture& texture,
                                           const Vec3u& origin) -> bool {
    return update_texture_from_frame(texture, origin, _currentFrame);
}

#ifdef _WIN32
static auto convert_dxgi_to_wgpu_format(DXGI_FORMAT fmt) -> TextureFormat {
    switch (fmt) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return TextureFormat::RGBA8Unorm;
    case DXGI_FORMAT_NV12:
        return TextureFormat::R8BG8Biplanar420Unorm;
    default:
        THROW_MSG("Unsupported DXGI format {}", (I32)fmt);
        return TextureFormat::Undefined;
    }
}

static void init_dx12_texture_interface(Texture& texInterface,
                                        AVD3D12VAFrame* vaFrame) {
    NVCHK(vaFrame != nullptr, "Invalid DX12 VA Frame.");
    NVCHK(texInterface == nullptr, "Texture interface already initialized.");

    // Get the DX12 resource:
    auto* dx21_res = vaFrame->texture;

    // Get texture description for WebGPU texture creation
    D3D12_RESOURCE_DESC desc = dx21_res->GetDesc();

    // We expect this to be a Texture2D:
    if (desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D) {
        logWARN("Unexpected resource dim: {}", desc.Dimension);
    }

    I32 width = (I32)desc.Width;
    I32 height = (I32)desc.Height;
    I32 depth = desc.DepthOrArraySize;

    // TODO: Implement D3D12 to WebGPU texture copy
    logDEBUG("D3D12 texture ready for WebGPU import (size=({},{},{}), "
             "format={})",
             width, height, depth, (I32)desc.Format);

    // Create shared texture memory descriptor
    dawn::native::d3d12::SharedBufferMemoryD3D12ResourceDescriptor d3d12Desc{};
    d3d12Desc.resource = dx21_res;

    SharedTextureMemoryDescriptor sharedDesc{};
    sharedDesc.nextInChain = &d3d12Desc;

    // Import the D3D12 resource into WebGPU
    auto* eng = WGPUEngine::instance();
    SharedTextureMemory sharedMemory =
        eng->import_shared_texture_memory(&sharedDesc);

    // Create texture from shared memory
    TextureDescriptor texDesc{};
    texDesc.usage = TextureUsage::TextureBinding | TextureUsage::CopyDst;
    texDesc.dimension = TextureDimension::e2D;
    texDesc.size = {(U32)width, (U32)height, 1};
    // Convert DXGI format to WebGPU format as needed
    texDesc.format = convert_dxgi_to_wgpu_format(desc.Format);

    texInterface = sharedMemory.CreateTexture(&texDesc);
    NVCHK(texInterface != nullptr, "Cannot create texture interface.");

    logDEBUG("Shared texture width: {}", texInterface.GetWidth());
    logDEBUG("Shared texture height: {}", texInterface.GetHeight());
    logDEBUG("Shared texture depth: {}", texInterface.GetDepthOrArrayLayers());
    logDEBUG("Shared texture format: {}", (I32)texInterface.GetFormat());
}

void FFMPEGVideoDecoder::init_nv12_convert_prog(ID3D11Texture2D* srcTex) {
    auto& dx11 = DX11Engine::instance();
    auto* device = dx11.device();

    D3D11_TEXTURE2D_DESC desc;
    srcTex->GetDesc(&desc);

    // Create the intermediate nv12 texture:
    _nv12Texture = dx11.createTexture2D(
        desc.Width, desc.Height, D3D11_BIND_SHADER_RESOURCE, desc.Format);

    // NVCHK((desc.BindFlags & D3D11_BIND_SHADER_RESOURCE) != 0,
    //       "Missing bind shader resource on input texture!");

    // Create shader resource view for NV12 texture
    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx
    // To access DXGI_FORMAT_NV12 in the shader, we need to map the luminance
    // channel and the chrominance channels into a format that shaders can
    // understand. In the case of NV12, DirectX understands how the texture is
    // laid out, so we can create these shader resource views which represent
    // the two channels of the NV12 texture. Then inside the shader we convert
    // YUV into RGB so we can render.

    // DirectX specifies the view format to be DXGI_FORMAT_R8_UNORM for NV12
    // luminance channel. Luminance is 8 bits per pixel. DirectX will handle
    // converting 8-bit integers into normalized floats for use in the shader.
    D3D11_SHADER_RESOURCE_VIEW_DESC luminancePlaneDesc{};
    luminancePlaneDesc.Format = DXGI_FORMAT_R8_UNORM;
    luminancePlaneDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    luminancePlaneDesc.Texture2D.MipLevels = 1;
    luminancePlaneDesc.Texture2D.MostDetailedMip = 0;

    HRESULT hr = device->CreateShaderResourceView(
        _nv12Texture.Get(), &luminancePlaneDesc, _luminanceSRV.GetAddressOf());
    NVCHK(SUCCEEDED(hr), "Failed to create luminance SRV");

    D3D11_SHADER_RESOURCE_VIEW_DESC chromaPlaneDesc{};
    chromaPlaneDesc.Format = DXGI_FORMAT_R8G8_UNORM;
    chromaPlaneDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    chromaPlaneDesc.Texture2D.MipLevels = 1;
    chromaPlaneDesc.Texture2D.MostDetailedMip = 0;

    hr = device->CreateShaderResourceView(_nv12Texture.Get(), &chromaPlaneDesc,
                                          _chromaSRV.GetAddressOf());
    NVCHK(SUCCEEDED(hr), "Failed to create chroma SRV");

    // Create unordered access view for RGBA output
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;

    hr = device->CreateUnorderedAccessView(_rgbaTexture.Get(), &uavDesc,
                                           &_rgbaUAV);
    NVCHK(SUCCEEDED(hr), "Failed to create RGBA UAV");

    // Create constant buffer for layer index
    _layerBuffer = dx11.createConstantBuffer(sizeof(U32) * 4);

    // Create compute shader program for NV12 to RGBA conversion
    _nv12ToRgbaProgram =
        dx11.createComputeProgram("assets/shaders/dx11/nv12_to_rgba.hlsl");
}

void FFMPEGVideoDecoder::convert_nv12_to_rgba(ID3D11Texture2D* srcTex,
                                              U32 layerIdx) {
    auto& dx11 = DX11Engine::instance();
    auto* context = dx11.context();

    // Copy to our intermediate texture since the decoder output cannot be used
    // as shader resource. Copy specific array slice to the single-layer texture
    UINT srcSubresource = D3D11CalcSubresource(0,        // mip level
                                               layerIdx, // array slice
                                               1);       // mip levels
    UINT dstSubresource = 0;                             // Single layer, mip 0

    context->CopySubresourceRegion(_nv12Texture.Get(), dstSubresource, 0, 0,
                                   0, // Dest x, y, z
                                   srcTex, srcSubresource,
                                   nullptr // Copy entire subresource
    );

    // Get output texture dimensions
    D3D11_TEXTURE2D_DESC desc;
    _rgbaTexture->GetDesc(&desc);

    // Set compute shader
    context->CSSetShader(_nv12ToRgbaProgram.computeShader, nullptr, 0);

    // Bind input texture arrays
    ID3D11ShaderResourceView* srvs[] = {_luminanceSRV.Get(), _chromaSRV.Get()};
    context->CSSetShaderResources(0, 2, srvs);

    // Bind output texture
    ID3D11UnorderedAccessView* uavs[] = {_rgbaUAV.Get()};
    context->CSSetUnorderedAccessViews(0, 1, uavs, nullptr);

    // Dispatch compute shader
    UINT groupsX = (desc.Width + 7) / 8;
    UINT groupsY = (desc.Height + 7) / 8;
    context->Dispatch(groupsX, groupsY, 1);

    // Unbind resources
    ID3D11ShaderResourceView* nullSRVs[] = {nullptr, nullptr};
    context->CSSetShaderResources(0, 2, nullSRVs);
    ID3D11UnorderedAccessView* nullUAVs[] = {nullptr};
    context->CSSetUnorderedAccessViews(0, 1, nullUAVs, nullptr);
    // ID3D11Buffer* nullCBs[] = {nullptr};
    // context->CSSetConstantBuffers(0, 1, nullCBs);
    context->CSSetShader(nullptr, nullptr, 0);
}

void FFMPEGVideoDecoder::init_dx11_texture_interface(ID3D11Texture2D* srcTex) {
    // NVCHK(layerIdx == 0, "Unexpected layer index: {}", layerIdx);
    NVCHK(srcTex != nullptr, "Invalid DX11 input texture.");
    NVCHK(_textureInterface == nullptr,
          "Texture interface already initialized.");
    NVCHK(_rgbaTexture == nullptr, "RGBA Texture already initialized.");

    // Get the desc of the source texture:
    D3D11_TEXTURE2D_DESC tdesc;
    srcTex->GetDesc(&tdesc);

    // We expect the format to be NV12:
    NVCHK(tdesc.Format == DXGI_FORMAT_NV12, "Unexpected source tex format: {}",
          tdesc.Format);

    // Now create a corresponding RGBA Teture that is shared:
    logDEBUG("FFMPEGVideoDecoder: Creating target RGBA Texture.");

    auto& dx11 = DX11Engine::instance();
    HANDLE sharedHandle{nullptr};
    _rgbaTexture = dx11.createReadOnlySharedTexture2D(
        &sharedHandle, tdesc.Width, tdesc.Height,
        D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE |
            D3D11_BIND_UNORDERED_ACCESS);

    // Initialize the conversion pipeline
    init_nv12_convert_prog(srcTex);

    // Open the DX11 texture in Dawn from the shared handle and return it as a
    // WebGPU texture.
    SharedTextureMemoryDXGISharedHandleDescriptor sharedHandleDesc{};
    sharedHandleDesc.handle = sharedHandle;

    SharedTextureMemoryDescriptor desc;
    desc.nextInChain = &sharedHandleDesc;

    auto* eng = WGPUEngine::instance();
    _sharedTexMem = eng->import_shared_texture_memory(&desc);
    // Handle is no longer needed once resources are created.
    ::CloseHandle(sharedHandle);

    TextureDescriptor texDesc{};
    texDesc.usage = TextureUsage::TextureBinding | TextureUsage::CopyDst;
    texDesc.dimension = TextureDimension::e2D;
    texDesc.size = {tdesc.Width, tdesc.Height, 1};
    // Convert DXGI format to WebGPU format as needed
    texDesc.format = convert_dxgi_to_wgpu_format(DXGI_FORMAT_R8G8B8A8_UNORM);

    _textureInterface = _sharedTexMem.CreateTexture(&texDesc);

    logDEBUG("Shared texture width: {}", _textureInterface.GetWidth());
    logDEBUG("Shared texture height: {}", _textureInterface.GetHeight());
    logDEBUG("Shared texture depth: {}",
             _textureInterface.GetDepthOrArrayLayers());
    logDEBUG("Shared texture format: {}", (I32)_textureInterface.GetFormat());
}

#endif

auto FFMPEGVideoDecoder::update_texture_from_frame(const Texture& texture,
                                                   const Vec3u& origin,
                                                   AVFrame* hw_frame) -> bool {
#ifdef _WIN32
    // logDEBUG("Frame format is: {}", hw_frame->format);

    if (_isHWAccelerated && hw_frame->format == AV_PIX_FMT_D3D12) {
        logDEBUG("Initializing DX12 texture interface.");
        auto* vaframe = (AVD3D12VAFrame*)hw_frame->data[0];
        logDEBUG("DX12 src tex: {}", (const void*)vaframe->texture);
        if (_textureInterface == nullptr) {
            init_dx12_texture_interface(_textureInterface, vaframe);
        }
    }

    if (_isHWAccelerated && hw_frame->format == AV_PIX_FMT_D3D11) {
        auto* d3d_texture = (ID3D11Texture2D*)hw_frame->data[0];
        I64 idx = (I64)(intptr_t)hw_frame->data[1];
        // logDEBUG("DX11 src tex: {}, layer: {}", (const void*)d3d_texture,
        // idx);
        if (_textureInterface == nullptr) {
            init_dx11_texture_interface(d3d_texture);
        }
        convert_nv12_to_rgba(d3d_texture, idx);
    }
#endif

    // Fallback: copy software frame data to WebGPU texture
    if (_textureInterface == nullptr && hw_frame->data[0]) {
        logDEBUG("Software frame ready for WebGPU copy");
    }

    if (_textureInterface != nullptr) {
        if (_copyPass == nullptr) {
            logDEBUG("Creating texture copy compute pass.");
            _copyPass = WGPUComputePass::create_copy_texture_pass(
                _textureInterface, texture, nullptr, &origin);
        }

        SharedTextureMemoryBeginAccessDescriptor beginDesc{};
        beginDesc.initialized = true;
        beginDesc.concurrentRead = false;
        beginDesc.fenceCount = 0;
        beginDesc.fences = nullptr;
        beginDesc.signaledValues = nullptr;

        if (!_sharedTexMem.BeginAccess(_textureInterface, &beginDesc)) {
            logERROR("Cannot begin access to shared texture.");
        }

        // eng->copy_texture(_textureInterface, texture, nullptr, &origin);
        _copyPass->execute();

        SharedTextureMemoryEndAccessState endDesc{};

        if (!_sharedTexMem.EndAccess(_textureInterface, &endDesc)) {
            logERROR("Cannot end access to shared texture.");
        }
    }

    // logDEBUG("Should copy texture interface here.");

    return true;
}

auto FFMPEGVideoDecoder::err2str(int ret) -> char* {
    av_make_error_string(_errBuf, 64, ret);
    return _errBuf;
}
} // namespace nv
