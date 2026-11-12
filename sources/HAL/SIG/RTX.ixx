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
		// Made once (their sizes are the pass and raygen counts), so their addresses stay valid in DispatchRays
		// records built elsewhere (raygen_address()). init_ids() fills pending_*; prepare() uploads them.
		HAL::StructuredBufferView<HAL::shader_identifier> miss_ids;
		HAL::StructuredBufferView<raygen_type> raygen_ids;
		std::vector<HAL::shader_identifier> pending_miss_ids;
		std::vector<raygen_type> pending_raygen_ids;
		bool ids_dirty = false;

		HAL::virtual_gpu_buffer<hit_type>::ptr hitgroup_ids;


		// Materials with the same hit library share one collection: the library is cached by its text and macros, so
		// they differ only in the data their local_addr points at, and the hit record carries that separately.
		struct library_group
		{
			HAL::library_shader::ptr library;	// keeps the `groups` key alive
			std::wstring name;
			// Written by the build task before it sets `built`; read only after seeing `built`.
			HAL::StateObject::ptr collection;
			std::atomic<bool> built = false;

			// Under RTXPSO::m.
			UINT use_count = 0;
			bool in_state_object = false;
			bool queued = false;	// in pending_groups
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
		// Groups no material uses any more stay (the state object cannot shrink), so a material switching back finds
		// its old library already linked.
		std::map<HAL::library_shader*, group_ptr> groups;
		// Groups not in the state object yet: building, or built and waiting for update().
		std::vector<group_ptr> pending_groups;

		// Linked groups no material uses. Nothing relinks the state object without them yet; past this many it is
		// worth doing.
		static constexpr UINT dead_group_warning = 64;
		UINT dead_groups = 0;
		bool dead_group_warned = false;

		std::atomic<bool> need_recreate = false;
		std::mutex m;

		static void init_collection(HAL::StateObjectDesc& desc, const HAL::RootSignature::ptr& global_root)
		{
			desc.global_root = global_root;
			desc.MaxTraceRecursionDepth = T::MaxTraceRecursionDepth;
			desc.MaxAttributeSizeInBytes = MaxAttributeSizeInBytes;
			desc.MaxPayloadSizeInBytes = MaxPayloadSizeInBytes;
		}

		void init_collection(HAL::StateObjectDesc& desc)
		{
			init_collection(desc, m_root_sig);
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

		// Under the lock. `to_build` is set when the group is new: start_group_build() it once the lock is released.
		group_ptr join_group(material* mat, group_ptr& to_build)
		{
			auto& group = groups[mat->raytracing_lib.operator->()];
			if (!group)
			{
				group = std::make_shared<library_group>();
				group->library = mat->raytracing_lib;
				group->name = std::wstring(L"lib_") + std::to_wstring(ids.get());
				to_build = group;
			}
			else if (group->use_count == 0 && group->in_state_object)
				dead_groups--;

			if (!group->in_state_object && !group->queued)
			{
				group->queued = true;
				pending_groups.push_back(group);
				need_recreate = true;
			}

			group->use_count++;
			return group;
		}

		// Under the lock.
		void leave_group(group_ptr& group)
		{
			if (group && --group->use_count == 0 && group->in_state_object)
			{
				dead_groups++;
				if (dead_groups > dead_group_warning && !dead_group_warned)
				{
					dead_group_warned = true;
					Log::get() << Log::LEVEL_WARNING << "RTXPSO: " << dead_groups
						<< " hit libraries in the state object are used by no material; it is not relinked without them" << Log::endl;
				}
			}
			group.reset();
		}

		// One collection with the exports and hit groups of every per-material pass. Static: the build task must not
		// reach RTXPSO, which can be gone before the task ends.
		static HAL::StateObject::ptr make_group_collection(const HAL::RootSignature::ptr& global_root,
			const HAL::RootSignature::ptr& local_root, const library_group& group)
		{
			HAL::StateObjectDesc collection;
			collection.type = StateObjectType::Collection;
			init_collection(collection, global_root);

			HAL::LibraryObject lib;
			lib.library = group.library;

			(Passes::add_library_exports(local_root, lib, collection, group.name), ...);

			if (collection.hit_groups.empty() || !lib.library)
				return nullptr;

			collection.libraries.emplace_back(lib);
			return std::make_shared<HAL::StateObject>(collection);
		}

		// The collection is a driver compile, so it runs on the asset lane while the material loads, not in update();
		// update() links it once `built` is set. The task holds the group and the signatures, never `this`.
		void start_group_build(group_ptr group)
		{
			// IDLE, like the PSO variants: a queued asset load goes first.
			thread_pool::get().enqueue_detached(TaskOptions{ TaskPriority::IDLE, TaskLane::Asset },
				[group, global_root = m_root_sig, local_root = m_local_sig]()
				{
					HAL::StateObject::ptr collection;
					{
						PROFILE(L"rtx_build_group");
						try
						{
							collection = make_group_collection(global_root, local_root, *group);
						}
						catch (const std::exception& e)
						{
							Log::get() << Log::LEVEL_ERROR << "RTXPSO: hit library collection failed: " << e.what() << Log::endl;
						}
					}
					group->collection = std::move(collection);
					group->built.store(true, std::memory_order_release);
				});
		}

		void init_material(material* mat)
		{
			// A loaded material's hit library is looked up on first access (shader_with_id): do that here, not in
			// join_group() under the lock, where every loading material would queue behind it.
			if (mat->raytracing_lib)
				(void)mat->raytracing_lib.operator->();

			group_ptr to_build;
			{
				std::lock_guard<std::mutex> g(m);

				if (materials.contains(mat) || new_materials.contains(mat))
					return;

				auto& e = new_materials[mat];
				e.handle = hitgroup_ids->allocate(sizeof...(Passes));
				e.group = join_group(mat, to_build);

				need_recreate = true;

				mat->on_change.register_handler(this, [this, mat]() {
					if (mat->raytracing_lib)
						(void)mat->raytracing_lib.operator->();

					group_ptr to_build;
					{
						std::lock_guard<std::mutex> g(m);

						const bool added = materials.contains(mat);
						auto& owner = added ? materials : new_materials;
						auto it = owner.find(mat);
						if (it == owner.end())
							return;
						material_info& info = it->second;

						// The same library is the same code (it is cached by text and macros), so the group stays.
						if (!info.group || info.group->library.operator->() != mat->raytracing_lib.operator->())
						{
							leave_group(info.group);
							info.group = join_group(mat, to_build);
						}

						// Also with the same library: a regenerate can move local_addr without update_material(). A
						// group not linked yet writes null identifiers, not the old group's with a local_addr whose
						// data no longer matches it; add_pending_materials() rewrites them when the group lands.
						if (added)
							write_material_ids(mat, info);
					}
					if (to_build)
						start_group_build(to_build);
					});
			}
			if (to_build)
				start_group_build(to_build);
		}

		// From the material's teardown. Its hit-table range stays allocated: frames in flight may still index it, and
		// virtual_gpu_buffer::free() would hand it out again at once.
		void remove_material(material* mat)
		{
			std::lock_guard<std::mutex> g(m);

			mat->on_change.unregister(this);

			for (auto* owner : { &materials, &new_materials })
				if (auto it = owner->find(mat); it != owner->end())
				{
					leave_group(it->second.group);
					owner->erase(it);
				}
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

			for (auto& [lib, group] : groups)
				if (group->in_state_object)
					raytracingPipeline.collections.emplace_back(group->collection);
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

		// Under the lock. All identifiers again: the state object was built anew (engine shader reload, or an add on a
		// backend that does not keep identifiers).
		void init_ids()
		{
			PROFILE(L"rtx_init_ids");
			pending_miss_ids.clear();
			pending_raygen_ids.clear();

			(std::get<Passes>(passes).init_ids(m_dxrStateObject, pending_miss_ids), ...);
			(std::get<Raygens>(raygen).init_ids(m_dxrStateObject, pending_raygen_ids), ...);
			ids_dirty = true;

			for (auto& [mat, info] : materials)
				write_material_ids(mat, info);
		}

		// Under the lock.
		void upload_ids(HAL::CommandList::ptr& list)
		{
			if (!ids_dirty)
				return;
			list->get_copy().update(miss_ids, 0, pending_miss_ids);
			list->get_copy().update(raygen_ids, 0, pending_raygen_ids);
			ids_dirty = false;
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

		// The groups whose build has finished go into the existing state object (HAL::StateObject::add: only their
		// collections are linked in, and on D3D12 the identifiers already handed out stay valid). Nothing here waits
		// for a build: a group still building is looked at again next update(). A material moves to `materials`
		// once its group is built.
		void add_pending_materials()
		{
			PROFILE(L"rtx_add_pending_materials");

			std::lock_guard<std::mutex> g(m);

			std::list<HAL::StateObject::ptr> collections;
			std::set<library_group*> landed;
			for (auto it = pending_groups.begin(); it != pending_groups.end();)
			{
				library_group& group = **it;
				if (!group.built.load(std::memory_order_acquire))
				{
					++it;
					continue;
				}
				// One no material wants any more is left out; join_group() queues it again if one comes back.
				if (group.use_count > 0 && group.collection)
				{
					collections.push_back(group.collection);
					landed.insert(&group);
				}
				group.queued = false;
				it = pending_groups.erase(it);
			}

			bool all_ids = false;
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
				all_ids = !m_dxrStateObject->keeps_identifiers();
			}

			std::set<material*> added;
			for (auto it = new_materials.begin(); it != new_materials.end();)
			{
				if (it->second.group && !it->second.group->built.load(std::memory_order_acquire))
				{
					++it;
					continue;
				}
				added.insert(it->first);
				materials.insert(new_materials.extract(it++));
			}

			need_recreate = !pending_groups.empty() || !new_materials.empty();

			{
				PROFILE(L"rtx_write_material_ids");
				if (all_ids)
					init_ids();
				else if (!added.empty() || !landed.empty())
					for (auto& [mat, info] : materials)
						if (added.contains(mat) || landed.contains(info.group.get()))
							write_material_ids(mat, info);
			}
		}

		// Never compiles: group collections build on the pool (start_group_build), and an engine shader reload
		// rebuilds through the state object's own event_change.
		void update()
		{
			if (need_recreate)
				add_pending_materials();
		}

		void prepare(HAL::CommandList::ptr& list)
		{
			{
				std::lock_guard<std::mutex> g(m);
				upload_ids(list);
			}
			hitgroup_ids->prepare(list);
		}

		RTXPSO(HAL::Device& device)
		{
			m_device = &device;
			m_root_sig = device.get_engine_root_layout_holder().GetSignature(T::global_sig)->template create_global_signature<typename SelectLocal<Passes>::type...>();
			m_local_sig = create_local_signature<typename SelectLocal<Passes>::type...>(device);
			hitgroup_ids = std::make_shared< HAL::virtual_gpu_buffer<hit_type>>(device, 1024 * 1024);

			miss_ids = HAL::StructuredBufferView<HAL::shader_identifier>(device, sizeof...(Passes));
			raygen_ids = HAL::StructuredBufferView<raygen_type>(device, sizeof...(Raygens));
			miss_ids.resource->set_name("RTXPSO::miss_ids");
			raygen_ids.resource->set_name("RTXPSO::raygen_ids");

			init();

			// Uploaded here once, as before, so a dispatch recorded before the first prepare() finds them; later
			// changes go through prepare().
			std::lock_guard<std::mutex> g(m);
			auto list = device.get_upload_list();
			upload_ids(list);
			list->execute_and_wait();
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
		static void add_library_exports(const HAL::RootSignature::ptr& local_root, HAL::LibraryObject& lib, HAL::StateObjectDesc& collection, const std::wstring& prefix)
		{
			if constexpr (Desc::per_material)
			{
				// Export names must be unique per (library, pass): the group's exports are all linked into MainRTX
				// next to the other groups', so the bare function names would collide. Desc::hit_name differs per
				// pass, so append it.
				std::wstring hit_export = prefix + std::wstring(Desc::hit_name);
				lib.export_shader(/*new*/ hit_export, /*was*/ std::wstring(Desc::hit_name));

				HAL::HitGroup group;

				group.local_root = local_root;
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