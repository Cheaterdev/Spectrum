export module HAL:Autogen.Tables.SkyData;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct SkyData
	{
		static constexpr SlotID ID = SlotID::SkyData;
		float3 sunDir;
		HLSL::Texture2D<float> depthBuffer;
		HLSL::Texture2D<float4> transmittance;
		HLSL::Texture3D<float4> inscatter;
		HLSL::Texture2D<float4> irradiance;
		float3& GetSunDir() { return sunDir; }
		HLSL::Texture2D<float>& GetDepthBuffer() { return depthBuffer; }
		HLSL::Texture2D<float4>& GetTransmittance() { return transmittance; }
		HLSL::Texture3D<float4>& GetInscatter() { return inscatter; }
		HLSL::Texture2D<float4>& GetIrradiance() { return irradiance; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(sunDir);
			compiler.compile(depthBuffer);
			compiler.compile(transmittance);
			compiler.compile(inscatter);
			compiler.compile(irradiance);
		}
		struct Compiled
		{
			float3 sunDir; // float3
			uint depthBuffer; // Texture2D<float>
			uint transmittance; // Texture2D<float4>
			uint inscatter; // Texture3D<float4>
			uint irradiance; // Texture2D<float4>
		};
	};
	#pragma pack(pop)
}
