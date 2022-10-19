export module Graphics:Resource;

import :Memory;
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

		using update_tiling_info = HAL::update_tiling_info;
		using TiledResourceManager = HAL::TiledResourceManager;


		struct ResourceAddress
		{
			Resource* resource = nullptr;
			uint64 resource_offset = 0;

			explicit operator bool() const
			{
				return resource;
			}

			ResourceAddress offset(UINT offset) const
			{
				return { resource, resource_offset + offset };
			}
		};
	
		class Resource :public SharedObject<Resource>, public ObjectState<TrackedObjectState>,  public TrackedObject, public HAL::Resource
		{

			friend class HAL::Resource;
			LEAK_TEST(Resource)
			ResourceAddress gpu_address;
			HeapType heap_type;
			ResourceDesc desc;
		protected:
			ResourceStateManager state_manager;
			TiledResourceManager tiled_manager;
			void _init(const ResourceDesc& desc, HeapType heap_type = HeapType::DEFAULT, ResourceState state = ResourceState::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));
		public:
		
	
			const ResourceDesc& get_desc() const
			{
				return desc;
			}
	ResourceHandle alloc_handle;
			ResourceStateManager& get_state_manager()
			{
				return state_manager;
			}

			TiledResourceManager& get_tiled_manager()
			{
				return tiled_manager;
			}

			std::shared_ptr<Resource> get_tracked()
			{
				return get_ptr<Resource>();
			}

			ResourceAllocationInfo alloc_info;
		//	std::optional<FenceWaiter> load_fence;
			std::byte* buffer_data = nullptr;
			std::string name;
			void set_name(std::string name);


			using ptr = std::shared_ptr<Resource>;

			Resource(const ResourceDesc& desc, HeapType heap_type, ResourceState state = ResourceState::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));
			Resource(const D3D::Resource& resouce, ResourceState state);
			Resource(const ResourceDesc& desc, PlacementAddress handle);
		
			Resource(const ResourceDesc& desc, ResourceHandle handle,bool own=false);
			Resource():state_manager(this), tiled_manager(this) {};

			virtual ~Resource();

			HeapType get_heap_type() const
			{
				return heap_type;
			}

			ResourceAddress get_resource_address() const
			{
				return gpu_address;
			}

			// TODO:: works only for buffer now
			uint64 get_size()
			{
				return get_desc().as_buffer().SizeInBytes;// desc.BufferDesc.desc.get<BufferDesc>
			}

			template<class T, class F, class ...Args>
			typename T create_view(F& frame, Args ...args)
			{
				return T(this, frame, args...);
			}

			auto get_dx() const
			{
				return native_resource.Get();
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

				private:
					SERIALIZE_PRETTY()
					{
						ar& NVP(desc);
					}
		};
		// TODOmove to hal
		GPUAddressPtr to_native(const ResourceAddress& address)
		{
			return address.resource ? (address.resource->get_address() + address.resource_offset) : 0;
		}

	}



	Graphics::Resource* to_resource(HAL::Resource* resource)
	{
		return static_cast<Graphics::Resource*>(resource);
	}


	const Graphics::Resource* to_resource(const HAL::Resource* resource)
	{
		return static_cast<const Graphics::Resource*>(resource);
	}
}




export
{

	namespace cereal
	{
		template<class Archive>
		void serialize(Archive& ar, Graphics::Resource*& g)
		{
			if (g)
			{
				auto desc = g->get_desc();
			//	auto native_desc = g->native_resource->GetDesc();

				ar& NVP(desc);
			//	ar& NVP(native_desc);
			}
		}
	}

}
