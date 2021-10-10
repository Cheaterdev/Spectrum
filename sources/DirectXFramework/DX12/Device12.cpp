#include "pch.h"
#include "Samplers.h"
#include "Device12.h"
#include "Swapchain12.h"
#include "GPUTimer.h"

#include "GFSDK_Aftermath.h"
#include "NsightAftermathGpuCrashTracker.h"


namespace DX12
{


	struct DREDNode
	{
		std::wstring name;
	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
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
			m_device.Get()->QueryInterface(IID_PPV_ARGS(&debugController));

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
		if(FAILED(hr))DumpDRED();
		
	}
	ComPtr<ID3D12Device5> Device::get_native_device()
	{
		return m_device;
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

		m_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

		return info;
	}
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO Device::calculateBuffers(const RaytracingBuildDescTopInputs& desc)
	{
		auto inputs = to_native(desc);
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;

		m_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);
		return info;
	}

	std::shared_ptr<SwapChain> Device::create_swap_chain(const swap_chain_desc& desc)
	{
		RECT r;
		GetClientRect(desc.window->get_hwnd(), &r);
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = r.right - r.left;
		swapChainDesc.Height = r.bottom - r.top;
		swapChainDesc.Format = desc.format;
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
		m_device->CreateSampler(&desc, handle);
	}
	size_t Device::get_vram()
	{
		DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
		vAdapters[0]->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);

		size_t usedVRAM = videoMemoryInfo.CurrentUsage / 1024 / 1024;

		return usedVRAM;
	}

	Device::Device()
	{


		D3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModels, nullptr, nullptr);



		//	Singleton::depends_on<Application>();
		auto t = CounterManager::get().start_count<Device>();
		//#ifdef DEBUG
				// Enable the D3D12 debug layer.


		ComPtr<ID3D12DeviceRemovedExtendedDataSettings> pDredSettings;
		TEST(D3D12GetDebugInterface(IID_PPV_ARGS(&pDredSettings)));

		// Turn on AutoBreadcrumbs and Page Fault reporting
		pDredSettings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
		pDredSettings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);


		

		ComPtr<ID3D12Debug> debugController;
		ComPtr<ID3D12Debug1> spDebugController1;

