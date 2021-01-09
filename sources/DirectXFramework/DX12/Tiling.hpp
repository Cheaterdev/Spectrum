#pragma once
namespace DX12
{
		template<std::ranges::view R>
		void TiledResourceManager::load_tiles2(CommandList* list, R tiles, uint subres, bool recursive)
		{
			update_tiling_info info;
			info.resource = static_cast<Resource*>(this);

			for (auto t : tiles)
				load_tile(info, t, subres, recursive);


			// TODO: make list
			if (list)
			{
				list->update_tilings(std::move(info));
			}
			else
				Device::get().get_queue(CommandListType::DIRECT)->update_tile_mappings(info);
		}

}