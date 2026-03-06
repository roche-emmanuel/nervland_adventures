#include <video/VideoDecoder.h>

namespace nv {

VideoDecoder::VideoDecoder(const VideoDecoderDesc& desc) : _desc(desc) {
    logDEBUG("VideoDecoder initialized.");
};

VideoDecoder::~VideoDecoder() = default;

} // namespace nv
