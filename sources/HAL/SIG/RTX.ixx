export module HAL:RTX;

import :VirtualBuffer;

import :Concepts;
import Core;

export

{

template<typename T> concept HasLocalData =
requires () {
	typename T::LocalData;
};


template<class T>
struct SelectLocal
{
	using type = int;
	static const UINT size = 0;

};


template<HasLocalData T>
struct SelectLocal<T>
{
	using type = typename T::LocalData;
	static const UINT size = sizeof(type); 
};



	CACHE_ALIGN(64)
		struct raygen_type
	{
		HAL::shader_identifier id;

	private:
		SERIALIZE()
		{
			
		}
	};

	template < typename... >
	struct Typelist {};

	template< class T, typename TypeListOne, typename TypeListTwo> struct RTXPSO;

	template<class T, typename... Passes, typename... Raygens>
	struct RTXPSO<T, Typelist<Passes...>, Typelist<Raygens...>> : public virtual Events::prop_handler
	{
		using this_type = RTXPSO<T, Typelist<Passes...>, Typelist<Raygens...>>;

		std::tuple<Passes...> passes;
		std::tuple<Raygens...> raygen;

		static const UINT MaxPayloadSizeInBytes = static_cast<UINT>(Templates::max(sizeof(Underlying<typename Passes::Payload>)...));
		static const UINT MaxAttributeSizeInBytes = sizeof(float2);

		HAL::Device* m_device = nullptr;
		HAL::StateObject::ptr m_dxrStateObject;
		HAL::RootSignature::ptr m_root_sig;
		HAL::RootSignature::ptr m_local_sig;

		IdGenerator<Thread::Free> ids;
		
		struct material
		{
			std::wstring wshader_name;
			Events::Event<> on_change;
			uint local_addr;
			//D3D12_GPU_VIRTUAL_ADDRESS local_addr_ids;

			HAL::library_shader::ptr raytracing_lib;		
		};

		CACHE_ALIGN(32)
			struct hit_type
		{
			HAL::shader_identifier id;
			uint local_addr;
		//	D3D12_GPU_VIRTUAL_ADDRESS local_addr_ids;

		private:
			SERIALIZE()
			{

				ar& id;
			/*	ar& NP("data", boost::serialization::make_array(
					(std::byte*)this,
					D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES
				));*/
				
			}


			//std::array<std::byte, Templates::max(SelectLocal<Passes>::size...)> local_sig_data_could_be_anything;
		};

		//	using hit_type = shader_identifier;
		HAL::StructuredBufferView<HAL::shader_identifier> miss_ids;
		HAL::StructuredBufferView<raygen_type> raygen_ids;

		HAL::virtual_gpu_buffer<hit_type>::ptr hitgroup_ids;


		// Materials with the same hit library share one collection: the library is cached by its text and macros, so
		// they differ only in the data their local_addr points at, and the hit record carries that separately.
		struct library_group
		{
			HAL::library_shader::ptr library;	// keeps the `groups` key alive
			std::wstring name;
			HAL::StateObject::ptr collection;
			UINT use_count = 0;
			bool built = false;
			bool in_state_object = false;
		};
		using group_ptr = std::shared_ptr<library_group>;

		struct material_info
		{
			UINT index;
			TypedHandle<hit_type> handle;
			group_ptr group;
		};


		std::map< material*, material_info> materials;
		std::map<material*, material_info> new_materials;
		// Groups no material uses any more stay until the next full rebuild, so a material switching back finds its
		// old library already linked.
		std::map<HAL::library_shader*, group_ptr> groups;

		std::atomic<bool> need_recreate = false;
		// A material already in the state object moved to a hit library that is not linked yet (its shader was
		// regenerated). New materials alone are added to the existing object (add_pending_materials).
		std::atomic<bool> need_full_rebuild = false;
		std::mutex m;

		void init_collection(HAL::StateObjectDesc& desc)
		{
			desc.global_root = m_root_sig;
			desc.MaxTraceRecursionDepth = T::MaxTraceRecursionDepth;
			desc.MaxAttributeSizeInBytes = MaxAttributeSizeInBytes;
			desc.MaxPayloadSizeInBytes = MaxPayloadSizeInBytes;
		}

		UINT get_index(material* mat)
		{

			std::lock_guard<std::mutex> g(m);

			if (auto it = materials.find(mat); it != materials.end())
				return static_cast<UINT>(it->second.handle.get_offset());
			if (auto it = new_materials.find(mat); it != new_materials.end())
				return static_cast<UINT>(it->second.handle.get_offset());
			return 0;
		}

		// Under the lock.
		group_ptr join_group(material* mat)
		{
			auto& group = groups[mat->raytracing_lib.operator->()];
			if (!group)
			{
				group = std::make_shared<library_group>();
				group->library = mat->raytracing_lib;
				group->name = std::wstring(L"lib_") + std::to_wstring(ids.get());
			}
			group->use_count++;
			return group;
		}

		// Under the lock.
		void leave_group(group_ptr& group)
		{
			if (group)
				group->use_count--;
			group.reset();
		}

		// One collection with the exports and hit groups of every per-material pass. A driver compile, and only the
		// group's library and name are read, which never change, so several are made at once without the lock.
		HAL::StateObject::ptr make_group_collection(const library_group& group)
		{
			HAL::StateObjectDesc collection;
			collection.type = StateObjectType::Collection;
			init_collection(collection);

			HAL::LibraryObject lib;
			lib.library = group.library;

			(std::get<Passes>(passes).add_library_exports(*this, lib, collection, group.name), ...);

			if (collection.hit_groups.empty() || !lib.library)
				return nullptr;

			collection.libraries.emplace_back(lib);
			return std::make_shared<HAL::StateObject>(collection);
		}

		// The collections of the groups created since the last update(), all at once on the thread pool. Done here,
		// not in init_material(): that runs while a material loads, under this lock, so every material waited for the
		// previous one's compile.
		void build_pending_collections()
		{
			PROFILE(L"rtx_build_pending_collections");

			std::vector<group_ptr> pending;
			{
				std::lock_guard<std::mutex> g(m);
				for (auto& [lib, group] : groups)
					if (!group->built && group->use_count > 0)
						pending.push_back(group);
			}

			if (pending.empty())
				return;

			std::vector<HAL::StateObject::ptr> built(pending.size());
			std::vector<std::future<void>> tasks;
			tasks.reserve(pending.size());
			for (size_t i = 0; i < pending.size(); i++)
				tasks.emplace_back(thread_pool::get().enqueue(TaskOptions{ TaskPriority::HIGH }, [this, &pending, &built, i]()
				{
					{
						PROFILE(L"rtx_build_group");
						built[i] = make_group_collection(*pending[i]);
					}
				}));
			for (auto& task : tasks)
				task.wait();

			std::lock_guard<std::mutex> g(m);
			for (size_t i = 0; i < pending.size(); i++)
			{
				pending[i]->collection = std::move(built[i]);
				pending[i]->built = true;
			}
		}

		void init_material(material* mat)
		{
			std::lock_guard<std::mutex> g(m);

			if (materials.contains(mat) || new_materials.contains(mat))
				return;

			auto& e = new_materials[mat];
			e.handle = hitgroup_ids->allocate(sizeof...(Passes));
			// Its group's collection is built by update() (build_pending_collections), together with the others.
			e.group = join_group(mat);

			need_recreate = true;

			mat->on_change.register_handler(this, [this, mat]() {
				std::lock_guard<std::mutex> g(m);

				const bool added = materials.contains(mat);
				auto& owner = added ? materials : new_materials;
				auto it = owner.find(mat);
				if (it == owner.end())
					return;
				material_info& info = it->second;

				// The same library is the same code (it is cached by text and macros), so the group stays. A regenerate
				// can still move local_addr without going through update_material(), hence the rewrite below.
				if (!info.group || info.group->library.operator->() != mat->raytracing_lib.operator->())
				{
					leave_group(info.group);
					info.group = join_group(mat);
				}

				if (added && info.group->in_state_object)
				{
					write_material_ids(mat, info);
					return;
				}

				// One already in the state object whose new library is not linked yet: that takes a full rebuild.
				if (added)
					need_full_rebuild = true;

				need_recreate = true;
				});
		}

		void init()
		{

			{
				auto init_part = [&](auto& e) {
					e.init(*this);
				};

				(init_part(std::get<Passes>(passes)), ...);
				(init_part(std::get<Raygens>(raygen)), ...);
			}

			/*

					*/

			compile();
		}

		void compile()
		{
			std::lock_guard<std::mutex> g(m);
			HAL::StateObjectDesc raytracingPipeline;
			raytracingPipeline.global_root = m_root_sig;
			{
				auto init_part = [&](auto& e) {
					raytracingPipeline.collections.emplace_back(e.m_Collection);
				};

				(init_part(std::get<Passes>(passes)), ...);
				(init_part(std::get<Raygens>(raygen)), ...);
			}

			for (auto it = groups.begin(); it != groups.end();)
			{
				if (it->second->use_count == 0)
				{
					it = groups.erase(it);
					continue;
				}

				library_group& group = *it->second;
				group.in_state_object = group.built && group.collection;
				if (group.in_state_object)
					raytracingPipeline.collections.emplace_back(group.collection);
				++it;
			}
			m_dxrStateObject = std::make_shared<HAL::StateObject>(raytracingPipeline);
			m_dxrStateObject->event_change.register_handler(this, [this]() {
				std::lock_guard<std::mutex> g(m);
				init_ids();
				});

			init_ids();
		}


		void update_material(material* mat)
		{
			std::lock_guard<std::mutex> g(m);

			// One still in new_materials has nothing written yet: add_pending_materials() writes its records, with
			// the local_addr it has then.
			if (auto it = materials.find(mat); it != materials.end())
				write_material_ids(mat, it->second);
		}

		void init_ids()
		{
			PROFILE(L"rtx_init_ids");
			std::vector<HAL::shader_identifier> miss_ids;
			std::vector<raygen_type> raygen_ids;

			(std::get<Passes>(passes).init_ids(m_dxrStateObject, miss_ids), ...);
			(std::get<Raygens>(raygen).init_ids(m_dxrStateObject, raygen_ids), ...);

			this->miss_ids = HAL::StructuredBufferView<HAL::shader_identifier>(*m_device, miss_ids.size());
			this->raygen_ids = HAL::StructuredBufferView<raygen_type>(*m_device, raygen_ids.size());

			   this->miss_ids.resource->set_name("RTXPSO::miss_ids");
			  	   this->raygen_ids.resource->set_name("RTXPSO::raygen_ids");


			auto list = (m_device->get_upload_list());
			list->get_copy().update(this->miss_ids, 0, miss_ids);
			list->get_copy().update(this->raygen_ids, 0, raygen_ids);
			list->execute_and_wait();


			for (auto& [mat, info] : materials)
				write_material_ids(mat, info);
		}

		// Under the lock.
		void write_material_ids(material* mat, material_info& info)
		{
			const std::wstring* group_name = (info.group && info.group->in_state_object) ? &info.group->name : nullptr;

			auto elem = info.handle.map();
			int i = 0;
			auto init_part = [&](auto& e) {

				e.init_material_ids(m_dxrStateObject, group_name, mat->local_addr, elem[i]);
				i++;
			};

			(init_part(std::get<Passes>(passes)), ...);

			info.handle.write(0, elem);
		}

		// The groups built since the last update() go into the existing state object (HAL::StateObject::add: only
		// their collections are compiled, and the identifiers already handed out stay valid), instead of building
		// the passes' collections and every group's again. A material moves to `materials` once its group is built;
		// one whose group is created while this update builds stays pending for the next.
		void add_pending_materials()
		{
			PROFILE(L"rtx_add_pending_materials");

			std::lock_guard<std::mutex> g(m);

			std::list<HAL::StateObject::ptr> collections;
			std::set<library_group*> landed;
			bool unbuilt = false;
			for (auto& [lib, group] : groups)
			{
				if (group->use_count == 0)
					continue;
				if (!group->built)
					unbuilt = true;
				else if (group->collection && !group->in_state_object)
				{
					collections.push_back(group->collection);
					landed.insert(group.get());
				}
			}

			if (!collections.empty())
			{
				PROFILE(L"rtx_add_to_state_object");
				m_dxrStateObject = HAL::StateObject::add(m_dxrStateObject, collections);
				m_dxrStateObject->event_change.register_handler(this, [this]() {
					std::lock_guard<std::mutex> g(m);
					init_ids();
					});
				for (library_group* group : landed)
					group->in_state_object = true;
			}

			std::set<material*> added;
			for (auto it = new_materials.begin(); it != new_materials.end();)
			{
				if (it->second.group && !it->second.group->built)
				{
					++it;
					continue;
				}
				added.insert(it->first);
				materials.insert(new_materials.extract(it++));
			}

			need_recreate = unbuilt || !new_materials.empty();

			{
				PROFILE(L"rtx_write_material_ids");
				for (auto& [mat, info] : materials)
					if (added.contains(mat) || landed.contains(info.group.get()))
						write_material_ids(mat, info);
			}
		}

		 void update()
		{
			if (need_recreate)
			{
				build_pending_collections();

				if (need_full_rebuild.exchange(false))
				{
					PROFILE(L"rtx_full_rebuild");
					init();
				}

				// After a full rebuild this only moves the pending materials: compile() linked every built group.
				add_pending_materials();
			}

		}

		void prepare(HAL::CommandList::ptr& list)
		{
			hitgroup_ids->prepare(list);
		}

		RTXPSO(HAL::Device& device)
		{
			m_device = &device;
			m_root_sig = device.get_engine_root_layout_holder().GetSignature(T::global_sig)->template create_global_signature<typename SelectLocal<Passes>::type...>();
			m_local_sig = create_local_signature<typename SelectLocal<Passes>::type...>(device);
			hitgroup_ids = std::make_shared< HAL::virtual_gpu_buffer<hit_type>>(device, 1024 * 1024);
			init();
		}

		template<class Rg>   // not T: that would shadow the struct's own parameter
		void dispatch(ivec3 size, HAL::ComputeContext& compute)
		{
			constexpr size_t generator = tuple_element_index<Rg, std::tuple<Raygens...> >();


			static_assert(static_cast<UINT>(generator) == Rg::ID);
			compute.set_pipeline(m_dxrStateObject);
			compute.dispatch_rays<hit_type, HAL::shader_identifier, HAL::shader_identifier>(size,
				hitgroup_ids->buffer.get_resource_address(), static_cast<UINT>(hitgroup_ids->max_size()),
				miss_ids.get_resource_address(), static_cast<UINT>(miss_ids.get_count()),
				raygen_ids.get_resource_address(generator),
				&raygen_slots<Rg>());
		}

		template<class Rg>
		const UsedSlots& raygen_slots() const
		{
			return std::get<Rg>(raygen).slots;
		}

		// Same generator-index lookup dispatch<T>() uses, exposed on its own
		// for callers building a DispatchRaysArguments record (raytracing.prism)
		// for GPU-driven ExecuteIndirect instead of a direct dispatch_rays call
		// (DDGIProbeDispatchArgsBuild, ddgi.prism, is the first user).
		template<class Rg>
		HAL::ResourceAddress raygen_address() const
		{
			constexpr size_t generator = tuple_element_index<Rg, std::tuple<Raygens...> >();
			static_assert(static_cast<UINT>(generator) == Rg::ID);
			return raygen_ids.get_resource_address(generator);
		}
	};

	template <class Desc>
	struct RaytraceRaygen
	{
		HAL::StateObject::ptr m_Collection;
		HAL::shader_identifier raygen_id;

		// This raygen's own reflected slots -- the null-slot check scope for
		// its dispatches (see commit_tables' check_scope). Hit/miss slots are
		// deliberately excluded: which of those a dispatch reaches depends on
		// its payload (e.g. RayPayload::use_vsm_shadow), which reflection
		// cannot see.
		UsedSlots slots;
		HAL::library_shader::ptr library;

		template<class RTX>
		void init(RTX& rtx)
		{
			HAL::StateObjectDesc raytracingPipeline;
			raytracingPipeline.type = StateObjectType::Collection;

			rtx.init_collection(raytracingPipeline);

			HAL::LibraryObject lib;
			lib.library = HAL::library_shader::get_resource({ std::string(Desc::shader), "" , HAL::ShaderOptions::None, {} });
			lib.export_shader(std::wstring(Desc::raygen));
			raytracingPipeline.libraries.emplace_back(lib);

			library = lib.library;
			update_slots();

			m_Collection = std::make_shared<HAL::StateObject>(raytracingPipeline);
		}

		void update_slots()
		{
			slots.clear();

			for (auto& f : library->blob.functions)
				if (is_function(f.wname, Desc::raygen))
				{
					slots.merge(f.slots);
					return;
				}

			// Not found by name: fall back to the whole library, which is
			// what the check used before scoping existed.
			slots.merge(library->slots_usage);
		}

		// DXIL library reflection reports functions by their MSVC-mangled
		// name ("\x01?MyRaygenShader@@YAXXZ"), not the plain export name.
		static bool is_function(std::wstring_view reflected, std::wstring_view name)
		{
			if (reflected == name)
				return true;

			if (!reflected.empty() && reflected.front() == L'\x01')
				reflected.remove_prefix(1);
			if (reflected.empty() || reflected.front() != L'?')
				return false;
			reflected.remove_prefix(1);

			return reflected.starts_with(name) && reflected.substr(name.size()).starts_with(L"@@");
		}

		void init_ids(HAL::StateObject::ptr& state, std::vector<raygen_type>& raygen_ids)
		{
			// Also runs after a hot reload rebuilt the state object, so
			// refresh the scope from the reloaded library here too.
			update_slots();

			raygen_id = state->get_shader_id(std::wstring(Desc::raygen));

			raygen_type gen;

			gen.id = raygen_id;
			raygen_ids.push_back(gen);
		}

	};

	template <class Desc>
	struct RaytracePass
	{
		HAL::StateObject::ptr m_Collection;

		HAL::shader_identifier group_id;
		HAL::shader_identifier miss_id;

		template<class RTX>
		void init(RTX& rtx)
		{
			HAL::StateObjectDesc raytracingPipeline;
			raytracingPipeline.type = StateObjectType::Collection;
			rtx.init_collection(raytracingPipeline);

			HAL::LibraryObject lib;
			lib.library = HAL::library_shader::get_resource({ std::string(Desc::shader), "" , HAL::ShaderOptions::None, {} });

			if constexpr (!Desc::per_material)
			{
				HAL::HitGroup group;
				group.local_root = rtx.m_local_sig;
				group.name = std::wstring(Desc::name);
				group.closest_hit_shader = std::wstring(Desc::hit_name);
				group.type = HAL::HitGroupType::TRIANGLES;
				if (!Desc::any_hit_name.empty())
				{
					group.any_hit_shader = std::wstring(Desc::any_hit_name);
					lib.export_shader(std::wstring(Desc::any_hit_name));
				}
				raytracingPipeline.hit_groups.emplace_back(group);
				lib.export_shader(std::wstring(Desc::hit_name));
			}


			lib.export_shader(std::wstring(Desc::miss_name));
			raytracingPipeline.libraries.emplace_back(lib);

			m_Collection = std::make_shared<HAL::StateObject>(raytracingPipeline);
		}

		// This pass's exports and hit group, into the one collection of a hit library shared by all per-material
		// passes (RTXPSO::make_group_collection).
		template<class RTX>
		void add_library_exports(RTX& rtx, HAL::LibraryObject& lib, HAL::StateObjectDesc& collection, const std::wstring& prefix)
		{
			if constexpr (Desc::per_material)
			{
				// Export names must be unique per (library, pass): the group's exports are all linked into MainRTX
				// next to the other groups', so the bare function names would collide. Desc::hit_name differs per
				// pass, so append it.
				std::wstring hit_export = prefix + std::wstring(Desc::hit_name);
				lib.export_shader(/*new*/ hit_export, /*was*/ std::wstring(Desc::hit_name));

				HAL::HitGroup group;

				group.local_root = rtx.m_local_sig;
				group.name = prefix + std::wstring(Desc::name);
				group.closest_hit_shader = hit_export;
				group.type = HAL::HitGroupType::TRIANGLES;

				// Same per-(library,pass) rename as the closest-hit export above --
				// empty when this pass doesn't declare an any_hit stage at all (see
				// Desc::any_hit_name's own comment).
				if (!Desc::any_hit_name.empty())
				{
					std::wstring any_hit_export = prefix + std::wstring(Desc::any_hit_name);
					lib.export_shader(/*new*/ any_hit_export, /*was*/ std::wstring(Desc::any_hit_name));
					group.any_hit_shader = any_hit_export;
				}

				collection.hit_groups.emplace_back(group);
			}
		}

		// group_name is null while the material's hit library is not in `state`: a null identifier runs no shader,
		// where looking up a name the object does not have fails (D3D12 returns no identifier at all).
		template<class T>
		void init_material_ids(HAL::StateObject::ptr& state, const std::wstring* group_name, uint local_addr, T& hit)
		{
			if constexpr (!Desc::per_material)
			{
				hit.id = state->get_shader_id(std::wstring(Desc::name));
				hit.local_addr = 0;
			}
			else
			{
				hit.id = group_name ? state->get_shader_id(*group_name + std::wstring(Desc::name)) : HAL::shader_identifier{};
				hit.local_addr = local_addr;
			}

		}

		void init_ids(HAL::StateObject::ptr& state, std::vector<HAL::shader_identifier>& miss_ids)
		{
			if constexpr (!Desc::per_material) group_id = state->get_shader_id(std::wstring(Desc::name));
			miss_id = state->get_shader_id(std::wstring(Desc::miss_name));

			miss_ids.push_back(miss_id);
		}
	};


}