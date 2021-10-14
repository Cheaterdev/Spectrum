#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct TilingParams
	{
		struct SRV
		{
			Render::HLSL::StructuredBuffer<uint2> tiles;
		} &srv;
		Render::HLSL::StructuredBuffer<uint2>& GetTiles() { return srv.tiles; }
		TilingParams(SRV&srv) :srv(srv){}
	};
	#pragma pack(pop)
}
