export module HAL:Resource;
import <HAL.h>;
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
		SERIALIZE_PRETTY(){}
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
		protected:
			friend class API::Resource;
			HeapType heap_type;
			ResourceDesc desc;
			
		protected:
			ResourceStateManager state_manager;
			TiledResourceManager tiled_manager;
			void _init(const ResourceDesc& desc, HeapType heap_type = HeapType::DEFAULT, TextureLayout initialLayout = TextureLayout::UNDEFINED, vec4 clear_value = vec4(0, 0, 0, 0));

		
			void write(GPUBinaryData<true>&);
	
		public:
			FenceWaiter load_waiter;

			bool is_ready() const
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
			std::string name;
			void set_name(std::string name);


			using ptr = std::shared_ptr<Resource>;
	protected:
			Resource(const ResourceDesc& desc, HeapType heap_type, TextureLayout initialLayout = TextureLayout::UNDEFINED, vec4 clear_value = vec4(0, 0, 0, 0));
			Resource(const D3D::Resource& resouce, TextureLayout initialLayout);
			Resource(const ResourceDesc& desc, PlacementAddress handle);

			Resource(const ResourceDesc& desc, ResourceHandle handle, bool own = false);
			Resource() :state_manager(this), tiled_manager(this) {};
			public:
			virtual ~Resource();

			HeapType get_heap_type() const
			{
				return heap_type;
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

			}
		};



		class Buffer : public Resource
		{
			void init();
			std::vector<std::byte> read();
			void write(std::vector<std::byte>&);
		public:
			using ptr = std::shared_ptr<Buffer>;

			std::byte* buffer_data = nullptr;

			ResourceAddress gpu_address;
			Buffer() = default;

			Buffer(const ResourceDesc& desc, HeapType heap_type) : Resource(desc, heap_type, TextureLayout::UNDEFINED, vec4(0, 0, 0, 0)) {

				init();

			}
			Buffer(const ResourceDesc& desc, PlacementAddress handle) : Resource(desc, handle) { init(); }
			Buffer(const ResourceDesc& desc, ResourceHandle handle, bool own = false) : Resource(desc, handle, own) { init(); }
			// TODO:: works only for buffer now
			uint64 get_size() const
			{
				return get_desc().as_buffer().SizeInBytes;// desc.BufferDesc.desc.get<BufferDesc>
			}

			std::span<std::byte> cpu_data() const;


			ResourceAddress get_resource_address() const
			{
				return gpu_address;
			}

			virtual ~Buffer();
		private:
			SERIALIZE()
			{
				SAVE_PARENT(Resource);


				if constexpr (Archive::is_loading::value)
				{
					GPUBinaryData<true> binary;
					ar& NVP(binary);
					Resource::write(binary);
					init();
				}
				else
				{
					auto data = read();
					GPUBinaryData<false> binary(GPUBinaryData<false>::Buffer{ 0,desc.as_buffer().SizeInBytes }, data);
					ar& NVP(binary);
				}
			}
		};


		class TextureResource: public Resource
		{
			void init(){};
		//	std::vector<std::byte> read();
		std::vector<std::byte> read(uint i);
		public:
			using ptr = std::shared_ptr<TextureResource>;
			TextureResource() = default; // NULL texture
			TextureResource(const ResourceDesc& desc, HeapType heap_type, TextureLayout initialLayout = TextureLayout::UNDEFINED, vec4 clear_value = vec4(0, 0, 0, 0)): Resource(desc, heap_type,initialLayout,clear_value) { init(); }
		
			TextureResource(const ResourceDesc& desc, PlacementAddress handle) : Resource(desc, handle) { init(); }
			TextureResource(const ResourceDesc& desc, ResourceHandle handle, bool own = false) : Resource(desc, handle, own) { init(); }
				TextureResource(const D3D::Resource& resouce, TextureLayout initialLayout): Resource(resouce, initialLayout) { init(); }
			virtual~TextureResource() {}
		private:

			std::vector<GPUBinaryData<true>> load_subresources; // TODO: make shared temp
			SERIALIZE()
			{
				SAVE_PARENT(Resource);


			
				if constexpr (Archive::is_loading::value)
				{
					
					_init(desc, HeapType::DEFAULT, TextureLayout::COPY_QUEUE);
					

						load_subresources.resize(desc.as_texture().Subresources());
						for (uint i = 0; i < desc.as_texture().Subresources(); i++)
						{
							//GPUBinaryData<true> binary;

							ar& NVP(load_subresources[i]);
							if (!desc.is_virtual())
								Resource::write(load_subresources[i]);

						}
					


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
		};


		 Resource::ptr create_resource(const HAL::ResourceDesc&desc, HeapType heap_type);

		  Resource::ptr create_resource(const HAL::ResourceDesc&desc, ResourceHandle addr);
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
