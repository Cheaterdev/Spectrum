#pragma once
#include "sig_hlsl.hlsl"
struct DispatchParameters
{
	float SurfaceThickness; // float
	float BilinearThreshold; // float
	float ShadowContrast; // float
	bool IgnoreEdgePixels; // bool
	bool UsePrecisionOffset; // bool
	bool BilinearSamplingOffsetMode; // bool
	bool DebugOutputEdgeMask; // bool
	bool DebugOutputThreadIndex; // bool
	bool DebugOutputWaveIndex; // bool
	float2 DepthBounds; // float2
	bool UseEarlyOut; // bool
	float4 LightCoordinate; // float4
	int2 WaveOffset; // int2
	float FarDepthValue; // float
	float NearDepthValue; // float
	float2 InvDepthTextureSize; // float2
	uint DepthTexture; // Texture2D<float>
	uint OutputTexture; // RWTexture2D<float4>
	float GetSurfaceThickness() { return SurfaceThickness; }
	float GetBilinearThreshold() { return BilinearThreshold; }
	float GetShadowContrast() { return ShadowContrast; }
	bool GetIgnoreEdgePixels() { return IgnoreEdgePixels; }
	bool GetUsePrecisionOffset() { return UsePrecisionOffset; }
	bool GetBilinearSamplingOffsetMode() { return BilinearSamplingOffsetMode; }
	bool GetDebugOutputEdgeMask() { return DebugOutputEdgeMask; }
	bool GetDebugOutputThreadIndex() { return DebugOutputThreadIndex; }
	bool GetDebugOutputWaveIndex() { return DebugOutputWaveIndex; }
	float2 GetDepthBounds() { return DepthBounds; }
	bool GetUseEarlyOut() { return UseEarlyOut; }
	float4 GetLightCoordinate() { return LightCoordinate; }
	int2 GetWaveOffset() { return WaveOffset; }
	float GetFarDepthValue() { return FarDepthValue; }
	float GetNearDepthValue() { return NearDepthValue; }
	float2 GetInvDepthTextureSize() { return InvDepthTextureSize; }
	Texture2D<float> GetDepthTexture() { return ResourceDescriptorHeap[DepthTexture]; }
	RWTexture2D<float4> GetOutputTexture() { return ResourceDescriptorHeap[OutputTexture]; }
};