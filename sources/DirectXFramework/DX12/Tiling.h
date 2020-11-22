#pragma once
namespace DX12
{
	class ResourceHeap;


	struct TileHeapPosition
	{
		ResourceHeap* heap = nullptr;
		UINT64 offset;
	};


	struct ResourceTile
	{
		ivec3 pos;
		UINT mip;
		UINT array_index;
		TileHeapPosition heap_position;
	};


	class TiledResourceManager
	{

		std::vector<std::vector<ResourceTile>> tiles;
	protected:

		virtual	ComPtr<ID3D12Resource>& get_d3d_resource() = 0;
		void init_tilings();

	
	public:
		void map_buffer_tile(size_t x);
		void map_buffer_part(size_t offset, size_t size);
	};
}