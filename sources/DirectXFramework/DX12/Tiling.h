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
		ComPtr<ID3D12Resource>& m_Resource;


		std::vector<std::vector<ResourceTile>> tiles;
	protected:

		TiledResourceManager(ComPtr<ID3D12Resource>& m_Resource) :m_Resource(m_Resource)
		{

		}

		void init_tilings();

	
	public:
		void map_buffer_tile(int x);
		void map_buffer_part(size_t offset, size_t size);
	};
}