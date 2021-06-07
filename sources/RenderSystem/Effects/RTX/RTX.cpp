#include "pch.h"



UINT RTX::get_material_id(materials::universal_material* universal)
{
	std::lock_guard<std::mutex> g(m);
	auto it = materials.find(universal);

	if (it == materials.end())
	{
		new_materials[universal] = (UINT)materials.size();

		need_recreate = true;
		universal->on_change.register_handler(this, [this]() {

			run([this]() {
				need_recreate = true;

				});
			});

	}

	return materials[universal];
}

void RTX::CreateCommonProps(StateObjectDesc& desc)
{
	desc.global_root = global_sig;
	desc.MaxTraceRecursionDepth = 8;
	desc.MaxAttributeSizeInBytes = 2 * sizeof(float);
	desc.MaxPayloadSizeInBytes = sizeof(Table::RayPayload::CB);
}

void RTX::CreateSharedCollection()
{
	StateObjectDesc raytracingPipeline;
	raytracingPipeline.collection = true;
	CreateCommonProps(raytracingPipeline);

	LibraryObject lib;
	lib.library = library;

	lib.export_shader(L"MyMissShader");
	lib.export_shader(L"ShadowClosestHitShader");
	lib.export_shader(L"ShadowMissShader");
	lib.export_shader(L"MyRaygenShader");
	
	raytracingPipeline.libraries.emplace_back(library);


	
	for (auto& type : ray_types)
	{
		if (type.per_hit_id) continue;
		
		HitGroup group;
		group.name = convert(type.group_name);
		group.closest_hit_shader = convert(type.hit_name);
		group.type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
		raytracingPipeline.hit_groups.emplace_back(group);
	}

	m_SharedCollection = std::make_shared<StateObject>(raytracingPipeline);
}

StateObject::ptr RTX::CreateGlobalCollection(materials::universal_material* mat)
{

		StateObjectDesc raytracingPipeline;
		raytracingPipeline.collection = true;
		CreateCommonProps(raytracingPipeline);

		LibraryObject lib;
		lib.library = mat->raytracing_lib;
		
		for (auto& type : ray_types)
		{
			if (type.per_hit_id)
			{
				lib.export_shader(mat->wshader_name,convert(type.hit_name));

				HitGroup group;

				group.local_root = local_sig;
				group.name = mat->wshader_name+convert(type.group_name);
				group.closest_hit_shader = mat->wshader_name;
				group.type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
				raytracingPipeline.hit_groups.emplace_back(group);
			}
		}
	
		raytracingPipeline.libraries.emplace_back(lib);

		mat->m_RTXCollection = std::make_shared<StateObject>(raytracingPipeline);

		get_material_id(mat);

		return mat->m_RTXCollection;

}

void RTX::CreateRaytracingPipelineStateObject()
{

	StateObjectDesc raytracingPipeline;
	raytracingPipeline.collections.emplace_back(m_SharedCollection);
	
	for (auto& pair : materials)
	{
		auto mat = pair.first;
		raytracingPipeline.collections.emplace_back(mat->m_RTXCollection);
	}
	
	m_dxrStateObject = std::make_shared<StateObject>(raytracingPipeline);


	auto change_func = [this]()
	{
		for (auto& type : ray_types)
		{
			if (!type.per_hit_id)
				type.hit = m_dxrStateObject->get_shader_id(convert(type.group_name));

			type.miss = m_dxrStateObject->get_shader_id(convert(type.miss_name));
		}

		for (auto& type : raygen_types)
		{
			type.raygen = m_dxrStateObject->get_shader_id(convert(type.name));
		}

		for (auto& pair : materials)
		{
			auto mat = pair.first;

			std::vector<shader_identifier>hit_table;

			for (auto& type : ray_types)
			{
				shader_identifier id;

				if (type.per_hit_id)
				{
					id = m_dxrStateObject->get_shader_id(mat->wshader_name + convert(type.group_name));

				}
				else
				{
					id = *type.hit;
				}

				hit_table.emplace_back(id);
			}

			mat->set_identifier(hit_table);
		}
	};
	
	//m_dxrStateObject->event_change.register_handler(this, change_func);
	change_func();
}

