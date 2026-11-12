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
		// Set by a load that found the old layout (see below): the owner should have the asset saved again.
		bool old_layout = false;

		// Two layouts. The first field tells them apart: a mesh count for the old one, GroupedLayout for the current.
		//   old:      count, then per mesh: size, blob (metadata, then pages).
		//   grouped:  GroupedLayout, version, count, a BlobEntry per mesh, every mesh's metadata, every mesh's pages.
		// Loading needs only the metadata. In the old layout each mesh's metadata sits behind the previous mesh's pages,
		// which is one scattered read per mesh; grouped, it is one read for the whole asset.
		static constexpr uint32_t GroupedLayout = 0xFFFFFFFFu;
		static constexpr uint32_t GroupedVersion = 1;

		struct BlobEntry
		{
			uint64_t size;       // the whole blob, as Quanta::serialize made it
			uint64_t metadata;   // its leading bytes that are stored with the metadata; the rest is with the pages
		};

		SERIALIZE()
		{
			uint32_t count = GroupedLayout;
			ar& NVP(count);

			if constexpr (Archive::is_loading::value)
			{
				UniversalContext& context = cereal::get_user_data<UniversalContext>(ar);
				std::fstream* stream = context.template get_context<std::fstream*>();

				PROFILE(L"quanta_partition_serialize");
				const std::filesystem::path& file = context.template get_context<std::filesystem::path>();
				meshes.clear();

				if (count == GroupedLayout)
				{
					uint32_t version = 0;
					ar& NVP(version);
					ar& NVP(count);
					if (version != GroupedVersion)
						throw std::runtime_error("unknown Quanta partition layout in asset file");

					std::vector<BlobEntry> entries(count);
					if (count)
						ar& NVP(cereal::binary_data(entries.data(), entries.size() * sizeof(BlobEntry)));

					uint64_t metadata_bytes = 0;
					uint64_t page_bytes = 0;
					for (const BlobEntry& entry : entries)
					{
						if (entry.metadata > entry.size)
							throw std::runtime_error("corrupt Quanta data in asset file");
						metadata_bytes += entry.metadata;
						page_bytes += entry.size - entry.metadata;
					}

					auto metadata = std::make_unique_for_overwrite<uint8_t[]>(metadata_bytes);
					{
						PROFILE(L"quanta_partition_read");
						if (metadata_bytes)
							ar& NVP(cereal::binary_data(metadata.get(), metadata_bytes));
					}

					const uint64_t pages_at = static_cast<uint64_t>(stream->tellg());
					const uint64_t metadata_at = pages_at - metadata_bytes;

					meshes.reserve(count);
					std::vector<uint8_t> whole;
					uint64_t metadata_cursor = 0;
					uint64_t page_cursor = 0;
					for (const BlobEntry& entry : entries)
					{
						const uint8_t* head = metadata.get() + metadata_cursor;
						const uint64_t tail = entry.size - entry.metadata;

						// Page offsets inside a blob count from its start. The blob's tail sits at pages_at + page_cursor,
						// so the position its start would have is that minus the head it is missing. A blob stored whole
						// (no tail) starts where its head is.
						const uint64_t blob_at = tail ? pages_at + page_cursor - entry.metadata : metadata_at + metadata_cursor;

						const uint8_t* bytes = head;
						uint64_t have = entry.metadata;
						if (Quanta::keep_page_data() && tail)
						{
							whole.resize(entry.size);
							std::memcpy(whole.data(), head, entry.metadata);
							stream->seekg(static_cast<std::streamoff>(pages_at + page_cursor));
							stream->read(reinterpret_cast<char*>(whole.data() + entry.metadata), static_cast<std::streamsize>(tail));
							if (stream->fail())
								throw std::runtime_error("corrupt Quanta data in asset file");
							bytes = whole.data();
							have = entry.size;
						}

						auto mesh = std::make_shared<Mesh>();
						{
							PROFILE(L"quanta_partition_deserialize");
							if (!Quanta::deserialize(bytes, have, *mesh, blob_at, Quanta::keep_page_data(), entry.size))
								throw std::runtime_error("corrupt Quanta data in asset file");
						}
						mesh->file = file;
						meshes.push_back(std::move(mesh));

						metadata_cursor += entry.metadata;
						page_cursor += tail;
					}

					stream->seekg(static_cast<std::streamoff>(pages_at + page_bytes));
					return;
				}

				old_layout = true;
				meshes.reserve(count);

				// Each mesh is a size field, then its blob (metadata, then the pages the load leaves in the file). One read of
				// Window bytes covers the size field and, for all but the largest meshes, the whole metadata: the stream is
				// then touched twice per mesh (that read and the seek over the pages) instead of five or six times. A mesh whose
				// metadata does not fit the window is read again at its own size. A window that runs past the end of the file
				// leaves a failed stream: the failure is cleared and every mesh is checked against the bytes it got.
				constexpr size_t Window = 16 * 1024;
				auto window = std::make_unique_for_overwrite<uint8_t[]>(Window);

				// For the metadata that outgrew the window: grown to the largest and never zero-filled.
				std::unique_ptr<uint8_t[]> buffer;
				size_t capacity = 0;
				auto reserve = [&](size_t bytes)
				{
					if (bytes > capacity)
					{
						buffer = std::make_unique_for_overwrite<uint8_t[]>(bytes);
						capacity = bytes;
					}
				};

				constexpr uint64_t Unknown = std::numeric_limits<uint64_t>::max();
				uint64_t start = static_cast<uint64_t>(stream->tellg());   // where the next mesh's size field is
				uint64_t position = start;                                  // where the stream stands, or Unknown

				for (uint32_t i = 0; i < count; i++)
				{
					stream->read(reinterpret_cast<char*>(window.get()), static_cast<std::streamsize>(Window));
					const size_t got = static_cast<size_t>(stream->gcount());
					if (stream->fail())
					{
						stream->clear();
						position = Unknown;
					}
					else
						position = start + got;

					if (got < sizeof(uint64_t))
						throw std::runtime_error("corrupt Quanta data in asset file");

					uint64_t size = 0;
					std::memcpy(&size, window.get(), sizeof(size));
					const uint64_t offset = start + sizeof(size);
					const uint8_t* body = window.get() + sizeof(size);
					const size_t have = got - sizeof(size);

					uint64_t metadata = size;
					if (!Quanta::keep_page_data() && size >= Quanta::FileHeaderBytes)
					{
						if (have < Quanta::FileHeaderBytes)
							throw std::runtime_error("corrupt Quanta data in asset file");

						// An older file needs its page bytes to build the parent table: read it whole.
						bool needs_pages = false;
						uint64_t meta = 0;
						if (!Quanta::metadata_size(body, meta, needs_pages) || meta > size)
							throw std::runtime_error("corrupt Quanta data in asset file");
						if (!needs_pages)
							metadata = meta;
					}

					const uint8_t* bytes = body;
					if (metadata > have)
					{
						reserve(metadata);
						stream->seekg(static_cast<std::streamoff>(offset));
						stream->read(reinterpret_cast<char*>(buffer.get()), static_cast<std::streamsize>(metadata));
						if (stream->fail())
							throw std::runtime_error("corrupt Quanta data in asset file");
						position = offset + metadata;
						bytes = buffer.get();
					}

					auto mesh = std::make_shared<Mesh>();
					{
						PROFILE(L"quanta_partition_deserialize");
						if (!Quanta::deserialize(bytes, metadata, *mesh, offset, Quanta::keep_page_data(), size))
							throw std::runtime_error("corrupt Quanta data in asset file");
					}

					start = offset + size;
					if (position != start)
					{
						stream->seekg(static_cast<std::streamoff>(start));
						position = start;
					}

					mesh->file = file;
					meshes.push_back(std::move(mesh));
				}
			}
			else
			{
				uint32_t version = GroupedVersion;
				ar& NVP(version);
				count = static_cast<uint32_t>(meshes.size());
				ar& NVP(count);

				std::vector<std::vector<uint8_t>> blobs;
				std::vector<BlobEntry> entries;
				blobs.reserve(meshes.size());
				entries.reserve(meshes.size());
				for (auto& mesh : meshes)
				{
					std::vector<uint8_t>& blob = blobs.emplace_back(Quanta::serialize(*mesh));

					// A blob whose header does not give a metadata size, or that needs its pages to load (an older blob
					// version), is stored whole with the metadata.
					uint64_t metadata = blob.size();
					bool needs_pages = false;
					uint64_t meta = 0;
					if (blob.size() >= Quanta::FileHeaderBytes && Quanta::metadata_size(blob.data(), meta, needs_pages) && !needs_pages && meta <= blob.size())
						metadata = meta;
					entries.push_back({ blob.size(), metadata });
				}

				if (!entries.empty())
					ar& NVP(cereal::binary_data(entries.data(), entries.size() * sizeof(BlobEntry)));
				for (size_t i = 0; i < blobs.size(); i++)
					if (entries[i].metadata)
						ar& NVP(cereal::binary_data(blobs[i].data(), entries[i].metadata));
				for (size_t i = 0; i < blobs.size(); i++)
					if (entries[i].size > entries[i].metadata)
						ar& NVP(cereal::binary_data(blobs[i].data() + entries[i].metadata, entries[i].size - entries[i].metadata));
			}
		}
	};
}
