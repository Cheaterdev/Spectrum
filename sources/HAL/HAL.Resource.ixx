export module HAL:Resource;

import :HeapAllocators;
import :API.Device;
import :API.Resource;

import :TiledMemoryManager;
import :ResourceStates;
import :FrameManager;

import Core;

export{
	namespace HAL
	{

		
		class Resource :public SharedObject<Resource>, public ObjectState<TrackedObjectState>,  public TrackedObject, public API::Resource
		{
			friend class API::Resource;
			ResourceAddress gpu_address;
			HeapType heap_type;
			ResourceDesc desc;
		protected:
			ResourceStateManager state_manager;
			TiledResourceManager tiled_manager;
			void _init(const ResourceDesc& desc, HeapType heap_type = HeapType::DEFAULT, ResourceState state = ResourceState::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));

			void write(std::vector<std::byte>&);
			std::vector<std::byte> read();
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

			std::span<std::byte> cpu_data() const;

			// TODO:: works only for buffer now
			uint64 get_size()const
			{
				return get_desc().as_buffer().SizeInBytes;// desc.BufferDesc.desc.get<BufferDesc>
			}

			template<class T, class ...Args>
			typename T create_view(GPUEntityStorageInterface& frame, Args ...args)
			{
				return T(get_ptr(), frame, args...);
			}


				private:
					SERIALIZE_PRETTY()
					{
						ar& NVP(desc);
					}


					SERIALIZE()
					{
						ar& NVP(desc);

						std::vector<std::byte> data;
						if constexpr(Archive::is_loading::value)
						{

							 _init(desc, HeapType::DEFAULT);
								ar & NVP(data);
								write(data);
						}
						else
						{
							data = read();
						ar& NVP(data);

						}
					}
		};


	}


}




export
{

	namespace cereal
	{
		template<class Archive>
		void serialize(Archive& ar, HAL::Resource*& g)
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
