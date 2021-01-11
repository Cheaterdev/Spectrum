#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBuffer
	{
		struct SRV
		{
			DX12::HLSL::Texture2D<float4> albedo;
			DX12::HLSL::Texture2D<float4> normals;
			DX12::HLSL::Texture2D<float4> specular;
			DX12::HLSL::Texture2D<float> depth;
			DX12::HLSL::Texture2D<float2> motion;
		} &srv;
		DX12::HLSL::Texture2D<float4>& GetAlbedo() { return srv.albedo; }
		DX12::HLSL::Texture2D<float4>& GetNormals() { return srv.normals; }
		DX12::HLSL::Texture2D<float4>& GetSpecular() { return srv.specular; }
		DX12::HLSL::Texture2D<float>& GetDepth() { return srv.depth; }
		DX12::HLSL::Texture2D<float2>& GetMotion() { return srv.motion; }
		GBuffer(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
