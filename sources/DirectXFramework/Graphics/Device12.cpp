module;
import d3d12;
#include <dxgi.h>

#include "GFSDK_Aftermath.h"
#include "NsightAftermathGpuCrashTracker.h"
module Graphics:Device;
import Log;
import :SwapChain;
import :PipelineState;
import :GPUTimer;
import :Samplers;
import :CommandList;
import :Queue;


import Debug;
import d3d12;

import IdGenerator;
import StateContext;
import Data;
import Debug;
import HAL;

//using namespace HAL;

namespace Graphics
{


	struct DREDNode
	{
		std::wstring name;
	private:
		SERIALIZE()
		{
			ar& NVP(name);
		}
	};


	void  Device::stop_all()
	{
		swap_chains.clear();

		for (auto&& q : queues)
			q = nullptr;


		Singleton<StaticCompiledGPUData>::reset();
		DescriptorHeapManager::reset();
		PipelineStateCache::reset();
		//#ifdef DEBUG
		// Enable the D3D12 debug layer.
		//#endif
	}
	Device::~Device()
	{
		stop_all();
		{
			ComPtr<ID3D12DebugDevice > debugController;
			//  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			//    debugController->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
			m_device->native_device.Get()->QueryInterface(IID_PPV_ARGS(&debugController));

			//	if (debugController)
			//		debugController->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
		}
	}

	std::shared_ptr<CommandList> Device::get_upload_list()
	{
		auto list = queues[CommandListType::COPY]->get_free_list();
		list->begin("UploadList");
		return list;
	}


	void  Device::check_errors()
	{
		auto hr = get_native_device()->GetDeviceRemovedReason();
		if (FAILED(hr))DumpDRED();

	}
	ComPtr<ID3D12Device5> Device::get_native_device()
	{
		return m_device->native_device;
	}

	Queue::ptr& Device::get_queue(CommandListType type)
	{
		return queues[type];
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS Device::to_native(const RaytracingBuildDescBottomInputs& desc)
	{
		return desc.to_native();
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS Device::to_native(const RaytracingBuildDescTopInputs& desc)
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs;

		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE::D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		inputs.Flags = desc.Flags;
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT::D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.NumDescs = desc.NumDescs;
		inputs.InstanceDescs = desc.instances.address;


		return inputs;
	}

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO Device::calculateBuffers(const RaytracingBuildDescBottomInputs& desc)
	{
		auto inputs = to_native(desc);
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;

		m_device->native_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

		return info;
	}
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO Device::calculateBuffers(const RaytracingBuildDescTopInputs& desc)
	{
		auto inputs = to_native(desc);
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;

		m_device->native_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);
		return info;
	}

	std::shared_ptr<SwapChain> Device::create_swap_chain(const swap_chain_desc& desc)
	{
		RECT r;
		GetClientRect(desc.window->get_hwnd(), &r);
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = r.right - r.left;
		swapChainDesc.Height = r.bottom - r.top;
		swapChainDesc.Format = ::to_native(desc.format);
		swapChainDesc.Stereo = desc.stereo && factory->IsWindowedStereoEnabled();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 3 + static_cast<int>(swapChainDesc.Stereo);
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		ComPtr<IDXGISwapChain1> swapChain;
		HRESULT res = factory->CreateSwapChainForHwnd(
			queues[CommandListType::DIRECT]->get_native().Get(),
			desc.window->get_hwnd(),
			&swapChainDesc, nullptr, nullptr, &swapChain);
		ComPtr<IDXGISwapChain3> m_swapChain;
		swapChain.As(&m_swapChain);
		std::shared_ptr<SwapChain> result;

		if (m_swapChain)
		{
			result.reset(new SwapChain(m_swapChain, desc));
			swap_chains.push_back(result);
		}

		return result;
	}

