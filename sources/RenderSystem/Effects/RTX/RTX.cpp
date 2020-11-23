#include "pch.h"

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
			lib->DefineExport(c_missShaderName);

			lib->DefineExport(L"ShadowMissShader");
			lib->DefineExport(L"ShadowClosestHitShader");

		}
	}

	{
		auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
		hitGroup->SetClosestHitShaderImport(L"ShadowClosestHitShader");
		hitGroup->SetHitGroupExport(L"ShadowClosestHitGroup");
		hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

		//	rootSignatureAssociation->AddExport(L"ShadowClosestHitGroup");
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
		{
			lib->DefineExport(mat->wshader_name.c_str(), c_closestHitShaderName);

		}

		{
			auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
			hitGroup->SetClosestHitShaderImport(mat->wshader_name.c_str());
			hitGroup->SetHitGroupExport((mat->wshader_name + L"HIT").c_str());
			hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

			rootSignatureAssociation->AddExport((mat->wshader_name + L"HIT").c_str());

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

	rayGenShaderIdentifier = ::identify(stateObjectProperties->GetShaderIdentifier(c_raygenShaderName));
	missShaderIdentifier = ::identify(stateObjectProperties->GetShaderIdentifier(c_missShaderName));
	missShadowShaderIdentifier = ::identify(stateObjectProperties->GetShaderIdentifier(L"ShadowMissShader"));

	for (auto& pair : materials)
	{
		auto mat = pair.first;


		auto id = ::identify(stateObjectProperties->GetShaderIdentifier((mat->wshader_name + L"HIT").c_str()));
		auto shadow = ::identify(stateObjectProperties->GetShaderIdentifier(L"ShadowClosestHitGroup"));

		mat->set_identifier(
			id, shadow
		);
	}
}

RTX::RTX()
{
	material_hits = std::make_shared< virtual_gpu_buffer<closesthit_identifier>>(1024 * 1024);

	library = Render::library_shader::get_resource({ "shaders\\raytracing.hlsl", "" , 0, {} });

	global_sig = get_Signature(Layouts::DefaultLayout)->create_global_signature<Slots::MaterialInfo>();
	local_sig = create_local_signature<Slots::MaterialInfo>();

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
	auto timer = context->list->start(L"raytracing");

	auto& _list = context->list;
	// Get shader identifiers.
	UINT  shaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;



	auto DispatchRays = [&](auto* commandList, auto* stateObject, auto* dispatchDesc)
	{
		auto m_rayGenShaderTable = _list->place_raw(rayGenShaderIdentifier);
		auto m_missShaderTable = _list->place_raw(missShaderIdentifier, missShadowShaderIdentifier);

		// Since each shader table has only one shader record, the stride is same as the size.
		dispatchDesc->HitGroupTable.StartAddress = material_hits->buffer->get_gpu_address();
		dispatchDesc->HitGroupTable.SizeInBytes = sizeof(closesthit_identifier) * material_hits->max_size();
		dispatchDesc->HitGroupTable.StrideInBytes = sizeof(closesthit_identifier);

		dispatchDesc->MissShaderTable.StartAddress = m_missShaderTable.get_gpu_address();
		dispatchDesc->MissShaderTable.SizeInBytes = m_missShaderTable.size;
		dispatchDesc->MissShaderTable.StrideInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

		dispatchDesc->RayGenerationShaderRecord.StartAddress = m_rayGenShaderTable.get_gpu_address();
		dispatchDesc->RayGenerationShaderRecord.SizeInBytes = m_rayGenShaderTable.size;
		dispatchDesc->Width = texture.get_size().x;
		dispatchDesc->Height = texture.get_size().y;
		dispatchDesc->Depth = 1;
		commandList->SetPipelineState1(stateObject);
		commandList->DispatchRays(dispatchDesc);
	};


	{
		Slots::Raytracing rtx;
		rtx.GetIndex_buffer() = universal_index_manager::get().buffer->create_view<FormattedBufferView<UINT, DXGI_FORMAT::DXGI_FORMAT_R32_UINT>>(*_list->frame_resources).get_srv();
		rtx.GetScene() = scene_as->resource->create_view<RTXSceneView>(*_list->frame_resources).get_srv();
		rtx.set(_list->get_compute());
	}

	{
		Slots::RaytracingRays rays;
		rays.GetOutput() = texture.get_uav();
		gbuffer.SetTable(rays.MapGbuffer());
		float view = context->cam->angle;		
		rays.GetPixelAngle() = atan(2*tan(view/2)/texture.get_size().y);
		rays.set(_list->get_compute());
	}

	_list->transition(material_hits->buffer.get(), ResourceState::NON_PIXEL_SHADER_RESOURCE);
	_list->flush_transitions();


	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
	{
		DispatchRays(_list->get_native_list().Get(), m_dxrStateObject.Get(), &dispatchDesc);
	}
}
