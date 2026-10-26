export module Graphics:QuantaAsset;

import Core;
import :Quanta;

// The "quanta" entry of a mesh asset's file (see Asset::save_partitions): one Quanta::serialize blob per
// mesh of the asset. The blobs are plain bytes in the file, so loading records where each page sits
// (Page::file_offset), the way GPUBinaryData does for GPU buffers; a streamer can then have DirectStorage
// read a page straight from the asset file.

export namespace Quanta
{
	struct AssetPartition
	{
		std::vector<std::shared_ptr<Mesh>> meshes;

		SERIALIZE()
		{
			uint32_t count = static_cast<uint32_t>(meshes.size());
			ar& NVP(count);

			if constexpr (Archive::is_loading::value)
			{
				UniversalContext& context = cereal::get_user_data<UniversalContext>(ar);
				std::fstream* stream = context.template get_context<std::fstream*>();

				meshes.clear();
				for (uint32_t i = 0; i < count; i++)
				{
					uint64_t size = 0;
					ar& NVP(size);

					uint64_t offset = stream->tellg();
					std::vector<uint8_t> bytes(size);
					ar& NVP(cereal::binary_data(bytes.data(), bytes.size()));

					auto mesh = std::make_shared<Mesh>();
					if (!Quanta::deserialize(bytes.data(), bytes.size(), *mesh, offset))
						throw std::runtime_error("corrupt Quanta data in asset file");

					mesh->file = context.template get_context<std::filesystem::path>();
					meshes.push_back(std::move(mesh));
				}
			}
			else
			{
				for (auto& mesh : meshes)
				{
					std::vector<uint8_t> bytes = Quanta::serialize(*mesh);
					uint64_t size = bytes.size();
					ar& NVP(size);
					ar& NVP(cereal::binary_data(bytes.data(), bytes.size()));
				}
			}
		}
	};
}
