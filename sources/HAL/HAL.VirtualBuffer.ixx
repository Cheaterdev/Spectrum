export module HAL:VirtualBuffer;

export import :Concepts;
import :Buffer;
import :Types;
import :Debug;

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

				size_t offset;
				size_t size;
			};
			std::list<update_data> update_list;

			update_tiling_info updates;

		public:
			using Handle = TypedHandle<T>;
			StructureBuffer<Type>::ptr buffer;
			using ptr = std::shared_ptr<virtual_gpu_buffer<Type, AllocatorType>>;



			virtual	std::span<T> aquire(size_t offset, size_t size) override {
				assert(false);
				return {};
			}


			virtual	void write(size_t offset, std::vector<T>& v)  override {
				std::lock_guard<std::mutex> g(m);
				update_list.emplace_back(offset, v);
			}


			TypedHandle<T> allocate(size_t n)
			{
				std::lock_guard<std::mutex> g(m);
				TypedHandle<T> result = Base::Allocate(n);

				if constexpr (use_virtual)	buffer->resource->get_tiled_manager().map_buffer_part(updates, result.get_offset() * sizeof(T), n * sizeof(T));
				return result;
			}

			void allocate(TypedHandle<T>& result, size_t n)
			{
				std::lock_guard<std::mutex> g(m);

				result.Free();
				result = Base::Allocate(n);
				if constexpr (use_virtual) buffer->resource->get_tiled_manager().map_buffer_part(updates, result.get_offset() * sizeof(T), n * sizeof(T));
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
				if constexpr (use_virtual)	buffer->resource->get_tiled_manager().map_buffer_part(updates, 0, offset * sizeof(T));

				updates.resource = buffer->resource.get();
				list.update_tilings(std::move(updates));
			}



			void free(TypedHandle<T>& h)
			{
				std::lock_guard<std::mutex> g(m);
				Base::Free(h);
			}

			size_t max_size()
			{
				return Base::get_max_usage();
			}

			void debug_print(CommandList& list)
			{

				list.get_copy().read_buffer(buffer->resource.get(), 0, sizeof(T) * 16, [this](const char* data, UINT64 size)
					{
						Log::get() << "debug_print" << Log::endl;
						auto result = reinterpret_cast<const T*>(data);

						for (int i = 0; i < 16; i++)
							Log::get() << result[i] << Log::endl;
					});
			}

			void prepare(CommandList::ptr& list)
			{
				std::lock_guard<std::mutex> g(m);

				updates.resource = buffer->resource.get();
				list->update_tilings(std::move(updates));
				for (auto& elems : update_list)
				{
					buffer->set_data(list, static_cast<UINT>(elems.offset * sizeof(T)), elems.data);
					//buffer->set_data(list, (UINT)elems.offset * sizeof(T), elems.data.data(), (UINT)elems.size);
				}

				update_list.clear();
			}
			virtual_gpu_buffer(size_t max_size, counterType countType = counterType::NONE, HAL::ResFlags flags = HAL::ResFlags::ShaderResource, HAL::ResourceState state = HAL::ResourceState::COMMON) :Base(max_size)
			{
				if constexpr (use_virtual)
					buffer = std::make_shared<StructureBuffer<Type>>(max_size, countType, flags, HeapType::RESERVED, state);

				else
					buffer = std::make_shared<StructureBuffer<Type>>(std::min(256_mb, max_size), countType, flags, HeapType::DEFAULT, state);

				// buffer = std::make_shared<HAL::StructureBuffer<T>>(max_size, counterType::NONE, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE, DefaultAllocator::get());
			}
		};



	}

}