	void Device::create_sampler(D3D12_SAMPLER_DESC desc, CD3DX12_CPU_DESCRIPTOR_HANDLE handle)
	{
		m_device->native_device->CreateSampler(&desc, handle);
	}
	size_t Device::get_vram()
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
		adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);

		size_t usedVRAM = videoMemoryInfo.CurrentUsage / 1024 / 1024;

		return usedVRAM;
	}

	Device::Device()
	{
		const UUID _D3D12ExperimentalShaderModels = { /* 76f5573e-f13a-40f5-b297-81ce9e18933f */
	0x76f5573e,
	0xf13a,
	0x40f5,
	{ 0xb2, 0x97, 0x81, 0xce, 0x9e, 0x18, 0x93, 0x3f }
		};

		D3D12EnableExperimentalFeatures(1, &_D3D12ExperimentalShaderModels, nullptr, nullptr);



		//	Singleton::depends_on<Application>();
		auto t = CounterManager::get().start_count<Device>();
		//#ifdef DEBUG
				// Enable the D3D12 debug layer.


		ComPtr<ID3D12DeviceRemovedExtendedDataSettings> pDredSettings;
		(D3D12GetDebugInterface(IID_PPV_ARGS(&pDredSettings)));

		if (pDredSettings)
		{
			pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
			pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);

		}
		// Turn on AutoBreadcrumbs and Page Fault reporting




		ComPtr<ID3D12Debug> debugController;
		ComPtr<ID3D12Debug1> spDebugController1;

#ifdef DEV
		if constexpr (BuildOptions::Debug)
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->QueryInterface(IID_PPV_ARGS(&spDebugController1));


				debugController->EnableDebugLayer();
				//	spDebugController1->SetEnableGPUBasedValidation(true);
			}
