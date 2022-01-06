#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct SkyData
	{
		float3 sunDir;
		Render::HLSL::Texture2D<float> depthBuffer;
		Render::HLSL::Texture2D<float4> transmittance;
		Render::HLSL::Texture3D<float4> inscatter;
		Render::HLSL::Texture2D<float4> irradiance;
		float3& GetSunDir() { return sunDir; }
		Render::HLSL::Texture2D<float>& GetDepthBuffer() { return depthBuffer; }
		Render::HLSL::Texture2D<float4>& GetTransmittance() { return transmittance; }
		Render::HLSL::Texture3D<float4>& GetInscatter() { return inscatter; }
		Render::HLSL::Texture2D<float4>& GetIrradiance() { return irradiance; }
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
