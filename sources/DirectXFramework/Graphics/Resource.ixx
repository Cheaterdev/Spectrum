export module Graphics:Resource;

import :Memory;
import :Tiling;
import :Fence;
import :States;
import :Device;
export import d3d12;
import stl.memory;
import Utils;
import Trackable;

//import :Definitions;

using namespace HAL;
export{
	namespace Graphics
	{
		struct ResourceAddress
		{
			ResourceAddress()
			{
				address = 0;
				resource = nullptr;

			}

			ResourceAddress(D3D12_GPU_VIRTUAL_ADDRESS address, Resource* resource) :address(address), resource(resource)
			{

			}

			D3D12_GPU_VIRTUAL_ADDRESS address;
			Resource* resource;

			explicit operator bool() const
			{
				return address;
			}

			ResourceAddress offset(UINT v) const
			{
				return ResourceAddress(address + v, resource);
			}


			operator D3D12_GPU_VIRTUAL_ADDRESS() const
			{
				return address;
			}
		};

		export using Resource_ptr = std::shared_ptr<Resource>;

		export class TrackedResource : public TrackedObject
		{
			friend class TiledResourceManager;

			std::vector<grid<uint3, ResourceTile>> gpu_tiles;
			ResourceTile gpu_packed_tile;
		public:
			void on_tile_update(const update_tiling_info& info)
			{
				for (auto& [heap, tiles] : info.tiles)
				{
					for (auto tile : tiles)
					{
						if (tile.subresource == gpu_tiles.size())
							gpu_packed_tile = tile;
						else
							gpu_tiles[tile.subresource][tile.pos] = tile;
					}
				}
			}

		public:
			HAL::Resource::ptr m_Resource;
			ResourceHandle alloc_handle;
			bool debug = false;

			using ptr = std::shared_ptr<TrackedResource>;
			TrackedResource() = default;

			void set_resource(HAL::Resource::ptr resource)
			{
				m_Resource = resource;
			}


			TrackedResource(HAL::Resource::ptr resource) :m_Resource(resource)
			{
			}

			~TrackedResource();
		};

		struct ResourceContextState
		{
			bool used = false;
			void reset()
			{
				used = false;
			}
		};

		class Resource :public SharedObject<Resource>, public Trackable<TrackedResource>, public ResourceStateManager, public TiledResourceManager
		{
			LEAK_TEST(Resource)
				bool force_delete = false;
			D3D12_GPU_VIRTUAL_ADDRESS gpu_adress;
			HeapType heap_type;
			//  std::vector< unsigned int> states;
			size_t id = 0;

		protected:


			void init(const ResourceDesc& desc, HeapType heap_type = HeapType::DEFAULT, ResourceState state = ResourceState::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));
		public:


			ResourceAllocationInfo alloc_info;
			std::optional<FenceWaiter> load_fence;
			//ResourceHandle tmp_handle;
			std::byte* buffer_data = nullptr;
			mutable bool debug = false;
			std::string name;
			void set_name(std::string name);

			const auto& get_desc() const
			{
				return get_hal()->get_desc();
			}

			using ptr = std::shared_ptr<Resource>;

			Resource(const ResourceDesc& desc, HeapType heap_type, ResourceState state = ResourceState::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));
			Resource(const HAL::Resource::ptr& resouce, ResourceState state, bool own = false);

			Resource(const ResourceDesc& desc, ResourceHandle handle, bool own = false);
			Resource() = default;

			virtual ~Resource();

			HeapType get_heap_type() const
			{
				return heap_type;
			}

			HAL::Resource::ptr get_hal() const
			{
				return tracked_info->m_Resource;
			}

			D3D12_GPU_VIRTUAL_ADDRESS get_gpu_address() const
			{
				return gpu_adress;
			}

			ResourceAddress get_resource_address()
			{
				assert(gpu_adress > 0);
				return ResourceAddress(gpu_adress, this);
			}

			// TODO:: works only for buffer now
			uint64 get_size()
			{
				return get_desc().as_buffer().SizeInBytes;// desc.BufferDesc.desc.get<BufferDesc>
			}

			template<class T, class F, class ...Args>
			typename T create_view(F& frame, Args ...args)
			{
				return T(get_ptr(), frame, args...);
			}

			auto get_dx() const
			{
				return get_hal()->native_resource.Get();
			}

#ifdef DEV
			std::mutex m;
			std::set<CommandList*> lists;
			void used(CommandList* list)
			{
				std::lock_guard<std::mutex> g(m);

				lists.emplace(list);
			}

			void not_used(CommandList* list)
			{
				std::lock_guard<std::mutex> g(m);

				lists.erase(list);
			}
#endif


		};

		struct IndexBufferView
		{
			D3D12_INDEX_BUFFER_VIEW view;
			Resource* resource = nullptr;
		};



	}
}