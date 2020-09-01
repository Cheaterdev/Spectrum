#pragma once


#ifndef APPVEYOR 
#ifndef DEBUG
//#define OCULUS_SUPPORT
#endif
#endif



#include "../DirectXFramework/pch.h"
#include "../FlowGraph/pch.h"



#include "autogen/includes.h"


#include "Context/Context.h"
#include "Helpers/MipMapGeneration.h"

#include "Mesh/VertexTransform/VertexTransform.h"
#include "Occlusion/Occluder.h"
#include "Scene/SceneObject.h"
#include "Camera/Camera.h"

#include "Font/TextSystem.h"
#include "GUI/GUI.h"
#include "GUI/Skin.h"

#include "GUI/Renderer/Base.h"
#include "GUI/Elements/ColoredRect.h"
#include "GUI/Elements/Image.h"
#include "GUI/Elements/Button.h"
#include "GUI/Elements/Label.h"
#include "GUI/Elements/EditText.h"
#include "GUI/Elements/Dragger.h"
#include "GUI/Elements/Resizer.h"
#include "GUI/Elements/Resizable.h"
#include "GUI/Elements/Window.h"
#include "GUI/Elements/CheckBox.h"
#include "GUI/Elements/HorizontalLayout.h"
#include "GUI/Elements/CheckBoxText.h"
#include "GUI/Elements/OptionBox.h"
#include "GUI/Elements/ScrollBar.h"
#include "GUI/Elements/ScrollContainer.h"
#include "GUI/Elements/ListBox.h"
#include "GUI/Elements/MenuList.h"
#include "GUI/Elements/ComboBox.h"
#include "GUI/Elements/TabControl.h"
#include "GUI/Elements/StatusBar.h"
#include "GUI/Elements/DockBase.h"
#include "GUI/Elements/Container.h"
#include "GUI/Elements/Tree.h"
#include "GUI/Elements/ValueBox.h"
#include "GUI/Elements/CircleSelector.h"
#include "GUI/Elements/FlowGraph/ParameterWindow.h"
#include "GUI/Elements/FlowGraph/ComponentWindow.h"
#include "GUI/Elements/FlowGraph/Comment.h"
#include "GUI/Elements/FlowGraph/Canvas.h"
#include "GUI/Elements/FlowGraph/FlowManager.h"

#include "GUI/Debugging/OutputWindow.h"
#include "GUI/Debugging/TaskViewer.h"
#include "GUI/Debugging/TimerWatcher.h"
#include "GUI/Debugging/TimerGraph.h"


#include "Assets/Asset.h"

#include "Assets/TextureAsset.h"
#include "Assets/MaterialAsset.h"
#include "Assets/BinaryAsset.h"
#include "Assets/TiledTexture.h"

#include "Mesh/vertexes.h"
#include "Mesh/Mesh.h"
#include "Assets/MeshAsset.h"


using shader_identifier = std::array<std::byte, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES>;
static shader_identifier identify(void* data)
{
	shader_identifier result;

	memcpy(result.data(), data, result.size());

	return result;
}
#define CACHE_ALIGN(x) __declspec(align(x))

CACHE_ALIGN(64)
struct closesthit_identifier
{
	shader_identifier identifier;
	D3D12_GPU_VIRTUAL_ADDRESS mat_buffer;
};


#include "Materials/Values.h"
#include "Materials/universal_material.h"
#include "Materials/ShaderMaterial.h" 


#include "Scene/Scene.h"
#include "Renderer/Renderer.h"

#include "Assets/AssetRenderer.h"

#include "GUI/Renderer/Renderer.h"

#include "GUI/Elements/AssetExplorer.h"
#include "Lighting/PSSM.h"
/*
#include "Lighting/GBuffer.h"

#include "Lighting/Light.h"
*/


#include "Renderer/StencilRenderer.h"

#include "TiledTextures/VisibilityBuffer.h"
#include "TiledTextures/GPUTilesBuffer.h"
#include "TiledTextures/Texture3DTiled.h"
#include "TiledTextures/DynamicTileGenerator.h"


#include "Effects/Sky.h"

