export module HAL:Autogen.Tables.RaytracingRays;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.GBuffer;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct RaytracingRays
	{
		static constexpr SlotID ID = SlotID::RaytracingRays;
		float pixelAngle;
		HLSL::RWTexture2D<float4> output;
		GBuffer gbuffer;
		HLSL::RWTexture2D<float4>& GetOutput() { return output; }
		float& GetPixelAngle() { return pixelAngle; }
		GBuffer& GetGbuffer() { return gbuffer; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(pixelAngle);
			compiler.compile(output);
			compiler.compile(gbuffer);
		}
		struct Compiled
		{
			float pixelAngle; // float
			uint output; // RWTexture2D<float4>
			GBuffer::Compiled gbuffer; // GBuffer
		};
	};
	#pragma pack(pop)
}
