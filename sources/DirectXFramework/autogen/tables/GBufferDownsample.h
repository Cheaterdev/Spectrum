#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct GBufferDownsample
	{
		struct SRV
		{
			Render::HLSL::Texture2D<float4> normals;
			Render::HLSL::Texture2D<float> depth;
		} &srv;
		Render::HLSL::Texture2D<float4>& GetNormals() { return srv.normals; }
		Render::HLSL::Texture2D<float>& GetDepth() { return srv.depth; }
		GBufferDownsample(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
