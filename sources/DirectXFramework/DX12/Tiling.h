#pragma once
namespace DX12
{
	class ResourceHeap;



	class TiledResourceManager
	{

		std::vector<grid<uint3, ResourceTile>> tiles;
	protected:

		//virtual	ComPtr<ID3D12Resource>& get_d3d_resource() = 0;
		void init_tilings();

	
	public:
		void map_buffer_range(size_t from, size_t to);
		void map_buffer_part( size_t offset, size_t size);
	};
}