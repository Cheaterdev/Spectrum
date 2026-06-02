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


		save(std::span<std::byte> uncompressed, HAL::Device& device)
		{
			binary_data = device.compress(uncompressed);
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
	GPUBinaryData(Buffer desc, std::span<std::byte> binary_data, HAL::Device& device) requires(!is_load)
		: desc(desc), operation(binary_data, device), uncompressed_size(binary_data.size_bytes())
	{


	}


	GPUBinaryData(Texture desc, std::span<std::byte> binary_data, HAL::Device& device) requires(!is_load)
		: desc(desc), operation(binary_data, device), uncompressed_size(binary_data.size_bytes())
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


		class Resource :public SharedObject<Resource>, public ObjectState<TrackedObjectState>, public TrackedObject, public API::Resource, public TypedObject<Resource>
		{
					protected:
						bool serialize_from_derived = false;
		protected:
			friend class API::Resource;
			HeapType heap_type;
			ResourceDesc desc;
			Device* m_device = nullptr;

		protected:
			ResourceStateManager state_manager;
			TiledResourceManager tiled_manager;
			void _init(Device& device, const ResourceDesc& desc, HeapType heap_type = HeapType::DEFAULT, TextureLayout initialLayout = TextureLayout::UNDEFINED, vec4 clear_value = vec4(0, 0, 0, 0));


			void write(GPUBinaryData<true>&);

		public:
			Device& get_device() const { ASSERT(m_device); return *m_device; }

			FenceWaiter load_waiter;
			bool debug=false;
			bool debug_transitions = false;
			bool is_ready() const;

			ResourceType get_type() const;
			const ResourceDesc& get_desc() const;
			ResourceHandle alloc_handle;
			ResourceStateManager& get_state_manager();

			TiledResourceManager& get_tiled_manager();

			std::shared_ptr<Resource> get_tracked();

			void disable_state_tracking();
			ResourceAllocationInfo alloc_info;
			std::string name;
			void set_name(std::string name);


			using ptr = std::shared_ptr<Resource>;
	protected:
			Resource(Device& device, const ResourceDesc& desc, HeapType heap_type, TextureLayout initialLayout = TextureLayout::UNDEFINED, vec4 clear_value = vec4(0, 0, 0, 0));
			Resource(Device& device, const API::NativeImportHandle& handle, TextureLayout initialLayout);
			Resource(Device& device, const ResourceDesc& desc, PlacementAddress handle);

			Resource(Device& device, const ResourceDesc& desc, ResourceHandle handle, bool own = false);
			Resource();
		public:
			virtual ~Resource();

			HeapType get_heap_type() const;




			template<class T, class ...Args>
			typename T create_view(GPUEntityStorageInterface& frame, Args ...args)
			{
				return T(get_ptr<typename T::ResourceType>(), frame, args...);
			}


		private:
			SERIALIZE_PRETTY()
			{
				ar& NVP(desc);
			}


			SERIALIZE()
			{

				desc.Flags &= ~ResFlags::DisableStateTracking;
				if (check(desc.Flags & ResFlags::DisableStateTracking))
				{
					ASSERT(false);
					Log::get() << "AlARMA!!" << Log::endl;
				}
	ASSERT(!check(desc.Flags & ResFlags::DisableStateTracking));
			
				ASSERT(serialize_from_derived);
				ar& NVP(desc);
				ASSERT(!check(desc.Flags & ResFlags::DisableStateTracking));

			}
		};
			




		Resource::ptr create_resource(Device& device, const HAL::ResourceDesc& desc, HeapType heap_type);

		// MSVC C++20 module $$_A/$$_B fix for the ResourceHandle overload.
		//
		// Problem: MSVC mangles cross-partition template arguments differently
		// inside a module ($$_B / [!HAL]) vs outside via `import HAL;` ($$_A /
		// [HAL]).  ResourceHandle = HeapHandle<HAL::Heap> has HAL::Heap from the
		// :Heap partition, so the function defined in HAL.Resource.cpp (a module
		// implementation unit) gets the $$_B mangling that FrameGraph never finds.
		//
		// Fix A: declare the real work as _create_resource_placed_impl with only
		//   primitive/non-template parameters (void*, size_t) — no cross-partition
		//   template arguments → symbol is stable from inside and outside the module.
		// Fix B: make create_resource(ResourceHandle) inline here in the *interface*
		//   unit.  Inline bodies are compiled in each caller's context, so
		//   addr.get_heap().get() uses the caller's $$_A view of HAL::Heap.
		//   The call to _create_resource_placed_impl uses only void*/size_t — no
		//   mismatch.  And Resource in the return type is from THIS same partition
		//   interface, so it's always [HAL] on both sides.
		Resource::ptr _create_resource_placed_impl(Device& device,
		                                            const ResourceDesc& desc,
		                                            void* heap_raw, size_t offset);

		inline Resource::ptr create_resource(Device& device, const HAL::ResourceDesc& desc, ResourceHandle addr)
		{
			return _create_resource_placed_impl(device, desc,
				static_cast<void*>(addr.get_heap().get()),
				addr.get_offset());
		}
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