RTX::RTX()
{
	material_hits = std::make_shared< virtual_gpu_buffer<closesthit_identifier>>(1024 * 1024);

	library = Render::library_shader::get_resource({ "shaders\\raytracing.hlsl", "" , 0, {} });
	


	
	global_sig = get_Signature(Layouts::DefaultLayout)->create_global_signature<Slots::MaterialInfo>();
	local_sig = create_local_signature<Slots::MaterialInfo>();

	{
		RayType type;

		type.group_name = "MyHitGroup";
		type.per_hit_id = true;
		type.miss_name = "MyMissShader";
		type.hit_name = "MyClosestHitShader";
		ray_types.emplace_back(type);
	}

	{
		RayType type;
		
		type.group_name = "ShadowClosestHitGroup";
		type.per_hit_id = false;
		type.hit_name = "ShadowClosestHitShader";
		type.miss_name = "ShadowMissShader";
		ray_types.emplace_back(type);
	}

	{
		RayGenShader type;
		type.name = "MyRaygenShader";
		raygen_types.emplace_back(type);
	}

	{
		RayGenShader type;
		type.name = "MyRaygenShaderReflection";
		raygen_types.emplace_back(type);
	}

	CreateSharedCollection();
}

void RTX::prepare(CommandList::ptr& list)
{
	std::lock_guard<std::mutex> g(m);
	process_tasks();

	if (need_recreate)
	{

		materials.merge(new_materials);
		
		CreateRaytracingPipelineStateObject();

		need_recreate = false;
	}

	material_hits->prepare(list);
}

void RTX::render(ComputeContext & compute, Render::RaytracingAccelerationStructure::ptr scene_as, ivec3 size)
{
	PROFILE_GPU(L"raytracing");

	{
		Slots::Raytracing rtx;
		rtx.GetIndex_buffer() = universal_index_manager::get().buffer->create_view<StructuredBufferView<UINT>>(*compute.get_base().frame_resources).structuredBuffer;
		rtx.GetScene() = scene_as->resource->create_view<RTXSceneView>(*compute.get_base().frame_resources).scene;
		rtx.set(compute);
	}


	auto m_rayGenShaderTable = compute.get_base().place_raw(raygen_types[0].raygen);

	std::vector<shader_identifier> miss_table;

	for (auto& type : ray_types)
	{
		miss_table.emplace_back(type.miss);
	}
	
	auto m_missShaderTable = compute.get_base().place_raw(miss_table);

	compute.set_pipeline(m_dxrStateObject);
	compute.dispatch_rays<closesthit_identifier, shader_identifier, shader_identifier>(size, material_hits->buffer->get_resource_address(), material_hits->max_size(), m_missShaderTable.get_resource_address(), miss_table.size(), m_rayGenShaderTable.get_resource_address());

}


void RTX::render2(ComputeContext& compute, Render::RaytracingAccelerationStructure::ptr scene_as, ivec3 size)
{
	PROFILE_GPU(L"raytracing");

	{
		Slots::Raytracing rtx;
		rtx.GetIndex_buffer() = universal_index_manager::get().buffer->create_view<StructuredBufferView<UINT>>(*compute.get_base().frame_resources).structuredBuffer;
		rtx.GetScene() = scene_as->resource->create_view<RTXSceneView>(*compute.get_base().frame_resources).scene;
		rtx.set(compute);
	}


	auto m_rayGenShaderTable = compute.get_base().place_raw(raygen_types[1].raygen);

	std::vector<shader_identifier> miss_table;

	for (auto& type : ray_types)
	{
		miss_table.emplace_back(type.miss);
	}

	auto m_missShaderTable = compute.get_base().place_raw(miss_table);

	compute.set_pipeline(m_dxrStateObject);
	compute.dispatch_rays<closesthit_identifier, shader_identifier, shader_identifier>(size, material_hits->buffer->get_resource_address(), material_hits->max_size(), m_missShaderTable.get_resource_address(), miss_table.size(), m_rayGenShaderTable.get_resource_address());

}

