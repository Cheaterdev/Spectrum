#include "pch.h"
#include "Samplers.h"
#include "Device12.h"

namespace DX12
{

	Queue::Queue(CommandListType type, Device * device)
	{
		auto t = CounterManager::get().start_count<Queue>();
		this->type = type;
		// Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = static_cast<D3D12_COMMAND_LIST_TYPE>(type);
		queueDesc.NodeMask = 1;
		device->get_native_device()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&native));
		// Create synchronization objects.
		{
			device->get_native_device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
			m_fenceValue = 0;
			// Create an event handle to use for frame synchronization.
			m_fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		}
		static int counter = 0;
		name = std::wstring(L"Queue_")+std::to_wstring(++counter);
		native->SetName(name.c_str());
		queue_thread = std::thread([this]
		{
			std::string name;

			if (this->type == CommandListType::DIRECT)
				name = "DIRECT";
			else if (this->type == CommandListType::COPY)
				name = "COPY";
			else if (this->type == CommandListType::COMPUTE)
				name = "COMPUTE";
			SetThreadName(std::string("DXQueue: ") + name);

			for (;;)
			{
				CommandList::ptr list;
				UINT64 fence;
				std::list< resource_set> resources_copy;
				{
					std::unique_lock<std::mutex> lock(this->queue_mutex);
					this->condition.wait(lock,
						[this, &resources_copy]
					{
						if (!this->tasks.empty())
						{
							//std::swap(resources_copy, resources_to_destroy);
						//	std::swap(resources_pending, resources_to_destroy);

						}

						else
							is_workload = false;

						return this->stop || !this->tasks.empty();
					});

					if (this->stop && this->tasks.empty())
						return;

					// TODO:: get all tasks, not only 1!!!!
					list = std::move(this->tasks.front());
					fence = list->get_execution_fence().get();
					this->tasks.pop();

					while (!resources_to_destroy.empty() && resources_to_destroy.front().first <= fence)
					{
						resources_copy.emplace_back(resources_to_destroy.front().second);
						resources_to_destroy.pop_front();
					}
				}
				// Wait until the previous frame is finished.
				fence_mutex.lock();
				auto compl = m_fence->GetCompletedValue();

				if (compl < fence)
				{
					m_fence->SetEventOnCompletion(fence, m_fenceEvent);
					fence_mutex.unlock();
					WaitForSingleObject(m_fenceEvent, INFINITE);
				}

				else
					fence_mutex.unlock();

			

				SPECTRUM_TRY
					list->on_execute();
				SPECTRUM_CATCH



				completed_value = fence;
				condition_ended.notify_all();

				if (resources_copy.size())
				{
				//	         Log::get() << 3 << Log::endl;
					resources_copy.clear();
				//	      Log::get() << 4 << Log::endl;
				}
			}
		});
		del_func = [this](CommandList * list)
		{
			if (stop)
				delete list;
			else
			{
				std::lock_guard<std::mutex> g(list_mutex);
				lists.emplace(list, del_func);
			}
		};
	}

	void Queue::stop_all()
	{
		wait();
		stop = true;
		condition.notify_all();

		if (queue_thread.joinable())
			queue_thread.join();
	}

	Queue::~Queue()
	{
		stop_all();
		//	resources_pending.clear();
	}

	UINT64 Queue::signal()
	{
		fence_mutex.lock();
		const UINT64 fence = ++m_fenceValue;
		native->Signal(m_fence.Get(), fence);
		fence_mutex.unlock();
		return fence;
	}

	void Queue::unused(ComPtr<ID3D12Pageable > resource)
	{
		std::lock_guard<std::mutex> g(queue_mutex);
		/*     if (this->stop || (this->tasks.empty() && !is_workload))
		{
		}//Log::get() << "deleting" << Log::endl;
		else*/

		UINT64 destroy_fence = last_known_fence + 50;
		if (resources_to_destroy.empty() || resources_to_destroy.back().first != destroy_fence)
		{
			resources_to_destroy.emplace_back(destroy_fence, resource_set());
		}
		resources_to_destroy.back().second.insert(resource);
	}

	std::shared_ptr<CommandList> Queue::get_free_list()
	{
		std::lock_guard<std::mutex> g(list_mutex);

		if (!lists.empty())
		{
			auto e = lists.front();
			lists.pop();
			return e;
		}

		std::shared_ptr<CommandList> res_ptr;
		res_ptr.reset(new CommandList(type), del_func);
		return res_ptr;
	}

	void Queue::wait(UINT64 value)
	{
		/* //  if (!is_complete(value))
		   {
			   condition_ended.wait(std::unique_lock<std::mutex>(ended_mutex),
									[this, value]
			   {
				   return is_complete(value);
			   });
		   }*/

		while (!is_complete(value))
			std::this_thread::yield();
	}
	void Queue::wait()
	{
		wait(m_fenceValue);
	}

	bool Queue::is_complete(UINT64 fence)
	{
		return  m_fence->GetCompletedValue() >= fence;
	}

	UINT64 Queue::get_last_step()
	{
		return m_fenceValue;
	}

	ComPtr<ID3D12CommandQueue> Queue::get_native()
	{
		return native;
	}


	// synchronized
	UINT64 Queue::execute_internal(CommandList* list)
	{

		bool need_wait = has_tasks();
	

		SPECTRUM_TRY


			if (stop)
				return m_fenceValue;

		is_workload = true;

		for (auto&fun : list->on_send_funcs)
		{
			fun();
		}
		list->on_send_funcs.clear();

	//	fix_transitions
		{
			auto& timer = Profiler::get().start(L"fix_pretransitions");

			auto transition_list = list->fix_pretransitions();
			if (transition_list)
			{		
				ID3D12CommandList* s = transition_list->get_native().Get();
				native->ExecuteCommandLists(1, &s);
			}
		}

		ID3D12CommandList* s = list->get_native_list().Get();
		native->ExecuteCommandLists(1, &s);

		last_known_fence = signal();
		list->execute_fence.set_value(last_known_fence);

		tasks.emplace(list->get_ptr());


		list->on_send();

		SPECTRUM_CATCH
			
		condition.notify_one();


		if (list->parent_list)
		{
			if (list->parent_list->child_executed())
			{
				execute_internal(static_cast<CommandList*>(list->parent_list.get()));
			}

		}

		if (need_wait)
		{
			wait(last_known_fence);
			process_tasks();
		}

		return last_known_fence;
	}


	std::shared_future<UINT64> Queue::execute(std::shared_ptr<CommandList> list)
	{
		std::unique_lock<std::mutex> lock(queue_mutex);

		if (!list->wait_for_execution_count)
		{
			std::promise<UINT64> res;
			res.set_value(execute_internal(list.get()));
			return res.get_future();
		}

		return list->get_execution_fence();
	}


	std::shared_future<UINT64> Queue::execute(CommandList* list)
	{
		return execute(list->get_ptr());
	}

	void  Device::stop_all()
	{
		swap_chains.clear();

		for (auto && q : queues)
			q = nullptr;

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

			if (debugController)
				debugController->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
		}
	}

	void Device::unused(ComPtr<ID3D12Pageable> resource)
	{
		auto&& q = queues[static_cast<int>(CommandListType::DIRECT)];

		if (q)q->unused(resource);
	}

	std::shared_ptr<CommandList> Device::get_upload_list()
	{
		/*  CommandList::ptr upload_list;
		  //  if (!upload_list)
		  {
			  upload_list.reset(new CommandList(CommandListType::DIRECT));
			  upload_list->begin();
		  }
		  return upload_list;*/
		auto list = queues[static_cast<int>(CommandListType::DIRECT)]->get_free_list();
		list->begin("");
		return list;
	}

	ComPtr<ID3D12Device5> Device::get_native_device()
	{
		return m_device;
	}

	Queue::ptr & Device::get_queue(CommandListType type)
	{
		return queues[static_cast<int>(type)];
	}



	std::shared_ptr<SwapChain> Device::create_swap_chain(const DX11::swap_chain_desc& desc)
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
			queues[static_cast<int>(CommandListType::DIRECT)]->get_native().Get(),
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
	//	Singleton::depends_on<Application>();
		auto t = CounterManager::get().start_count<Device>();
