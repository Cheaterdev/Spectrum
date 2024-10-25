export module HAL:Autogen.Tables.DispatchParameters;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
import <HAL.h>;
export namespace Table
{
	#pragma pack(push, 1)

	struct DispatchParameters
	{
		static constexpr SlotID ID = SlotID::DispatchParameters;
		float SurfaceThickness = 0.005;
		float BilinearThreshold = 0.02;
		float ShadowContrast = 4;
		bool IgnoreEdgePixels = false;
		bool UsePrecisionOffset = false;
		bool BilinearSamplingOffsetMode = false;
		bool DebugOutputEdgeMask = false;
		bool DebugOutputThreadIndex = false;
		bool DebugOutputWaveIndex = false;
		float2 DepthBounds = float2(0,1);
		bool UseEarlyOut = false;
		float4 LightCoordinate;
		int2 WaveOffset;
		float FarDepthValue;
		float NearDepthValue;
		float2 InvDepthTextureSize;
		HLSL::Texture2D<float> DepthTexture;
		HLSL::RWTexture2D<float4> OutputTexture;
		float& GetSurfaceThickness() { return SurfaceThickness; }
		float& GetBilinearThreshold() { return BilinearThreshold; }
		float& GetShadowContrast() { return ShadowContrast; }
		bool& GetIgnoreEdgePixels() { return IgnoreEdgePixels; }
		bool& GetUsePrecisionOffset() { return UsePrecisionOffset; }
		bool& GetBilinearSamplingOffsetMode() { return BilinearSamplingOffsetMode; }
		bool& GetDebugOutputEdgeMask() { return DebugOutputEdgeMask; }
		bool& GetDebugOutputThreadIndex() { return DebugOutputThreadIndex; }
		bool& GetDebugOutputWaveIndex() { return DebugOutputWaveIndex; }
		float2& GetDepthBounds() { return DepthBounds; }
		bool& GetUseEarlyOut() { return UseEarlyOut; }
		float4& GetLightCoordinate() { return LightCoordinate; }
		int2& GetWaveOffset() { return WaveOffset; }
		float& GetFarDepthValue() { return FarDepthValue; }
		float& GetNearDepthValue() { return NearDepthValue; }
		float2& GetInvDepthTextureSize() { return InvDepthTextureSize; }
		HLSL::Texture2D<float>& GetDepthTexture() { return DepthTexture; }
		HLSL::RWTexture2D<float4>& GetOutputTexture() { return OutputTexture; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(SurfaceThickness);
			compiler.compile(BilinearThreshold);
			compiler.compile(ShadowContrast);
			compiler.compile(IgnoreEdgePixels);
			compiler.compile(UsePrecisionOffset);
			compiler.compile(BilinearSamplingOffsetMode);
			compiler.compile(DebugOutputEdgeMask);
			compiler.compile(DebugOutputThreadIndex);
			compiler.compile(DebugOutputWaveIndex);
			compiler.compile(DepthBounds);
			compiler.compile(UseEarlyOut);
			compiler.compile(LightCoordinate);
			compiler.compile(WaveOffset);
			compiler.compile(FarDepthValue);
			compiler.compile(NearDepthValue);
			compiler.compile(InvDepthTextureSize);
			compiler.compile(DepthTexture);
			compiler.compile(OutputTexture);
		}
		struct Compiled
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
		};

	};
	#pragma pack(pop)
}

