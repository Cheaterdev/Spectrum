export module HAL:Resource;

import :HeapAllocators;
import :Device;
import :API.Resource;

import :TiledMemoryManager;
import :ResourceStates;
import :FrameManager;
import :Fence;

import Core;

template<class T>
concept can_get_context = requires(T ar)
{
	cereal::get_user_data<UniversalContext>(ar);
};

template<bool is_load>
class GPUBinaryData
{

public:

	struct save
	{
		std::vector<std::byte> binary_data;
		uint64 size;


		save(std::span<std::byte> uncompressed)
		{
			binary_data = HAL::Device::get().compress(uncompressed);
			size = binary_data.size();
		}

		SERIALIZE()
		{
		
			ar& size;
			ar& NVP(cereal::binary_data(binary_data.data(), binary_data.size()));
		}

	};

	struct load
	{
		uint64 size;
		uint64 file_offset;
		std::filesystem::path path;

		SERIALIZE()
		{
			size = 0;
			ar& size;

			UniversalContext& context = cereal::get_user_data<UniversalContext>(ar);
			file_offset = context.get_context<std::fstream*>()->tellg();
			path = context.get_context<std::filesystem::path>();

			context.get_context<std::fstream*>()->seekg(file_offset + size, std::ios::beg);
		}
	};

	struct Buffer
	{
		uint64 offset;
		uint64 size;

		SERIALIZE()
		{
			ar& NVP(offset);
			ar& NVP(size);

		}

	};

	struct Texture
	{
		uint subresource;
		uint count;

		SERIALIZE()
		{
			ar& NVP(subresource);
			ar& NVP(count);

		}


	};

	std::variant<Buffer, Texture > desc;

	using operation_type = std::conditional<is_load, load, save>::type;
	operation_type operation;

	uint64 get_size() const
	{
		return operation.size;
	}
	uint64 uncompressed_size;

	GPUBinaryData()  requires(is_load) = default;
	GPUBinaryData(Buffer desc, std::span<std::byte> binary_data) requires(!is_load)
		: desc(desc), operation(binary_data), uncompressed_size(binary_data.size_bytes())
	{
		

	}


	GPUBinaryData(Texture desc, std::span<std::byte> binary_data) requires(!is_load)
		: desc(desc), operation(binary_data), uncompressed_size(binary_data.size_bytes())
	{
			

	}
private:
	SERIALIZE()
	{
		ar& NVP(desc);
		ar& NVP(operation);
	
		ar& NVP(uncompressed_size);
	}

};

export{
	namespace HAL
	{


		class Resource :public SharedObject<Resource>, public ObjectState<TrackedObjectState>, public TrackedObject, public API::Resource
		{
			friend class API::Resource;
			ResourceAddress gpu_address;
			HeapType heap_type;
			ResourceDesc desc;
			
		protected:
			ResourceStateManager state_manager;
			TiledResourceManager tiled_manager;
			void _init(const ResourceDesc& desc, HeapType heap_type = HeapType::DEFAULT, TextureLayout initialLayout = TextureLayout::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));

			void write(std::vector<std::byte>&);
			void write(GPUBinaryData<true>&);
			std::vector<std::byte> read(uint i);
		public:
			FenceWaiter load_waiter;

			bool is_ready()const
			{
				return load_waiter.is_completed();
			}

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

			Resource(const ResourceDesc& desc, HeapType heap_type,TextureLayout initialLayout = TextureLayout::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));
			Resource(const D3D::Resource& resouce, TextureLayout initialLayout);
			Resource(const ResourceDesc& desc, PlacementAddress handle);

			Resource(const ResourceDesc& desc, ResourceHandle handle, bool own = false);
			Resource() :state_manager(this), tiled_manager(this) {};

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

				if constexpr (Archive::is_loading::value)
				{

					_init(desc, HeapType::DEFAULT);
					if (desc.is_buffer())
					{
					GPUBinaryData<true> binary;

					ar& NVP(binary);
					write(binary);
					}else
					{
							for (uint i = 0; i < desc.as_texture().Subresources(); i++)
							{
								GPUBinaryData<true> binary;

					ar& NVP(binary);
					write(binary);

							}
					}


				}
				else
				{

					if (desc.is_buffer())
					{
						auto data = read(0);
					
						GPUBinaryData<false> binary(GPUBinaryData<false>::Buffer{ 0,desc.as_buffer().SizeInBytes }, data);

						ar& NVP(binary);
					}
					else
					{

						for (uint i = 0; i < desc.as_texture().Subresources(); i++)
						{
							auto data = read(i);
			
							GPUBinaryData<false> binary(GPUBinaryData<false>::Texture{ i,1 }, data);

							ar& NVP(binary);
						}

					}


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
