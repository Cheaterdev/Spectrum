#pragma once
namespace DX12
{

	enum class HeapType : int
	{
		DEFAULT = D3D12_HEAP_TYPE_DEFAULT,
		UPLOAD = D3D12_HEAP_TYPE_UPLOAD,
		READBACK = D3D12_HEAP_TYPE_READBACK,
		CUSTOM = D3D12_HEAP_TYPE_CUSTOM
	};

	enum ResourceState
	{
		COMMON = D3D12_RESOURCE_STATE_COMMON,
		VERTEX_AND_CONSTANT_BUFFER = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		INDEX_BUFFER = D3D12_RESOURCE_STATE_INDEX_BUFFER,
		RENDER_TARGET = D3D12_RESOURCE_STATE_RENDER_TARGET,
		UNORDERED_ACCESS = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		DEPTH_WRITE = D3D12_RESOURCE_STATE_DEPTH_WRITE,
		DEPTH_READ = D3D12_RESOURCE_STATE_DEPTH_READ,
		NON_PIXEL_SHADER_RESOURCE = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		PIXEL_SHADER_RESOURCE = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		STREAM_OUT = D3D12_RESOURCE_STATE_STREAM_OUT,
		INDIRECT_ARGUMENT = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
		COPY_DEST = D3D12_RESOURCE_STATE_COPY_DEST,
		COPY_SOURCE = D3D12_RESOURCE_STATE_COPY_SOURCE,
		RESOLVE_DEST = D3D12_RESOURCE_STATE_RESOLVE_DEST,
		RESOLVE_SOURCE = D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		GEN_READ = D3D12_RESOURCE_STATE_GENERIC_READ,
		PRESENT = D3D12_RESOURCE_STATE_PRESENT,
		PREDICATION = D3D12_RESOURCE_STATE_PREDICATION,
		UNKNOWN = -1
	};

	enum class DescriptorHeapType : int
	{
		CBV_SRV_UAV = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		SAMPLER = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		RTV = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		DSV = D3D12_DESCRIPTOR_HEAP_TYPE_DSV

	};
	enum class DescriptorHeapFlags : int
	{
		NONE = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		SHADER_VISIBLE = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE

	};

	struct Handle
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE cpu;
		CD3DX12_GPU_DESCRIPTOR_HANDLE gpu;

		bool is_valid() const
		{
			return cpu.ptr != 0 || gpu.ptr != 0;
		}

		Handle()
		{
			gpu.ptr = 0;
			cpu.ptr = 0;
		}

		void operator=(const std::function<void(const Handle&)>& f)
		{
			f(*this);
		}

		bool operator!=(const Handle& r)
		{
			if (cpu.ptr != r.cpu.ptr) return true;

			if (gpu.ptr != r.gpu.ptr) return true;

			return false;
		}

		void place(const Handle& r, D3D12_DESCRIPTOR_HEAP_TYPE t = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) const;