//#ifdef DEBUG
		// Enable the D3D12 debug layer.
		/*
			ComPtr<ID3D12Debug> debugController;
			CComPtr<ID3D12Debug1> spDebugController1;

			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			{
				debugController->EnableDebugLayer();

				debugController->QueryInterface(IID_PPV_ARGS(&spDebugController1));
				spDebugController1->SetEnableGPUBasedValidation(true);
			}
			*/
	
//#endif
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
			Log::get() <<"*** Unhandled Exception ***" << Log::endl;
		//	TRACE("*** Unhandled Exception ***");
		}
		

		

#ifdef DEBUG
		ComPtr<ID3D12InfoQueue> d3dInfoQueue;
		if (SUCCEEDED(m_device.As(&d3dInfoQueue)))
		{

		//	d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//	d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		}
#endif
		// Describe and create the command queue.
		queues[static_cast<int>(CommandListType::DIRECT)].reset(new Queue(CommandListType::DIRECT, this));
		// queues[static_cast<int>(CommandListType::COMPUTE)].reset(new Queue(CommandListType::COMPUTE));
		// queues[static_cast<int>(CommandListType::COPY)].reset(new Queue(CommandListType::COPY));
		auto res = m_device->GetNodeCount();
		D3D12_FEATURE_DATA_D3D12_OPTIONS featureData;
		m_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS, &featureData, sizeof(featureData));
		auto m_tiledResourcesTier = featureData.TiledResourcesTier;
	


	}

}
