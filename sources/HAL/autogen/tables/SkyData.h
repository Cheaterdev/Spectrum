#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SkyData
	{
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
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(sunDir);
			compiler.compile(depthBuffer);
			compiler.compile(transmittance);
			compiler.compile(inscatter);
			compiler.compile(irradiance);
		}
	};
	#pragma pack(pop)
}