/*
#include "Effects/TemporalAA.h"

#include "Effects/PostProcess/Adaptation.h"
#include "Effects/PostProcess/SSR.h"
#include "Effects/PostProcess/SSGI.h"
#include "Effects/PostProcess/SMAA.h"
*/
#include "Effects/PostProcess/SMAA.h"
#include "Effects/VoxelGI/VoxelGI.h"




#include "Lighting/BRDF.h"
namespace EngineAssets
{
	extern EngineAsset<MeshAsset> material_tester;
	extern EngineAsset<MeshAsset> axis;
	extern EngineAsset<MeshAsset> plane;
	extern EngineAsset<BinaryAsset> material_header;
	extern EngineAsset<BinaryAsset> material_raytracing_header;


	extern EngineAsset<TextureAsset> best_fit_normals;
	extern EngineAsset<TextureAsset> missing_texture;

	extern EngineAsset<BRDF> brdf;

}


static const wchar_t* c_hitGroupName = L"MyHitGroup";
static const wchar_t* c_raygenShaderName = L"MyRaygenShader";
static const wchar_t* c_closestHitShaderName = L"MyClosestHitShader";
static const wchar_t* c_missShaderName = L"MyMissShader";


struct RayGenConstantBuffer
{
	UINT texture_offset;
	UINT node_offset;
};


