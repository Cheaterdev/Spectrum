module;
#include "pch_Dx.h"
export module RTX;
import Descriptors;
import Buffer;
import VirtualBuffer;

import Concepts;
import Vectors;
import Utils;
import Memory;
import Events;
import IdGenerator;
import PipelineState;
import Shader;

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
	static const UINT size = sizeof(typename type::CB); //probably table, wtf to do with resources, cb only
};



	CACHE_ALIGN(64)
		struct raygen_type
	{
		Render::shader_identifier id;
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

		Render::StateObject::ptr m_dxrStateObject;
		Render::RootSignature::ptr m_root_sig = get_Signature(T::global_sig)->create_global_signature<SelectLocal<Passes>::type...>();
		Render::RootSignature::ptr m_local_sig = create_local_signature<SelectLocal<Passes>::type...>();

		IdGenerator<Thread::Free> ids;
		
		struct material
		{
			std::wstring wshader_name;
			Events::Event<void> on_change;
			D3D12_GPU_VIRTUAL_ADDRESS local_addr;
			D3D12_GPU_VIRTUAL_ADDRESS local_addr_ids;

			Render::library_shader::ptr raytracing_lib;		
		};

		CACHE_ALIGN(32)
			struct hit_type
		{
			Render::shader_identifier id;
			D3D12_GPU_VIRTUAL_ADDRESS local_addr;
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
		Render::StructureBuffer<Render::shader_identifier>::ptr miss_ids;
		Render::StructureBuffer<raygen_type>::ptr raygen_ids;

		Render::virtual_gpu_buffer<hit_type>::ptr hitgroup_ids;


		struct material_info
		{
			UINT index;
			TypedHandle<hit_type> handle;
			std::vector<Render::StateObject::ptr> collections;
		};


		std::map< material*, material_info> materials;
		std::map<material*, material_info> new_materials;

		bool need_recreate = false;
		std::mutex m;

		void init_collection(Render::StateObjectDesc& desc)
		{
			desc.global_root = m_root_sig;
			desc.MaxTraceRecursionDepth = T::MaxTraceRecursionDepth;
			desc.MaxAttributeSizeInBytes = MaxAttributeSizeInBytes;
			desc.MaxPayloadSizeInBytes = MaxPayloadSizeInBytes;
		}

		UINT get_index(material* mat)
		{

			material_info* info = nullptr;
			//init_material(mat);
			auto it = materials.find(mat);

			if (it == materials.end())
			{
				
				info = &new_materials[mat];
			}
			else
				info = &materials[mat];

			return static_cast<UINT>(info->handle.get_offset());
		}
		void init_material(material* mat)
		{
			std::lock_guard<std::mutex> g(m);
			auto it = materials.find(mat);

			if (it == materials.end())
			{
				mat->wshader_name	=std::wstring(L"material_") + std::to_wstring(ids.get());

				auto& e = new_materials[mat];
				e.handle = hitgroup_ids->allocate(sizeof...(Passes));

				auto init_part = [&](auto& pass) {
					auto collection = pass.init_for_material(*this, mat);

					if (collection)
					{
						e.collections.push_back(collection);
					}
				};

				(init_part(std::get<Passes>(passes)), ...);


				//	new_materials[mat] = (UINT)(materials.size() + new_materials.size());

				need_recreate = true;

				mat->on_change.register_handler(this, [this]() {
					need_recreate = true;
					});
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
			Render::StateObjectDesc raytracingPipeline;
			{
				auto init_part = [&](auto& e) {
					raytracingPipeline.collections.emplace_back(e.m_Collection);
				};

				(init_part(std::get<Passes>(passes)), ...);
				(init_part(std::get<Raygens>(raygen)), ...);
			}

			for (auto& [mat, info] : materials)
			{
				for (auto& c : info.collections)
					raytracingPipeline.collections.emplace_back(c);
			}
			m_dxrStateObject = std::make_shared<Render::StateObject>(raytracingPipeline);
			m_dxrStateObject->event_change.register_handler(this, [this]() {
				std::lock_guard<std::mutex> g(m);
				init_ids();
				});

			init_ids();
		}


		void update_material(material* mat)
		{
			auto& info = materials[mat];

			auto elem = info.handle.map();
			int i = 0;
			auto init_part = [&](auto& e) {

				e.init_material_ids(m_dxrStateObject, mat, elem[i]);
				i++;;
			};

			(init_part(std::get<Passes>(passes)), ...);

			info.handle.write(0, elem);

		}

		void init_ids()
		{
			std::vector<Render::shader_identifier> miss_ids;
			std::vector<raygen_type> raygen_ids;

			(std::get<Passes>(passes).init_ids(m_dxrStateObject, miss_ids), ...);
			(std::get<Raygens>(raygen).init_ids(m_dxrStateObject, raygen_ids), ...);

			this->miss_ids = std::make_shared<Render::StructureBuffer<Render::shader_identifier>>(miss_ids.size());
			this->raygen_ids = std::make_shared<Render::StructureBuffer<raygen_type>>(raygen_ids.size());

			this->miss_ids->set_raw_data(miss_ids);
			this->raygen_ids->set_raw_data(raygen_ids);


			for (auto& [mat, info] : materials)
			{
				auto elem = info.handle.map();
				int i = 0;
				auto init_part = [&](auto& e) {

					e.init_material_ids(m_dxrStateObject, mat, elem[i]);
					//	elem[i].id = e.get_group_id(m_dxrStateObject, mat);
					i++;;
				};

				(init_part(std::get<Passes>(passes)), ...);

				info.handle.write(0, elem);
			}
		}


		void prepare(Render::CommandList::ptr& list)
		{
			if (need_recreate)
			{
				materials.merge(new_materials);
				new_materials.clear();
				need_recreate = false;

				init();
			}

			hitgroup_ids->prepare(list);
			//	hitgroup_ids->debug_print(*list);

		}

		RTXPSO()
		{
			hitgroup_ids = std::make_shared< Render::virtual_gpu_buffer<hit_type>>(1024 * 1024);
			init();
		}

		template<class T>
		void dispatch(ivec3 size, Render::ComputeContext& compute)
		{
			constexpr size_t generator = tuple_element_index<T, std::tuple<Raygens...> >();


			static_assert(static_cast<UINT>(generator) == T::ID);
			compute.set_pipeline(m_dxrStateObject);
			compute.dispatch_rays<hit_type, Render::shader_identifier, Render::shader_identifier>(size,
				hitgroup_ids->buffer->get_resource_address(), static_cast<UINT>(hitgroup_ids->max_size()),
				miss_ids->get_resource_address(), static_cast<UINT>(miss_ids->get_count()),
				raygen_ids->get_resource_address().offset(static_cast<UINT>(generator * sizeof(raygen_type))));
		}
	};

	template <class Desc>
	struct RaytraceRaygen
	{
		Render::StateObject::ptr m_Collection;
		Render::shader_identifier raygen_id;

		template<class RTX>
		void init(RTX& rtx)
		{
			Render::StateObjectDesc raytracingPipeline;
			raytracingPipeline.collection = true;

			rtx.init_collection(raytracingPipeline);

			Render::LibraryObject lib;
			lib.library = Render::library_shader::get_resource({ std::string(Desc::shader), "" , 0, {} });
			lib.export_shader(std::wstring(Desc::raygen));
			raytracingPipeline.libraries.emplace_back(lib);

			m_Collection = std::make_shared<Render::StateObject>(raytracingPipeline);
		}

		void init_ids(Render::StateObject::ptr& state, std::vector<raygen_type>& raygen_ids)
		{
			raygen_id = state->get_shader_id(std::wstring(Desc::raygen));

			raygen_type gen;

			gen.id = raygen_id;
			raygen_ids.push_back(gen);
		}

	};

	template <class Desc>
	struct RaytracePass
	{
		Render::StateObject::ptr m_Collection;

		Render::shader_identifier group_id;
		Render::shader_identifier miss_id;

		template<class RTX>
		void init(RTX& rtx)
		{
			Render::StateObjectDesc raytracingPipeline;
			raytracingPipeline.collection = true;
			rtx.init_collection(raytracingPipeline);

			Render::LibraryObject lib;
			lib.library = Render::library_shader::get_resource({ std::string(Desc::shader), "" , 0, {} });

			if constexpr (!Desc::per_material)
			{
				Render::HitGroup group;
				group.local_root = rtx.m_local_sig;
				group.name = std::wstring(Desc::name);
				group.closest_hit_shader = std::wstring(Desc::hit_name);
				group.type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
				raytracingPipeline.hit_groups.emplace_back(group);
				lib.export_shader(std::wstring(Desc::hit_name));
			}


			lib.export_shader(std::wstring(Desc::miss_name));
			raytracingPipeline.libraries.emplace_back(lib);

			m_Collection = std::make_shared<Render::StateObject>(raytracingPipeline);
		}

		template<class RTX>
		Render::StateObject::ptr init_for_material(RTX& rtx, RTX::material* mat)
		{
			if constexpr (Desc::per_material)
			{
				Render::StateObjectDesc raytracingPipeline;
				raytracingPipeline.collection = true;
				rtx.init_collection(raytracingPipeline);

				Render::LibraryObject lib;
				lib.library = mat->raytracing_lib;
				lib.export_shader(/*new*/ mat->wshader_name, /*was*/ std::wstring(Desc::hit_name));

				Render::HitGroup group;

				group.local_root = rtx.m_local_sig;
				group.name = mat->wshader_name + std::wstring(Desc::name);
				group.closest_hit_shader = mat->wshader_name;
				group.type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
				raytracingPipeline.hit_groups.emplace_back(group);

				raytracingPipeline.libraries.emplace_back(lib);

				return std::make_shared<Render::StateObject>(raytracingPipeline);

				//	get_material_id(mat);
			}
			else
				return nullptr;
		}

		template<class M, class T>
		void init_material_ids(Render::StateObject::ptr& state, M* mat, T& hit)
		{
			if constexpr (!Desc::per_material)
			{
				hit.id = state->get_shader_id(std::wstring(Desc::name));
				hit.local_addr = 0;
			}
			else
			{
				hit.id = state->get_shader_id(mat->wshader_name + std::wstring(Desc::name));
				hit.local_addr = mat->local_addr;// compiled_material_info.cb;
			}

		}

		void init_ids(Render::StateObject::ptr& state, std::vector<Render::shader_identifier>& miss_ids)
		{
			if constexpr (!Desc::per_material) group_id = state->get_shader_id(std::wstring(Desc::name));
			miss_id = state->get_shader_id(std::wstring(Desc::miss_name));

			miss_ids.push_back(miss_id);
		}
	};


}