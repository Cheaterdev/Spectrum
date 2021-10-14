#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBuffer
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float4> albedo;
			Render::HLSL::Texture2D<float4> normals;
			Render::HLSL::Texture2D<float4> specular;
			Render::HLSL::Texture2D<float> depth;
			Render::HLSL::Texture2D<float2> motion;
		} &srv;
		Render::HLSL::Texture2D<float4>& GetAlbedo() { return srv.albedo; }
		Render::HLSL::Texture2D<float4>& GetNormals() { return srv.normals; }
		Render::HLSL::Texture2D<float4>& GetSpecular() { return srv.specular; }
		Render::HLSL::Texture2D<float>& GetDepth() { return srv.depth; }
		Render::HLSL::Texture2D<float2>& GetMotion() { return srv.motion; }
		GBuffer(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