#endif


		{
			auto t = CounterManager::get().start_count<IDXGIFactory2>();
			CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
		}
		UINT i = 0;
		//ComPtr<IDXGIAdapter3> pAdapter;

		// crasher = std::make_unique<GpuCrashTracker>();
		//
		// crasher->Initialize();
		{
			auto t = CounterManager::get().start_count<IDXGIAdapter>();

			IDXGIAdapter3* adapter;

			while (factory->EnumAdapters(i, reinterpret_cast<IDXGIAdapter**>(&adapter)) != DXGI_ERROR_NOT_FOUND)
			{

				ComPtr<IDXGIAdapter3> pAdapter;
				pAdapter.Attach(adapter);

				DXGI_ADAPTER_DESC adapter_desc;
				pAdapter->GetDesc(&adapter_desc);
				Log::get() << "adapter: " << adapter_desc.Description << Log::endl;
				++i;

				HAL::DeviceDesc desc;
				desc.adapter = pAdapter;
				auto device = std::make_shared<HAL::Device>(desc);
				D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
				D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
				D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
				D3D12_FEATURE_DATA_SHADER_MODEL supportedShaderModel = { D3D_SHADER_MODEL_6_7 };

				TEST(device->native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7)));
				TEST(device->native_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)));
				TEST(device->native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options)));
				device->native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_SHADER_MODEL, &supportedShaderModel, sizeof(supportedShaderModel));

				auto m_tiledResourcesTier = options.TiledResourcesTier;

				//	options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;

				if (supportedShaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_6 &&
					options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER::D3D12_MESH_SHADER_TIER_1)
				{

					m_device = device;
					this->adapter = pAdapter;
					Log::get() << "Selecting adapter: " << adapter_desc.Description << Log::endl;

				}
			}
		}
		assert(m_device);

		const uint32_t aftermathFlags =
			GFSDK_Aftermath_FeatureFlags_EnableMarkers |             // Enable event marker tracking.
			GFSDK_Aftermath_FeatureFlags_EnableResourceTracking |    // Enable tracking of resources.
			GFSDK_Aftermath_FeatureFlags_CallStackCapturing;    // Generate debug information for shaders.
		//
		// auto afterres = GFSDK_Aftermath_DX12_Initialize(
		// 	GFSDK_Aftermath_Version_API,
		// 	aftermathFlags,
		// 	m_device->native_device.Get());
		//


		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
		TEST(m_device->native_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5,
			&options5, sizeof(options5)));

		//#ifdef DEBUG
		ComPtr<ID3D12InfoQueue> d3dInfoQueue;
		if (SUCCEEDED(m_device->native_device.As(&d3dInfoQueue)))
		{

			//	d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			//	d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			D3D12_MESSAGE_ID hide[] =
			{
				D3D12_MESSAGE_ID_HEAP_ADDRESS_RANGE_INTERSECTS_MULTIPLE_BUFFERS,
				D3D12_MESSAGE_ID_GETHEAPPROPERTIES_INVALIDRESOURCE,
				D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT,
				D3D12_MESSAGE_ID_NON_RETAIL_SHADER_MODEL_WONT_VALIDATE,
				D3D12_MESSAGE_ID_CREATEPIPELINESTATE_CACHEDBLOBDESCMISMATCH,
				D3D12_MESSAGE_ID_EMPTY_DISPATCH
			};
			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);

		}
		//#endif
				// Describe and create the command queue.
		queues[CommandListType::DIRECT].reset(new Queue(CommandListType::DIRECT, this));
		queues[CommandListType::COMPUTE].reset(new Queue(CommandListType::COMPUTE, this));
		queues[CommandListType::COPY].reset(new Queue(CommandListType::COPY, this));


		//	queues[CommandListType::COPY] = queues[CommandListType::COMPUTE] = queues[CommandListType::DIRECT];
		//	m_device->SetStablePowerState(true);

		auto res = m_device->native_device->GetNodeCount();
		D3D12_FEATURE_DATA_D3D12_OPTIONS featureData;
		m_device->native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS, &featureData, sizeof(featureData));
		auto m_tiledResourcesTier = featureData.TiledResourcesTier;

		rtx = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;



		D3D12_FEATURE_DATA_SHADER_MODEL supportedShaderModel = { D3D_SHADER_MODEL_6_7 };
		m_device->native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_SHADER_MODEL, &supportedShaderModel, sizeof(supportedShaderModel));

		assert(supportedShaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_6);
	}


	Graphics::ResourceAllocationInfo Device::get_alloc_info(const HAL::ResourceDesc& desc)
	{
		auto native_desc = ::to_native(desc);
		/*if (native_desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
		{

			if ((native_desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) == 0)
			{
				native_desc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
			}
		}*/
		if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
		{

			assert(native_desc.SampleDesc.Count > 0);
		}
		D3D12_RESOURCE_ALLOCATION_INFO info = m_device->native_device->GetResourceAllocationInfo(0, 1, &native_desc);
		native_desc.Alignment = info.Alignment;


		// TODO small alignment
		/*if (info.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
		{
			native_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			info = m_device->native_device->GetResourceAllocationInfo(0, 1, &native_desc);


			if (info.Alignment != D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
			{
				native_desc.Alignment = 0;
				info = m_device->native_device->GetResourceAllocationInfo(0, 1, &native_desc);
			}

		}*/

		ResourceAllocationInfo result;

		result.size = info.SizeInBytes;
		result.alignment = info.Alignment;
		result.flags = D3D12_HEAP_FLAG_NONE;


		if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			result.flags |= D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		}
		else if (native_desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
		{
			result.flags |= D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
		}
		else
			result.flags |= D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
		if constexpr (BuildOptions::Debug)	TEST(m_device->native_device->GetDeviceRemovedReason());

		assert(result.size != UINT64_MAX);
		return result;
	}

	void Device::DumpDRED()
	{
		alive = false;
		ComPtr<ID3D12DeviceRemovedExtendedData>  pDred;
		TEST(m_device->native_device->QueryInterface(IID_PPV_ARGS(&pDred)));

		D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT DredAutoBreadcrumbsOutput = {};
		D3D12_DRED_PAGE_FAULT_OUTPUT DredPageFaultOutput = {};
		TEST(pDred->GetAutoBreadcrumbsOutput(&DredAutoBreadcrumbsOutput));
		TEST(pDred->GetPageFaultAllocationOutput(&DredPageFaultOutput));

		auto parse_node = [](const D3D12_AUTO_BREADCRUMB_NODE& node)
		{
			//	DREDNode parsed;

			//	parsed.name = node.pCommandListDebugNameW;


			Log::get() << node << Log::endl;
		};


		auto node = DredAutoBreadcrumbsOutput.pHeadAutoBreadcrumbNode;

		while (node)
		{
			parse_node(*node);
			node = node->pNext;
		}

	}

}