class RTX :public Singleton<RTX>,Events::prop_handler,
	public Events::Runner
{
public:
	Resource::ptr m_missShaderTable;
	Resource::ptr m_hitGroupShaderTable;
	Resource::ptr m_rayGenShaderTable;

	shader_identifier rayGenShaderIdentifier;
	shader_identifier missShaderIdentifier;
	shader_identifier missShadowShaderIdentifier;

	
	ComPtr<ID3D12StateObject> m_dxrStateObject;
	ComPtr<ID3D12StateObject> m_SharedCollection;
	ComPtr<ID3D12StateObject> m_GlobalCollection;

	std::string blob;

	RayGenConstantBuffer m_rayGenCB;
	ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
	RootSignature::ptr global_sig;

	using ptr = std::shared_ptr<RTX>;

	//ArraysHolder<InstanceData> instanceData;

	std::set<ComPtr<ID3D12StateObject>> all_objs;

	virtual_gpu_buffer<closesthit_identifier>::ptr material_hits;// (MAX_COMMANDS_SIZE)

	std::map< materials::universal_material*, int> materials;

	bool need_recreate = false;
	
	std::mutex m;
	UINT get_material_id(materials::universal_material* universal)
	{
		std::lock_guard<std::mutex> g(m);
		auto it = materials.find(universal);

		if (it == materials.end())
		{
			materials[universal] = materials.size();

			need_recreate = true;
			universal->on_change.register_handler(this, [this]() {

				run([this]() {
					need_recreate = true;
				
					});
				});

		}


		return materials[universal];
	}

	void CreateCommonProps(CD3DX12_STATE_OBJECT_DESC& raytracingPipeline)
	{

		auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();

		 global_sig = get_Signature(Layouts::DefaultLayout)->create_global_signature<Slots::MaterialInfo>();



		globalRootSignature->SetRootSignature(global_sig->get_native().Get());

		auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
		pipelineConfig->Config(8);

		auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
		UINT payloadSize = 6 * sizeof(float);   // float4 color
		UINT attributeSize = 2 * sizeof(float); // float2 barycentrics
		shaderConfig->Config(payloadSize, attributeSize);


	}

	void CreateSharedCollection()
	{
		CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_COLLECTION };


		CreateCommonProps(raytracingPipeline);

		{
			auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)blob.data(), blob.size());
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

		TEST(Device::get().get_native_device()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_SharedCollection)), L"Couldn't create DirectX Raytracing state object.\n");
	}

	void CreateGlobalCollection()
	{
		CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ D3D12_STATE_OBJECT_TYPE_COLLECTION};

		auto local_sig = create_local_signature<Slots::MaterialInfo>();

	auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
		localRootSignature->SetRootSignature(local_sig->get_native().Get());

		// Shader association
		auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
		rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);

		CreateCommonProps(raytracingPipeline);

		for (auto& pair : materials)
		{
			auto mat = pair.first;


			auto& blob = mat->raytracing_blob;
			auto lib = raytracingPipeline.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();
			D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)blob.data(), blob.size());
			lib->SetDXILLibrary(&libdxil);
			{
				lib->DefineExport(mat->wshader_name.c_str(),c_closestHitShaderName);

			}

			{
				auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();
				hitGroup->SetClosestHitShaderImport(mat->wshader_name.c_str());
				hitGroup->SetHitGroupExport((mat->wshader_name + L"HIT").c_str());
				hitGroup->SetHitGroupType(D3D12_HIT_GROUP_TYPE_TRIANGLES);

				rootSignatureAssociation->AddExport((mat->wshader_name + L"HIT").c_str());

			}

		
		}

		TEST(Device::get().get_native_device()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_GlobalCollection)), L"Couldn't create DirectX Raytracing state object.\n");
		all_objs.insert(m_GlobalCollection);

	


	}


	void CreateRaytracingPipelineStateObject()
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


		TEST(Device::get().get_native_device()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&m_dxrStateObject)), L"Couldn't create DirectX Raytracing state object.\n");
		all_objs.insert(m_dxrStateObject);


		TEST(m_dxrStateObject.As(&stateObjectProperties));

		//	shader_identifier.assign()
		rayGenShaderIdentifier = ::identify(stateObjectProperties->GetShaderIdentifier(c_raygenShaderName));
		missShaderIdentifier = ::identify(stateObjectProperties->GetShaderIdentifier(c_missShaderName));
		missShadowShaderIdentifier = ::identify(stateObjectProperties->GetShaderIdentifier(L"ShadowMissShader"));

		for (auto& pair : materials)
		{
			auto mat = pair.first;


			auto id = ::identify(stateObjectProperties->GetShaderIdentifier((mat->wshader_name + L"HIT").c_str()));
			auto shadow = ::identify(stateObjectProperties->GetShaderIdentifier(L"ShadowClosestHitGroup"));

			mat->set_identifier(
				id,shadow			
			);
		}

	}
	RTX()
	{

		material_hits = std::make_shared< virtual_gpu_buffer<closesthit_identifier>>(1024 * 1024);
		while (blob.empty()) {
			resource_file_depender depender;


			D3D::shader_include In("shaders\\", depender);

			auto res = D3D12ShaderCompilerInfo::get().Compile_Shader_File("raytracing.hlsl", {}, "lib_6_3", "", &In);

			if (res)
			{
				blob = *res;
				break;
			}

		}
		//		CreateRaytracingPipelineStateObject(blobs);

		}
	/*shader_identifier identify(std::wstring name)
	{
		return  stateObjectProperties?::identify(stateObjectProperties->GetShaderIdentifier((name + L"HIT").c_str())):shader_identifier();
	}

	shader_identifier get_shadow()
	{
		return  stateObjectProperties ? ::identify(stateObjectProperties->GetShaderIdentifier(L"ShadowClosestHitGroup")) : shader_identifier();
	}
*/


	void prepare(CommandList::ptr & list)
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

		void render(MeshRenderContext::ptr context, Render::TextureView& texture, Render::RaytracingAccelerationStructure::ptr scene_as)
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
				dispatchDesc->HitGroupTable.SizeInBytes = sizeof(closesthit_identifier)*material_hits->max_size();
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
			rtx.set(_list->get_graphics());

			}

			{
				Slots::RaytracingRays rays;

				rays.GetOutput() = texture.get_uav();

				rays.set(_list->get_compute());
				rays.set(_list->get_graphics());

			}

			_list->transition(material_hits->buffer.get(), ResourceState::NON_PIXEL_SHADER_RESOURCE);
			_list->flush_transitions();
		
			// Bind the heaps, acceleration structure and dispatch rays.
			D3D12_DISPATCH_RAYS_DESC dispatchDesc = {};
			{
				//	commandList->SetComputeRootShaderResourceView(GlobalRootSignatureParams::AccelerationStructureSlot, m_topLevelAccelerationStructure->GetGPUVirtualAddress());
				DispatchRays(_list->get_native_list().Get(), m_dxrStateObject.Get(), &dispatchDesc);
			}

		}
};
