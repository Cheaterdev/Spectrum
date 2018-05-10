class dxgi_system{};
class Device: public Singleton<Device>
{
    friend class Singleton<Device>;
	typedef ClassLogger<dxgi_system> Logger;
    DX11_Device device;
    std::shared_ptr<Context> immediate_context;

    DXGI_Device		gi_device;
    DXGI_Adapter	gi_adapter;
    DXGI_Factory	gi_factory;

	DX11_Debug debug;
	DX11_InfoQueue  info_queue;
    std::vector<DXGI_Output> outputs;

	std::vector<std::shared_ptr<SwapChain>> swap_chains;

	std::recursive_mutex device_mutex;
public:

	const std::lock_guard<std::recursive_mutex>& lock()
	{
		return std::lock_guard<std::recursive_mutex>(device_mutex);
	}

	DX11_Device get_native_device()
	{
		return device;
	}

	Context& get_context()
	{
		return *immediate_context;
	}

	void enable_logging(bool value)
	{
#ifdef DEBUG

		if (info_queue)
		{
		//	info_queue->SetMuteDebugOutput(value);
		}
#endif // _DEBUG
	}
	std::shared_ptr<SwapChain> create_swap_chain(const swap_chain_desc & desc)
	{
	
		RECT r;
		GetClientRect(desc.window->get_hwnd(), &r);


		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	//	swapChainDesc.OutputWindow = desc.window->get_hwnd();
		swapChainDesc.Width = r.right - r.left; 
		swapChainDesc.Height = r.bottom - r.top;
		swapChainDesc.Format = desc.format;
		swapChainDesc.Stereo = desc.stereo&&gi_factory->IsWindowedStereoEnabled();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		//swapChainDesc.Windowed = true;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1+(int)swapChainDesc.Stereo;

		if (swapChainDesc.Stereo)
		{
			swapChainDesc.Scaling = DXGI_SCALING_NONE;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		}
		else
		{
			swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		}

		DXGI_SwapChain swap_chain;
		gi_factory->CreateSwapChainForHwnd(device, desc.window->get_hwnd(), &swapChainDesc, NULL, NULL, &swap_chain);
		//gi_factory->CreateSwapChain(device, &swapChainDesc, &result->swap_chain);

		std::shared_ptr<SwapChain> result;

		if (swap_chain)
		{
			result.reset(new SwapChain(swap_chain, desc));
			swap_chains.push_back(result);
		}
	
		return result;
	}
    Device();
    virtual ~Device();
};