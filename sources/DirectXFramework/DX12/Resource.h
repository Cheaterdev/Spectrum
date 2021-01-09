#pragma once
namespace DX12
{


	class Resource;


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

		D3D12_GPU_VIRTUAL_ADDRESS address = 0;
		Resource* resource = nullptr;

		explicit operator bool() const
		{
			return address;
		}

		operator D3D12_GPU_VIRTUAL_ADDRESS() const
		{
			return address;
		}
	};




	class Resource;
	using Resource_ptr = std::shared_ptr<Resource>;

	class FrameResources;

	class TrackedResource
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
		ComPtr<ID3D12Resource> m_Resource;
		ResourceHandle alloc_handle;
		bool debug = false;

		static thread_local  bool allow_resource_delete;

		using ptr = std::shared_ptr<TrackedResource>;
		TrackedResource() = default;

		void set_resource(ComPtr<ID3D12Resource> resource)
		{
			m_Resource = resource;
		}


		TrackedResource(ComPtr<ID3D12Resource> resource) :m_Resource(resource)
		{
		}

		~TrackedResource();
	};

	class Trackable
	{
	public:
		TrackedResource::ptr tracked_info;
		Trackable()
		{
			tracked_info = std::make_shared<TrackedResource>();
		}
	};

	class Resource :public std::enable_shared_from_this<Resource>, public Trackable, public ResourceStateManager, public TiledResourceManager
	{
		LEAK_TEST(Resource)

			CD3DX12_RESOURCE_DESC desc;
		bool force_delete = false;
		D3D12_GPU_VIRTUAL_ADDRESS gpu_adress;
		HeapType heap_type = HeapType::CUSTOM;
		//  std::vector< unsigned int> states;
		size_t id = 0;

	protected:


		void init(const CD3DX12_RESOURCE_DESC& desc, HeapType heap_type = HeapType::DEFAULT, ResourceState state = ResourceState::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));
	public:
		ResourceAllocationInfo alloc_info;
		std::optional<FenceWaiter> load_fence;
		ResourceHandle tmp_handle;
		std::byte* buffer_data = nullptr;
		bool debug = false;
		std::string name;
		void set_name(std::string name)
		{
			this->name = name;
			tracked_info->m_Resource->SetName(convert(name).c_str());

			debug = name == "PSSM_Cameras";

			tracked_info->debug = debug;
		}
		const CD3DX12_RESOURCE_DESC& get_desc() const
		{
			return desc;
		}
		using ptr = std::shared_ptr<Resource>;

		Resource(const CD3DX12_RESOURCE_DESC& desc, HeapType heap_type, ResourceState state = ResourceState::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));
		Resource(const ComPtr<ID3D12Resource>& resouce, ResourceState state, bool own = false);

		Resource(const CD3DX12_RESOURCE_DESC& desc, ResourceHandle handle);
		Resource() = default;

		virtual ~Resource();

		HeapType get_heap_type() const
		{
			return heap_type;
		}

		ComPtr<ID3D12Resource> get_native() const
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
		auto get_size()
		{
			return desc.Width;
		}

		ptr get_ptr()
		{
			return shared_from_this();
		}

		template<class T, class F, class ...Args>
		typename T create_view(F& frame, Args ...args)
		{
			return T(shared_from_this(), frame, args...);
		}
	};


}