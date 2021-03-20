#include "pch.h"


static const wchar_t* c_raygenShaderName = L"MyRaygenShader";

UINT RTX::get_material_id(materials::universal_material* universal)
{
	std::lock_guard<std::mutex> g(m);
	auto it = materials.find(universal);

	if (it == materials.end())
	{
		materials[universal] = (UINT)materials.size();

		need_recreate = true;
		universal->on_change.register_handler(this, [this]() {

			run([this]() {
				need_recreate = true;

				});
			});

	}


	return materials[universal];
}

void RTX::CreateCommonProps(CD3DX12_STATE_OBJECT_DESC& raytracingPipeline)
{
	auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();

	globalRootSignature->SetRootSignature(global_sig->get_native().Get());

	auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
	pipelineConfig->Config(8);

	auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
	UINT payloadSize = sizeof(Table::RayPayload::CB);   // float4 color
	UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
	shaderConfig->Config(payloadSize, attributeSize);
}

void RTX::CreateSharedCollection()
{
	CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_COLLECTION };


	CreateCommonProps(raytracingPipeline);

	{
		auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)library->get_blob().data(), library->get_blob().size());
		lib->SetDXILLibrary(&libdxil);
		// Define which shader exports to surface from the library.
		// If no shader exports are defined for a DXIL library subobject, all shaders will be surfaced.
		// In this sample, this could be omitted for convenience since the sample uses all shaders in the library. 
		{
			lib->DefineExport(c_raygenShaderName);

			for (auto& type : ray_types)
			{
				if (!type.per_hit_id)
				{
					lib->DefineExport(convert(type.hit_name).c_str());
				}

				lib->DefineExport(convert(type.miss_name).c_str());
			}
		}
	}

	for (auto& type : ray_types)
	{
		if (type.per_hit_id) continue;

		auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hitGroup->SetClosestHitShaderImport(convert(type.hit_name).c_str());
		hitGroup->SetHitGroupExport(convert(type.group_name).c_str());
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);
	}

	TEST(Device::get().get_native_device()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_SharedCollection)));
}

void RTX::CreateGlobalCollection()
{
	CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_COLLECTION };

	
	auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
	localRootSignature->SetRootSignature(local_sig->get_native().Get());

	// Shader association
	auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
	rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);

	CreateCommonProps(raytracingPipeline);

	for (auto& pair : materials)
	{
		auto mat = pair.first;


		auto& blob = mat->raytracing_lib->get_blob();
		auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
		D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)blob.data(), blob.size());
		lib->SetDXILLibrary(&libdxil);
		for (auto& type : ray_types)
		{
			if (!type.per_hit_id) continue;

			lib->DefineExport(mat->wshader_name.c_str(), convert(type.hit_name).c_str());
		}

		
		for (auto& type : ray_types)
		{
			if (!type.per_hit_id) continue;

			auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
			hitGroup->SetClosestHitShaderImport(mat->wshader_name.c_str());
			hitGroup->SetHitGroupExport((mat->wshader_name + convert(type.group_name)).c_str());
			hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

			rootSignatureAssociation->AddExport((mat->wshader_name + convert(type.group_name)).c_str());

		}
	}
	

	TEST(Device::get().get_native_device()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_GlobalCollection)));
}

void RTX::CreateRaytracingPipelineStateObject()
{
	CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

	{
		auto sharedCollection = raytracingPipeline.CreateSubobject<CD3DX12_EXISTING_COLLECTION_SUBOBJECT>();
		sharedCollection->SetExistingCollection(m_SharedCollection.Get());
	}

	{
		auto sharedCollection = raytracingPipeline.CreateSubobject<CD3DX12_EXISTING_COLLECTION_SUBOBJECT>();
		sharedCollection->SetExistingCollection(m_GlobalCollection.Get());
	}


	TEST(Device::get().get_native_device()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)));
	TEST(m_dxrStateObject.As(&stateObjectProperties));


	for(auto &type:ray_types)
	{
		if(!type.per_hit_id)
			type.hit = ::identify(stateObjectProperties->GetShaderIdentifier(convert(type.group_name).c_str()));

		
		type.miss = ::identify(stateObjectProperties->GetShaderIdentifier(convert(type.miss_name).c_str()));
	}
	
	rayGenShaderIdentifier = ::identify(stateObjectProperties->GetShaderIdentifier(c_raygenShaderName));

	for (auto& pair : materials)
	{
		auto mat = pair.first;

		std::vector<shader_identifier>hit_table;

		for (auto& type : ray_types)
		{
			shader_identifier id;

			if(type.per_hit_id)
			{
				id = ::identify(stateObjectProperties->GetShaderIdentifier((mat->wshader_name + convert(type.group_name)).c_str()));;

			}else
			{
				id = *type.hit;
			}
			
			hit_table.emplace_back(id);
		}
		
		mat->set_identifier(hit_table);
	}
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
//		type.raygen_name = "ShadowRaygenShader";

		ray_types.emplace_back(type);
	}

}

void RTX::prepare(CommandList::ptr& list)
{
	process_tasks();

	if (need_recreate)
	{
		CreateGlobalCollection();
		CreateRaytracingPipelineStateObject();

		need_recreate = false;
	}

	material_hits->prepare(list);
}

void RTX::render(MeshRenderContext::ptr context, Render::TextureView& texture, Render::RaytracingAccelerationStructure::ptr scene_as, GBuffer& gbuffer)
{
	PROFILE_GPU(L"raytracing");
	auto& list = *context->list;

	auto& compute = list.get_compute();

	{
		Slots::Raytracing rtx;
		rtx.GetIndex_buffer() = universal_index_manager::get().buffer->create_view<StructuredBufferView<UINT>>(*list.frame_resources).structuredBuffer;
		rtx.GetScene() = scene_as->resource->create_view<RTXSceneView>(*list.frame_resources).srv_handle;
		rtx.set(compute);
	}

	{
		Slots::RaytracingRays rays;
		rays.GetOutput() = texture.rwTexture2D;
		gbuffer.SetTable(rays.MapGbuffer());
		float view = context->cam->angle;		
		rays.GetPixelAngle() = atan(2*tan(view/2)/texture.get_size().y);
		rays.set(compute);
	}


	auto m_rayGenShaderTable = list.place_raw(rayGenShaderIdentifier);

	std::vector<shader_identifier> miss_table;

	for (auto& type : ray_types)
	{
		miss_table.emplace_back(type.miss);
	}
	
	auto m_missShaderTable = list.place_raw(miss_table);

	compute.set_pso(m_dxrStateObject);
	compute.dispatch_rays<closesthit_identifier, shader_identifier, shader_identifier>(texture.get_size(), material_hits->buffer->get_resource_address(), material_hits->max_size(), m_missShaderTable.get_resource_address(), miss_table.size(), m_rayGenShaderTable.get_resource_address());

}