#ifdef DEV
		//if(false)
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

		{
			auto t = CounterManager::get().start_count<IDXGIAdapter>();

			IDXGIAdapter3* adapter;

			while (factory->EnumAdapters(i, reinterpret_cast<IDXGIAdapter**>(&adapter)) != DXGI_ERROR_NOT_FOUND)
			{

				ComPtr<IDXGIAdapter3> pAdapter;
				pAdapter.Attach(adapter);

				vAdapters.push_back(pAdapter);
				DXGI_ADAPTER_DESC desc;
				pAdapter->GetDesc(&desc);
				Log::get() << "adapter: " << convert(std::wstring(desc.Description)) << Log::endl;
				++i;
			}
		}

		crasher = std::make_unique<GpuCrashTracker>();

		crasher->Initialize();
		try
		{
			HRESULT hr = D3D12CreateDevice(
				vAdapters[0].Get(),
				D3D_FEATURE_LEVEL_12_1,
				IID_PPV_ARGS(&m_device)
			);
		}
		catch (_com_error  error)
		{
			Log::get() << error.ErrorMessage() << Log::endl;

		}
		catch (...)
		{
			Log::get() << "*** Unhandled Exception ***" << Log::endl;
			//	TRACE("*** Unhandled Exception ***");
		}

		const uint32_t aftermathFlags =
			GFSDK_Aftermath_FeatureFlags_EnableMarkers |             // Enable event marker tracking.
			GFSDK_Aftermath_FeatureFlags_EnableResourceTracking |    // Enable tracking of resources.
			GFSDK_Aftermath_FeatureFlags_CallStackCapturing;    // Generate debug information for shaders.

		auto afterres = GFSDK_Aftermath_DX12_Initialize(
			GFSDK_Aftermath_Version_API,
			aftermathFlags,
			m_device.Get());
		

		
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
		TEST(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5,
			&options5, sizeof(options5)));

		//#ifdef DEBUG
		ComPtr<ID3D12InfoQueue> d3dInfoQueue;
		if (SUCCEEDED(m_device.As(&d3dInfoQueue)))
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

		auto res = m_device->GetNodeCount();
		D3D12_FEATURE_DATA_D3D12_OPTIONS featureData;
		m_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS, &featureData, sizeof(featureData));
		auto m_tiledResourcesTier = featureData.TiledResourcesTier;

		rtx = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;



		D3D12_FEATURE_DATA_SHADER_MODEL supportedShaderModel = { D3D_SHADER_MODEL_6_7 };
		m_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_SHADER_MODEL, &supportedShaderModel, sizeof(supportedShaderModel));

		assert(supportedShaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_6);
	}


	DX12::ResourceAllocationInfo Device::get_alloc_info(CD3DX12_RESOURCE_DESC& desc)
	{

		if (desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			if ((desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) == 0)
			{
				desc.Alignment = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
			}
		}

		D3D12_RESOURCE_ALLOCATION_INFO info = m_device->GetResourceAllocationInfo(0, 1, &desc);
		desc.Alignment = info.Alignment;

		if (info.Alignment != D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT)
		{
			desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			info = m_device->GetResourceAllocationInfo(0, 1, &desc);


			if (info.Alignment != D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
			{
				desc.Alignment = 0;
				info = m_device->GetResourceAllocationInfo(0, 1, &desc);
			}

		}

		ResourceAllocationInfo result;

		result.size = info.SizeInBytes;
		result.alignment = info.Alignment;
		result.flags = D3D12_HEAP_FLAG_NONE;


		if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
		{
			result.flags |= D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		}
		else if (desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
		{
			result.flags |= D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
		}
		else
			result.flags |= D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;

		return result;
	}

	void  Device::create_rtv(Handle h, Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC rtv)
	{
		*h.resource_info = ResourceInfo(resource, rtv);
		m_device->CreateRenderTargetView(resource->get_native().Get(), &rtv, h.cpu);
	}

	void  Device::create_srv(Handle h, Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC srv)
	{
		*h.resource_info = ResourceInfo(resource, srv);
		if (srv.ViewDimension == D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2D)
		{
			assert(srv.Texture2D.MipLevels>0);
			assert(srv.Texture2D.MostDetailedMip + srv.Texture2D.MipLevels<= resource->get_desc().MipLevels);
		}

		if (srv.ViewDimension == D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURE2DARRAY)
		{
			assert(srv.Texture2DArray.MipLevels > 0);
			assert(srv.Texture2DArray.MostDetailedMip + srv.Texture2DArray.MipLevels <= resource->get_desc().MipLevels);
		}

	if (srv.ViewDimension == D3D12_SRV_DIMENSION::D3D12_SRV_DIMENSION_TEXTURECUBE)
		{
		assert(srv.TextureCube.MipLevels > 0);
			assert(srv.TextureCube.MostDetailedMip + srv.TextureCube.MipLevels <= resource->get_desc().MipLevels);
		}

	m_device->CreateShaderResourceView(resource ? resource->get_native().Get() : nullptr, &srv, h.cpu);
	}

	void  Device::create_uav(Handle h, Resource* resource, D3D12_UNORDERED_ACCESS_VIEW_DESC uav, Resource* counter) {
		*h.resource_info = ResourceInfo(resource, counter, uav);
		m_device->CreateUnorderedAccessView(resource->get_native().Get(), counter ? counter->get_native().Get() : nullptr, &uav, h.cpu);
	}

	void  Device::create_cbv(Handle h, Resource* resource, D3D12_CONSTANT_BUFFER_VIEW_DESC cbv) {
		*h.resource_info = ResourceInfo(resource, cbv);
		m_device->CreateConstantBufferView(&cbv, h.cpu);
	}

	void  Device::create_dsv(Handle h, Resource* resource, D3D12_DEPTH_STENCIL_VIEW_DESC dsv) {
		*h.resource_info = ResourceInfo(resource, dsv);
		m_device->CreateDepthStencilView(resource->get_native().Get(), &dsv, h.cpu);
	}


	void Device::DumpDRED()
	{
		alive = false;
		ComPtr<ID3D12DeviceRemovedExtendedData>  pDred;
		TEST(m_device->QueryInterface(IID_PPV_ARGS(&pDred)));

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

		while(node)
		{
			parse_node(*node);
			node = node->pNext;
		}
		
	}

}
