module Graphics:Device;
import Log;
import :PipelineState;
import :GPUTimer;
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
	void  Device::stop_all()
	{
		swap_chains.clear();

		for (auto&& q : queues)
			q = nullptr;


		Singleton<StaticCompiledGPUData>::reset();
		HAL::DescriptorHeapManager::reset();
		PipelineStateCache::reset();
	}

	Device::~Device()
	{
		stop_all();
	}

	std::shared_ptr<CommandList> Device::get_upload_list()
	{
		auto list = queues[CommandListType::COPY]->get_free_list();
		list->begin("UploadList");
		return list;
	}

	ComPtr<ID3D12Device5> Device::get_native_device()
	{
		return native_device;
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
		inputs.InstanceDescs = ::to_native(desc.instances);


		return inputs;
	}

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO Device::calculateBuffers(const RaytracingBuildDescBottomInputs& desc)
	{
		auto inputs = to_native(desc);
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;

		native_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

		return info;
	}
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO Device::calculateBuffers(const RaytracingBuildDescTopInputs& desc)
	{
		auto inputs = to_native(desc);
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;

		native_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);
		return info;
	}

	std::shared_ptr<HAL::SwapChain> Device::create_swap_chain(const HAL::swap_chain_desc& desc)
	{
		RECT r;
		GetClientRect(desc.window->get_hwnd(), &r);
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = r.right - r.left;
		swapChainDesc.Height = r.bottom - r.top;
		swapChainDesc.Format = ::to_native(desc.format);
		swapChainDesc.Stereo = desc.stereo && HAL::Adapters::get().get_factory()->IsWindowedStereoEnabled();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 3 + static_cast<int>(swapChainDesc.Stereo);
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		ComPtr<IDXGISwapChain1> swapChain;
		HRESULT res = HAL::Adapters::get().get_factory()->CreateSwapChainForHwnd(
			queues[CommandListType::DIRECT]->get_native().Get(),
			desc.window->get_hwnd(),
			&swapChainDesc, nullptr, nullptr, &swapChain);
		ComPtr<IDXGISwapChain3> m_swapChain;
		swapChain.As(&m_swapChain);
		std::shared_ptr<HAL::SwapChain> result;

		if (m_swapChain)
		{
			result.reset(new HAL::SwapChain(m_swapChain, desc));
			swap_chains.push_back(result);
		}

		return result;
	}

	 std::shared_ptr<Device> Device::create_singleton()
	{
		HAL::init();

		std::shared_ptr<Device> result;
		HAL::Adapters::get().enumerate([&](HAL::Adapter::ptr adapter)
			{

				DXGI_ADAPTER_DESC adapter_desc = adapter->get_desc();
				Log::get() << "adapter: " << adapter_desc.Description << Log::endl;

				HAL::DeviceDesc desc;
				desc.adapter = adapter;
				auto device = std::make_shared<Device>(desc);

				const auto &props = device->get_properties();


				if (props.mesh_shader && props.full_bindless)
				{
					Log::get() << "Selecting adapter: " << adapter_desc.Description << Log::endl;
					result = device;
				}
			});
		return result;
	}

	Device::Device(HAL::DeviceDesc desc)
	{
		init(desc);

		queues[CommandListType::DIRECT].reset(new Queue(CommandListType::DIRECT, this));
		queues[CommandListType::COMPUTE].reset(new Queue(CommandListType::COMPUTE, this));
		queues[CommandListType::COPY].reset(new Queue(CommandListType::COPY, this));

		rtx = get_properties().rtx;
	}



}
