export module HAL:VirtualBuffer;

import :Concepts;
import :Types;
import :Debug;
import :TiledMemoryManager;
import :ResourceViews;
import :CommandList;
import Core;

export
{
	constexpr bool use_virtual = !HAL::Debug::RunForPix;

	namespace HAL
	{




		template<class Type, class AllocatorType = CommonAllocator, class T = Underlying<Type>>
		class virtual_gpu_buffer : protected DataAllocator<T, AllocatorType>
		{

			using Base = DataAllocator<T, AllocatorType>;
			std::mutex m;
			//		CommonAllocator allocator;

			struct update_data
			{
				std::vector<T> data;

				update_data(size_t offset, size_t size) :offset(offset), size(size)
				{
					data.resize(size);
				}

				update_data(size_t offset, std::vector<T>& v) :offset(offset), data(v)
				{
					size = data.size();
				}

				update_data(size_t offset, std::vector<T>&& v) :offset(offset), data(std::move(v))
				{
					size = data.size();
				}

				update_data(size_t offset, std::span<const T> v) :offset(offset), data(v.begin(), v.end())
				{
					size = data.size();
				}

				size_t offset;
				size_t size;
			};
			std::list<update_data> update_list;

			update_tiling_info updates;

		public:
			using Handle = ::TypedHandle<T>;
			StructuredBufferView<Type>buffer;
			using ptr = std::shared_ptr<virtual_gpu_buffer<Type, AllocatorType>>;



			virtual	std::span<T> aquire(size_t offset, size_t size) override {
				ASSERT(false);
				return {};
			}


			virtual	void write(size_t offset, std::vector<T>& v)  override {
				std::lock_guard<std::mutex> g(m);
				update_list.emplace_back(offset, v);
			}

			// Overloads of the above that skip a copy of the caller's vector (the one above must keep its signature:
			// DataAllocator declares it). The span form copies once, from wherever it points (a stack value is fine).
			void write(size_t offset, std::span<const T> v)
			{
				std::lock_guard<std::mutex> g(m);
				update_list.emplace_back(offset, v);
			}

			void write(size_t offset, std::vector<T>&& v)
			{
				std::lock_guard<std::mutex> g(m);
				update_list.emplace_back(offset, std::move(v));
			}


			Handle allocate(size_t n)
			{
				std::lock_guard<std::mutex> g(m);
				Handle result = Base::Allocate(n);

				if constexpr (use_virtual)	buffer.resource->get_tiled_manager().map_buffer_part(updates, result.get_offset() * sizeof(T), n * sizeof(T));
				return result;
			}

			void allocate(Handle& result, size_t n)
			{
				std::lock_guard<std::mutex> g(m);

				result.Free();
				result = Base::Allocate(n);
				if constexpr (use_virtual) buffer.resource->get_tiled_manager().map_buffer_part(updates, result.get_offset() * sizeof(T), n * sizeof(T));
			}
			/*
				T* map_elements(size_t offset, size_t size = 1)
				{
					std::lock_guard<std::mutex> g(m);
					update_list.emplace_back(offset, size);
					return update_list.back().data.data();
				}
				*/

			void reserve(CommandList& list, size_t offset)
			{
				std::lock_guard<std::mutex> g(m);
				if constexpr (use_virtual)
				{
					buffer.resource->get_tiled_manager().map_buffer_part(updates, 0, offset * sizeof(T));
					updates.resource = buffer.resource.get();
					list.update_tilings(std::move(updates));
				}
			}



			void free(Handle& h)
			{
				std::lock_guard<std::mutex> g(m);
				Base::Free(h);
			}

			uint64 max_size()
			{
				return Base::get_max_usage();
			}

		/*	void debug_print(CommandList& list)
			{

				list.get_copy().read_buffer(buffer->resource.get(), 0, sizeof(T) * 16, [this](std::span<std::byte> memory)
					{
						Log::get() << "debug_print" << Log::endl;
						auto result = reinterpret_cast<const T*>(memory.data());

						for (int i = 0; i < 16; i++)
							Log::get() << result[i] << Log::endl;
					});
			}*/

			void prepare(CommandList::ptr& list)
			{
				std::lock_guard<std::mutex> g(m);

				if constexpr (use_virtual)
				{
					updates.resource = buffer.resource.get();
					list->update_tilings(std::move(updates));
				}
				if (!update_list.empty())
				{
					std::vector<CopyContext::BufferWrite> writes;
					writes.reserve(update_list.size());
					for (auto& elems : update_list)
						writes.push_back({ buffer.desc.offset + (buffer.get_data_offset() + elems.offset) * sizeof(T),
							std::as_bytes(std::span<const T>(elems.data)) });
					list->get_copy().update_buffer_batch(buffer.resource.get(), writes);
				}

				update_list.clear();
			}
			virtual_gpu_buffer(Device& device, size_t max_size, counterType countType = counterType::NONE, HAL::ResFlags flags = HAL::ResFlags::ShaderResource) :Base(max_size)
			{
				if constexpr (use_virtual)
					buffer = StructuredBufferView<Type>(device, uint(max_size), countType, flags, HeapType::RESERVED); // TODO uint size_t

				else
					buffer = StructuredBufferView<Type>(device, std::min(256_mb, max_size), countType, flags, HeapType::DEFAULT);

			/*	if constexpr (use_virtual)
				if(countType!= counterType::NONE)
				{
					buffer.resource->get_tiled_manager().map_buffer_part(updates, 0, 4);
				}*/
				// buffer = std::make_shared<HAL::StructureBuffer<T>>(max_size, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE, DefaultAllocator::get());
			}
		};



	}

}