export module Graphics:QuantaAsset;

import Core;
import :Quanta;

// The "quanta" entry of a mesh asset's file (see Asset::save_partitions): one Quanta::serialize blob per
// mesh of the asset. The blobs are plain bytes in the file, so loading records where each page sits
// (Page::file_offset), the way GPUBinaryData does for GPU buffers; a streamer can then have DirectStorage
// read a page straight from the asset file. Loading reads only the metadata in front of the pages (unless
// Quanta::set_keep_page_data(true)): the page bytes stay in the file until a page is loaded.

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

				PROFILE(L"quanta_partition_serialize");
				meshes.clear();
				for (uint32_t i = 0; i < count; i++)
				{
					uint64_t size = 0;
					ar& NVP(size);

					uint64_t offset;
					{
						PROFILE(L"quanta_partition_tellg");
						offset = stream->tellg();
					}
					auto mesh = std::make_shared<Mesh>();

					std::vector<uint8_t> bytes;
					uint64_t metadata = size;
					size_t have = 0;
					if (!Quanta::keep_page_data() && size >= Quanta::FileHeaderBytes)
					{
						PROFILE(L"quanta_partition_header");
						bytes.resize(Quanta::FileHeaderBytes);
						ar& NVP(cereal::binary_data(bytes.data(), bytes.size()));
						have = bytes.size();

						// An older file needs its page bytes to build the parent table: read it whole.
						bool needs_pages = false;
						uint64_t meta = 0;
						if (!Quanta::metadata_size(bytes.data(), meta, needs_pages) || meta > size)
							throw std::runtime_error("corrupt Quanta data in asset file");
						if (!needs_pages)
							metadata = meta;
					}

					{
						PROFILE(L"quanta_partition_read_metadata");
						bytes.resize(metadata);
						if (metadata > have)
							ar& NVP(cereal::binary_data(bytes.data() + have, metadata - have));
					}

					{
						PROFILE(L"quanta_partition_deserialize");
						if (!Quanta::deserialize(bytes.data(), bytes.size(), *mesh, offset, Quanta::keep_page_data(), size))
							throw std::runtime_error("corrupt Quanta data in asset file");
					}

					{
							PROFILE(L"quanta_partition_seek");
							stream->seekg(static_cast<std::streamoff>(offset + size));
						}

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
