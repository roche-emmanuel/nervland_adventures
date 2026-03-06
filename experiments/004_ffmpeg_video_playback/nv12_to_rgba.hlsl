// nv12_to_rgba.hlsl
// Compute shader to convert NV12 format to RGBA8 (separated Y and UV planes)

// Input texture arrays - Y plane and UV plane separated
Texture2D<float> lumTexture : register(t0); // Y plane array (single channel)
Texture2D<float2> chromaTexture : register(t1); // UV plane array (two channels)

// Output RGBA texture
RWTexture2D<float4> rgbaTexture : register(u0);

// YUV to RGB conversion matrix (BT.709)
// From Microsoft documentation for 8-bit YUV to RGB888
static const float3x3 YUVtoRGBCoeffMatrix = {1.164383f, 1.164383f,  1.164383f,
                                             0.000000f, -0.391762f, 2.017232f,
                                             1.596027f, -0.812968f, 0.000000f};

float3 ConvertYUVtoRGB(float3 yuv) {
  // Subtract the offset values (16/255 for Y, 128/255 for UV)
  yuv -= float3(0.062745f, 0.501960f, 0.501960f);
  yuv = mul(yuv, YUVtoRGBCoeffMatrix);
  return saturate(yuv);
}

[numthreads(8, 8, 1)] void cs_main(uint3 id : SV_DispatchThreadID) {
  uint2 outputSize;
  rgbaTexture.GetDimensions(outputSize.x, outputSize.y);

  // Check bounds
  if (id.x >= outputSize.x || id.y >= outputSize.y)
    return;

  // Sample Y component at full resolution from the specified layer
  float Y = lumTexture.Load(uint3(id.xy, 0));

  // Sample UV components at half resolution from the specified layer
  uint2 uvCoord = uint2(id.x / 2, id.y / 2);
  float2 UV = chromaTexture.Load(uint3(uvCoord, 0));

  // Convert YUV to RGB
  float3 rgb = ConvertYUVtoRGB(float3(Y, UV.x, UV.y));

  // Output RGBA (with full alpha)
  // Note: we flip the image vertically here to match our nervland convention:
  // rgbaTexture[id.xy] = float4(rgb, 1.0f);
  rgbaTexture[uint2(id.x, outputSize.y - id.y - 1)] = float4(rgb, 1.0f);
}