		void clear(CommandList& list, float4 = { 0, 0, 0, 0 }) const;
	};


	class DescriptorHeap;
	class CommandList;
	struct HandleTable
	{
		Handle operator[](UINT i) const
		{
			assert(i < info->count);
			Handle res = get_base();
			res.cpu.Offset(i, info->descriptor_size);
			res.gpu.Offset(i, info->descriptor_size);
			return res;
		}

		const Handle& get_base() const
		{
			return info->base;
		}

		UINT get_count() const
		{
			return info->count;
		}

		bool valid() const
		{
			return !!info;
		}
	private:
		friend class DescriptorHeap;

		struct helper
		{
			Handle base;
			UINT descriptor_size;
			UINT count;
		};


		std::shared_ptr<helper> info;
	};

	class DescriptorHeap : public std::enable_shared_from_this<DescriptorHeap>
	{
		//      std::vector<Handle> handles;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

		UINT descriptor_size;
		UINT current_offset = 0;
		UINT max_count;
		std::mutex m;
		std::map<UINT, std::deque<UINT>> frees;
		HandleTable make_table(UINT count, UINT offset)
		{
			HandleTable res;
			std::weak_ptr<DescriptorHeap> ptr = shared_from_this();
			res.info = std::shared_ptr<HandleTable::helper>(new HandleTable::helper, [ptr, count, offset](HandleTable::helper * e)
			{
				auto t = ptr.lock();

				if (t)
					t->frees[count].push_back(offset);

				delete e;
			});
			res.info->base.cpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), offset, descriptor_size);
			res.info->base.gpu = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetGPUDescriptorHandleForHeapStart(), offset, descriptor_size);
			res.info->descriptor_size = descriptor_size;
			res.info->count = count;
			return res;
		}
		template<class T>
		void place(HandleTable& table, UINT i, T smth)
		{
			smth(table[i]);
		}

		template<class T, class ...Args>
		void place(HandleTable& table, UINT i, T smth, Args...args)
		{
			place(table, i, smth);
			place(table, i + 1, args...);
		}

	public:
		using ptr = std::shared_ptr<DescriptorHeap>;

		ComPtr<ID3D12DescriptorHeap> get_native()
		{
			return m_rtvHeap;
		}


		DescriptorHeap(UINT num, DescriptorHeapType type, DescriptorHeapFlags flags = DescriptorHeapFlags::NONE);

		void reset()
		{
			current_offset = 0;
		}

		size_t size()
		{
			return max_count;
		}
		template<class T>
		HandleTable create_table(T _count,
			typename std::enable_if<std::is_scalar<T>::value>::type* = 0)
		{

			UINT count = static_cast<UINT>(_count);
			std::lock_guard<std::mutex> g(m);

			if (frees[count].size())
			{
				auto&& list = frees[count];
				UINT offset = list.front();
				list.pop_front();
				return make_table(count, offset);
			}

			if (current_offset >= max_count)
				Log::get() << Log::LEVEL_ERROR << "DescriptorHeap limit reached!" << Log::endl;

			HandleTable res = make_table(count, current_offset);
			current_offset += count;
			return res;
		}


		template<class ...Args>
		HandleTable create_table(Args... t)
		{
			const UINT count = sizeof...(t);
			std::lock_guard<std::mutex> g(m);

			if (frees[count].size())
			{
				auto&& list = frees[count];
				size_t offset = list.front();
				list.pop_front();
				return make_table(count, offset);
			}

			if (current_offset >= max_count)
				Log::get() << Log::LEVEL_ERROR << "DescriptorHeap limit reached!" << Log::endl;

			HandleTable res = make_table(count, current_offset);
			current_offset += count;
			place(res, 0, t...);
			return res;
		}

		CD3DX12_CPU_DESCRIPTOR_HANDLE get_handle(UINT i);
		CD3DX12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(UINT i);

		HandleTable get_table_view(UINT offset, UINT count)
		{
			assert((offset + count) <= max_count && "Not enought handles");
			HandleTable res;
			res.info = std::shared_ptr<HandleTable::helper>(new HandleTable::helper);

			res.info->base.cpu = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), offset, descriptor_size);
			res.info->base.gpu = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetGPUDescriptorHandleForHeapStart(), offset, descriptor_size);
			res.info->descriptor_size = descriptor_size;
			res.info->count = count;
			return res;
		}
	};

	class DescriptorHeapManager : public Singleton<DescriptorHeapManager>
	{
		friend class Singleton<DescriptorHeapManager>;


		DescriptorHeap::ptr heap_cb_sr_ua_static;

		DescriptorHeap::ptr heap_cb_sr_ua;
		DescriptorHeap::ptr heap_samplers;
		DescriptorHeap::ptr heap_rt;
		DescriptorHeap::ptr heap_ds;

		HandleTable samplers_default_table;
		DescriptorHeapManager();
	public:

		DescriptorHeap::ptr& get_csu_static()
		{
			return heap_cb_sr_ua_static;
		}

		DescriptorHeap::ptr& get_csu()
		{
			return heap_cb_sr_ua;
		}
		DescriptorHeap::ptr& get_samplers()
		{
			return heap_samplers;
		}
		DescriptorHeap::ptr& get_rt()
		{
			return heap_rt;
		}
		DescriptorHeap::ptr& get_ds()
		{
			return heap_ds;
		}

		HandleTable get_default_samplers()
		{
			return samplers_default_table;
		}


	};

	class DynamicDescriptorManager
	{
		std::vector<DescriptorHeap::ptr> pages;

		UINT offset;
		// std::vector<bool> setted;
		// std::vector<int> offsets;


		struct row
		{
			std::vector<Handle> handles;
			bool changed;
			bool fixed;
			UINT count;
		};

		std::map<UINT, row> root_tables;
		//static std::mutex free_mutex;

		static DescriptorHeap::ptr get_free_table();
		static void free_table(DescriptorHeap::ptr);

		HandleTable null_srv;
		HandleTable null_uav;

	public:

		DynamicDescriptorManager();

		DescriptorHeap::ptr create_heap();

		void reset();

		void set(UINT i, UINT offset, Handle h);
		void set(UINT i, UINT offset, HandleTable h);
		void set(UINT i, UINT offset, std::vector<Handle>& h);
		Handle& get(UINT i, UINT offset)
		{
			return root_tables[i].handles[offset];
		}

		Handle place(const Handle &h);
		bool has_free_size(UINT count);


		UINT calculate_needed_size();

		void unbind_all();
		void bind(CommandList * list, std::function<void(UINT, Handle)> f);
		void bind(CommandList * list);
		void bind_table(UINT i, std::vector<Handle>& h);
		/*void set(int i, int offset, HandleTable h)
		{
			handles[i][offset] = h;
		}*/

		void parse(RootSignature::ptr root);



	};




	class ResourceStateManager
	{

		struct ResourceListState
		{
			unsigned int first_state= ResourceState::UNKNOWN;

			unsigned int current_state= ResourceState::UNKNOWN;

			uint64_t last_list_full_id = -1;

			bool subres_used = false;
		};

		struct SubResources
		{
			std::vector<ResourceListState> subres;
			unsigned int first_subres_state= ResourceState::UNKNOWN;

		};

		mutable  SpinLock states_lock;

		mutable std::vector<SubResources> states;

		SubResources & get_state(int id) const
		{
			if (states.size() > id)
				return states[id];

		//	std::lock_guard<boost::detail::spinlock> guard(states_lock);

		//	if (states.size() > id)
		//		return states[id];

			auto last_size = states.size();
			states.resize(id + 1);


			while (last_size <= id)
			{
				states[last_size++].subres.resize(subres_count);
			}
		

			return states[id];

		}
		


	protected:

		unsigned int gpu_state = 0;
		unsigned int subres_count = 0;
	public:

		using ptr = std::unique_ptr<ResourceStateManager>;

		ResourceStateManager()
		{
			//states.resize(64);
		}

		void init_subres(int count)
		{

			std::lock_guard<SpinLock> guard(states_lock);
			subres_count = 1 + count;
			for (auto &e : states)
				e.subres.resize(1 + count);
		}
		void assume_gpu_state(unsigned int state)
		{
			gpu_state = state;
		}



		unsigned int get_start_state(int id, uint64_t full_id)
		{
			std::lock_guard<SpinLock> guard(states_lock);

			ResourceListState & s = get_state(id).subres[0];
			return  s.first_state;
		}

		unsigned int get_end_state(int id, uint64_t full_id)
		{
			std::lock_guard<SpinLock> guard(states_lock);

			ResourceListState & s = get_state(id).subres[0];
			return  s.current_state;
		}

		unsigned int get_gpu_state()
		{
			return gpu_state;
		}

		unsigned int gpu_transition(unsigned int state)
		{
			if (gpu_state == state) return ResourceState::UNKNOWN;
			unsigned int result = gpu_state;

			gpu_state = state;
			return result;
		}


		bool is_new(int id, uint64_t full_id) const
		{
			std::lock_guard<SpinLock> guard(states_lock);

		//	if (id >= states.size())
		//		return true;

			const ResourceListState & s = get_state(id).subres[0];

			return  (s.last_list_full_id != full_id);

		}
		
/*
		unsigned int transition(unsigned int state, unsigned int subres, int id, uint64_t full_id) const
		{
			return const_cast<ResourceStateManager*>(this)->transition(state, subres, id, full_id);
		}*/

		unsigned int transition(unsigned int state, unsigned int subres, int id, uint64_t full_id) const
		{
		//	assert(id < states.size());
			std::lock_guard<SpinLock> guard(states_lock);

			auto &states = get_state(id);
			//	s.subres_used = subres != D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			ResourceListState & s_all = states.subres[0];

			//	if (subres != D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
			{
				if (s_all.last_list_full_id != full_id)
				{
					s_all.last_list_full_id = full_id;
					s_all.first_state = state;
					s_all.current_state = state;
					states.first_subres_state = ResourceState::UNKNOWN;
				}

			//	s_all.subres_used = true;
				
			}

			ResourceListState & s = states.subres[subres + 1];



			if (s.last_list_full_id != full_id)
			{
				s.last_list_full_id = full_id;
				s.first_state = state;
			

				if (subres != D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES&&	states.first_subres_state == ResourceState::UNKNOWN)
				{
					states.first_subres_state = s_all.current_state;
				

				}

				s.current_state = states.first_subres_state;
		//		return ResourceState::UNKNOWN;
			}



			if (s.current_state == state)
				return  ResourceState::UNKNOWN;

			unsigned int prev_state = s.current_state;

			s.current_state = state;
			s_all.current_state = state;

			return prev_state;
		}

	};

	class Resource :public ResourceStateManager
	{

		LEAK_TEST(Resource)

			CD3DX12_RESOURCE_DESC desc;
		bool force_delete = false;
		D3D12_GPU_VIRTUAL_ADDRESS gpu_adress;
		HeapType heap_type;
		//  std::vector< unsigned int> states;
		size_t id=0;

		/*unsigned int states;*/
	protected:
		ComPtr<ID3D12Resource> m_Resource;
		void init(const CD3DX12_RESOURCE_DESC& desc, HeapType type = HeapType::DEFAULT, ResourceState state = ResourceState::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));
	public:
		bool debug = false;
		void set_name(std::string name)
		{

			m_Resource->SetName(convert(name).c_str());
		}
		CD3DX12_RESOURCE_DESC get_desc() const
		{
			return desc;
		}
		using ptr = std::shared_ptr<Resource>;

		Resource(const CD3DX12_RESOURCE_DESC& desc, HeapType type = HeapType::DEFAULT, ResourceState state = ResourceState::COMMON, vec4 clear_value = vec4(0, 0, 0, 0));
		Resource(const ComPtr<ID3D12Resource>& resouce, bool own = false);
		Resource()
		{
			//  states.resize(50);
		}
		virtual ~Resource();

		ComPtr<ID3D12Resource> get_native() const
		{
			return m_Resource;
		}
		D3D12_GPU_VIRTUAL_ADDRESS get_gpu_address() const
		{
			return gpu_adress;
		}
		/*  void change_state(std::shared_ptr<CommandList>& command_list, ResourceState from, ResourceState to, UINT subres = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

		   void assume_state(int id, unsigned int state);
		   void assume_state( unsigned int state);

		   unsigned int get_state(int id);*/
		   // TODO:: works only for buffer now
		auto get_size()
		{
			return desc.Width;
		}
	};
}