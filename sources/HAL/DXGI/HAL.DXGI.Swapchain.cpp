module HAL:SwapChain;
import:Device;
import HAL;


import d3d12;

namespace HAL
{
	// With DXGI_SCALING_NONE, SetSourceSize does not crop: DXGI scales the source region up
	// to the whole buffer size and the window then shows the top-left part of that, i.e.
	// the picture is magnified by capacity/window. Leaving the source at the full buffer
	// makes NONE show the buffer 1:1 from its top-left corner, cropped to the window.
	constexpr bool kUseSetSourceSize = false;

	SwapChain::SwapChain(Device& device, swap_chain_desc c_desc) :device(device)
	{

		HWND hwnd = static_cast<HWND>(c_desc.window->get_native_window());
		RECT r;
		GetClientRect(hwnd, &r);
		logical_size = { std::max<int>(1, r.right - r.left), std::max<int>(1, r.bottom - r.top) };
		capacity = logical_size;
		oversized = c_desc.oversized;
		// TEMP-DEBUG: false = start with a window-sized buffer (rounded up to 256) instead of monitor-sized,
		// to test whether the 3440x1440 FP16 buffer is what limits fps to ~166 (exact-size mode ran 800).
		constexpr bool kCapacityToMonitor = false;
		if (c_desc.oversized && !kCapacityToMonitor)
		{
			capacity.x = (capacity.x + 255) & ~255;
			capacity.y = (capacity.y + 255) & ~255;
		}
		if (HMONITOR monitor = (c_desc.oversized && kCapacityToMonitor) ? MonitorFromWindow(hwnd, 2 /* MONITOR_DEFAULTTONEAREST: macros aren't exported by the d3d12 module */) : nullptr)
		{
			MONITORINFO mi = { sizeof(mi) };
			if (GetMonitorInfoW(monitor, &mi))
			{
				capacity.x = std::max<int>(capacity.x, mi.rcMonitor.right - mi.rcMonitor.left);
				capacity.y = std::max<int>(capacity.y, mi.rcMonitor.bottom - mi.rcMonitor.top);
			}
		}

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = capacity.x;
		swapChainDesc.Height = capacity.y;
		// NONE (flip model): the buffer is shown 1:1 from its top-left corner and cropped to
		// the window, so an oversized buffer needs no ResizeBuffers when the window changes.
		// (SetSourceSize was tried with STRETCH and the compositor ignored it.)
		swapChainDesc.Scaling = c_desc.oversized ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
		swapChainDesc.Format = ::to_native(c_desc.format);
		swapChainDesc.Stereo = c_desc.stereo && HAL::Adapters::get().get_factory()->IsWindowedStereoEnabled();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI::USAGE_RENDER_TARGET_OUTPUT | DXGI::USAGE_BACK_BUFFER;
		swapChainDesc.BufferCount = (c_desc.oversized ? 3 : 2) + static_cast<int>(swapChainDesc.Stereo);	// TEMP-DEBUG: 3 to test whether wait_for_free is vblank-paced
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

		//swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		{
			// Needed for Present(0, ALLOW_TEARING) to run uncapped in windowed mode.
			ComPtr<IDXGIFactory7> tear_factory;
			BOOL allow_tearing = FALSE;
			if (SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&tear_factory))) &&
				SUCCEEDED(tear_factory->CheckFeatureSupport(DXGI::FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing))) && allow_tearing)
				swapChainDesc.Flags |= DXGI::SWAP_CHAIN_FLAG_ALLOW_TEARING;
		}

		ComPtr<IDXGISwapChain1> swapChain;
		HRESULT res = HAL::Adapters::get().get_factory()->CreateSwapChainForHwnd(
			device.get_queue(CommandListType::DIRECT)->get_native().Get(),
			static_cast<HWND>(c_desc.window->get_native_window()),
			&swapChainDesc, nullptr, nullptr, &swapChain);

		swapChain.As(&m_swapChain);

		// Manual hooking's substitute for the interposer's automatic
		// IDXGISwapChain::Present hook — see [[project-streamline-dlss-integration]].
		if (nvidia::Streamline::get().available())
		{
			IDXGISwapChain3* raw = m_swapChain.Detach();
			void* raw_void = raw;
			if (nvidia::Streamline::get().upgrade_interface(&raw_void))
				raw = static_cast<IDXGISwapChain3*>(raw_void);
			m_swapChain.Attach(raw);
		}

		m_swapChain->GetDesc(&desc);
		HRESULT hr_src = kUseSetSourceSize ? m_swapChain->SetSourceSize(logical_size.x, logical_size.y) : S_OK;
		{ // TEMP-DEBUG: remove after investigating the oversized swapchain
			UINT sw = 0, sh = 0;
			HRESULT hr_get = m_swapChain->GetSourceSize(&sw, &sh);
			FILE* f = nullptr;
			if (fopen_s(&f, "swapchain_debug.temp", "a") == 0 && f)
			{
				fprintf(f, "SetSourceSize(%d,%d) hr=0x%08x; GetSourceSize hr=0x%08x -> %ux%u; capacity=%dx%d; scaling=%d swapeffect=%d\n",
					logical_size.x, logical_size.y, (unsigned)hr_src, (unsigned)hr_get, sw, sh, capacity.x, capacity.y,
					(int)swapChainDesc.Scaling, (int)swapChainDesc.SwapEffect);
				fclose(f);
			}
		}

		// FP16 backbuffers are scRGB: linear Rec.709, 1.0 = 80 nits, values
		// above 1 reach into HDR. DWM converts this for SDR displays too, so the
		// same buffer works whether or not the monitor is in HDR mode.
		if (swapChainDesc.Format == DXGI_FORMAT_R16G16B16A16_FLOAT)
		{
			UINT support = 0;
			if (SUCCEEDED(m_swapChain->CheckColorSpaceSupport(DXGI::COLOR_SPACE_RGB_FULL_G10_NONE_P709, &support))
				&& (support & DXGI::SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT))
				m_swapChain->SetColorSpace1(DXGI::COLOR_SPACE_RGB_FULL_G10_NONE_P709);
			else
				Log::get() << Log::LEVEL_WARNING << "[SwapChain] scRGB color space not supported for present" << Log::endl;
		}

		native_window = c_desc.window->get_native_window();
		refresh_display_info();

		frames.resize(desc.BufferCount);
		on_change();
		  
	//	swapchain_waiter = m_swapChain->GetFrameLatencyWaitableObject();

		//DWORD result = WaitForSingleObjectEx(
		//	swapchain_waiter,
		//	1000, // 1 second timeout (shouldn't ever occur)
		//	true
		//);
	}

	void SwapChain::present()
	{
		//uint index = m_frameIndex;

		auto native = m_swapChain;
		const ivec2 visible = logical_size;
		const bool set_every_frame = oversized && kUseSetSourceSize;
		const bool immediate = present_immediate;
		device.get_queue(CommandListType::DIRECT)->run([native, this, visible, set_every_frame, immediate](){
			PROFILE(L"Present");
			// TEMP-DEBUG: re-apply the source size right before each Present, and log what
			// DXGI reports right after it (does Present reset it?).
			if (set_every_frame)
				native->SetSourceSize(visible.x, visible.y);
			DXGI_SWAP_CHAIN_DESC1 sc_desc = {};
			native->GetDesc1(&sc_desc);
			const bool can_tear = (sc_desc.Flags & DXGI::SWAP_CHAIN_FLAG_ALLOW_TEARING) != 0;
			native->Present(immediate ? 0 : 1, immediate && can_tear ? DXGI::PRESENT_ALLOW_TEARING : 0);
			if (set_every_frame)
			{
				static int n = 0;
				UINT sw = 0, sh = 0;
				native->GetSourceSize(&sw, &sh);
				FILE* f = nullptr;
				if (n++ < 5 && fopen_s(&f, "swapchain_debug.temp", "a") == 0 && f)
				{
					fprintf(f, "after Present: wanted %dx%d, GetSourceSize -> %ux%u\n", visible.x, visible.y, sw, sh);
					fclose(f);
				}
			}
			});
	
		frames[m_frameIndex].fence_event = device.get_queue(CommandListType::DIRECT)->signal();

		m_frameIndex = (m_frameIndex + 1) % static_cast<UINT>(frames.size());
	/*	DWORD result = WaitForSingleObjectEx(
			swapchain_waiter,
			1000, // 1 second timeout (shouldn't ever occur)
			true
		);		 */
	}

	void  SwapChain::on_change()
	{
		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		for (UINT n = 0; n < frames.size(); n++)
		{
			D3D::Resource  render_target;
			m_swapChain->GetBuffer(n, IID_PPV_ARGS(&render_target));
			frames[n].m_renderTarget.reset(new TextureResource(device, API::NativeImportHandle{render_target}, TextureLayout::PRESENT));
			frames[n].m_renderTarget->set_name(std::string("swap_chain_") + std::to_string(n));
			
		}

		frames[0].m_renderTarget->debug=true;

			m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
	}

	namespace
	{
		// Windows' per-display "SDR content brightness", via DisplayConfig
		// (DXGI does not expose it). Matches the DXGI output by GDI device name.
		float query_sdr_white_nits(const wchar_t* gdi_device_name)
		{
			UINT32 path_count = 0, mode_count = 0;
			if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &path_count, &mode_count) != ERROR_SUCCESS)
				return 80.0f;

			std::vector<DISPLAYCONFIG_PATH_INFO> paths(path_count);
			std::vector<DISPLAYCONFIG_MODE_INFO> modes(mode_count);
			if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &path_count, paths.data(), &mode_count, modes.data(), nullptr) != ERROR_SUCCESS)
				return 80.0f;

			for (UINT32 i = 0; i < path_count; ++i)
			{
				DISPLAYCONFIG_SOURCE_DEVICE_NAME source = {};
				source.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
				source.header.size      = sizeof(source);
				source.header.adapterId = paths[i].sourceInfo.adapterId;
				source.header.id        = paths[i].sourceInfo.id;
				if (DisplayConfigGetDeviceInfo(&source.header) != ERROR_SUCCESS)
					continue;
				if (wcscmp(source.viewGdiDeviceName, gdi_device_name) != 0)
					continue;

				DISPLAYCONFIG_SDR_WHITE_LEVEL white = {};
				white.header.type      = DISPLAYCONFIG_DEVICE_INFO_GET_SDR_WHITE_LEVEL;
				white.header.size      = sizeof(white);
				white.header.adapterId = paths[i].targetInfo.adapterId;
				white.header.id        = paths[i].targetInfo.id;
				if (DisplayConfigGetDeviceInfo(&white.header) != ERROR_SUCCESS)
					return 80.0f;

				// Documented encoding: 1000 == 80 nits.
				return white.SDRWhiteLevel / 1000.0f * 80.0f;
			}
			return 80.0f;
		}
	}

	void SwapChain::refresh_display_info()
	{
		PROFILE(L"refresh_display_info");

		// A fresh factory every time: an existing one keeps reporting the
		// output state from when it was created (IsCurrent() goes false but
		// GetDesc1 stays stale), so HDR being toggled in Windows would be missed.
		ComPtr<IDXGIFactory7> factory;
		if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory))))
			return;

		RECT wr;
		if (!GetWindowRect(static_cast<HWND>(native_window), &wr))
			return;

		ComPtr<IDXGIOutput6> best;
		long best_area = -1;

		ComPtr<IDXGIAdapter1> adapter;
		for (UINT a = 0; SUCCEEDED(factory->EnumAdapters1(a, &adapter)); ++a)
		{
			ComPtr<IDXGIOutput> output;
			for (UINT o = 0; SUCCEEDED(adapter->EnumOutputs(o, &output)); ++o)
			{
				DXGI_OUTPUT_DESC od;
				output->GetDesc(&od);
				const RECT& r = od.DesktopCoordinates;
				long w = std::max(0L, std::min(wr.right, r.right) - std::max(wr.left, r.left));
				long h = std::max(0L, std::min(wr.bottom, r.bottom) - std::max(wr.top, r.top));
				if (w * h > best_area)
				{
					ComPtr<IDXGIOutput6> o6;
					if (SUCCEEDED(output.As(&o6)))
					{
						best_area = w * h;
						best = o6;
					}
				}
			}
		}

		DisplayInfo info;
		if (best)
		{
			DXGI_OUTPUT_DESC1 d;
			if (SUCCEEDED(best->GetDesc1(&d)))
			{
				info.hdr                 = d.ColorSpace == DXGI::COLOR_SPACE_RGB_FULL_G2084_NONE_P2020;
				info.min_nits            = d.MinLuminance;
				info.max_nits            = d.MaxLuminance;
				info.max_full_frame_nits = d.MaxFullFrameLuminance;
				info.sdr_white_nits      = info.hdr ? query_sdr_white_nits(d.DeviceName) : 80.0f;
			}
		}

		if (!(info == display_info))
		{
			Log::get() << "[SwapChain] display: hdr=" << info.hdr
				<< " max_nits=" << info.max_nits
				<< " max_full_frame_nits=" << info.max_full_frame_nits
				<< " sdr_white_nits=" << info.sdr_white_nits << Log::endl;
			display_info = info;
		}
	}

	void SwapChain::resize(ivec2 size)
	{
		if (size.x < 64) size.x = 64;
		if (size.y < 64) size.y = 64;

		if (size.x == logical_size.x && size.y == logical_size.y)
			return;

		// Fast path: the backbuffers are big enough, only the visible region changes.
		// Queued on the direct queue so it lands between the Presents of the frames
		// before and after the change, instead of racing the queue thread's Present.
		if (oversized && size.x <= capacity.x && size.y <= capacity.y)
		{
			logical_size = size;
			if constexpr (!kUseSetSourceSize)
				return;
			auto native = m_swapChain;
			device.get_queue(CommandListType::DIRECT)->run([native, size]() {
				HRESULT hr = native->SetSourceSize(size.x, size.y);
				UINT sw = 0, sh = 0; native->GetSourceSize(&sw, &sh); // TEMP-DEBUG
				FILE* f = nullptr;
				if (fopen_s(&f, "swapchain_debug.temp", "a") == 0 && f)
				{
					fprintf(f, "fast path: SetSourceSize(%d,%d) hr=0x%08x -> %ux%u\n", size.x, size.y, (unsigned)hr, sw, sh);
					fclose(f);
				}
			});
			return;
		}

		// Slow path: outgrew the allocation. While the user drags the window edge, jump
		// straight to the monitor size so the rest of the drag never lands here again;
		// trim_if_idle() shrinks it back afterwards (a big buffer costs ~4 ms/frame in
		// the compositor, 166 vs 600+ fps).
		last_resize_tick = GetTickCount64();
		ivec2 new_capacity = size;
		if (oversized)
		{
			new_capacity = ivec2((size.x + 255) & ~255, (size.y + 255) & ~255);
			if (HMONITOR monitor = MonitorFromWindow(static_cast<HWND>(native_window), 2 /* MONITOR_DEFAULTTONEAREST: macros aren't exported by the d3d12 module */))
			{
				MONITORINFO mi = { sizeof(mi) };
				if (GetMonitorInfoW(monitor, &mi))
				{
					new_capacity.x = std::max<int>(new_capacity.x, mi.rcMonitor.right - mi.rcMonitor.left);
					new_capacity.y = std::max<int>(new_capacity.y, mi.rcMonitor.bottom - mi.rcMonitor.top);
				}
			}
		}
		logical_size = size;
		reallocate(new_capacity);
	}

	void SwapChain::trim_if_idle()
	{
		if (!oversized)
			return;
		const ivec2 want((logical_size.x + 255) & ~255, (logical_size.y + 255) & ~255);
		if (capacity.x <= want.x && capacity.y <= want.y)
			return;
		if (GetTickCount64() - last_resize_tick < 300)
			return;
		reallocate(want);
	}

	void SwapChain::reallocate(ivec2 new_capacity)
	{
		for (auto type : magic_enum::enum_values<CommandListType>())
			device.get_queue(type)->signal_and_wait();

		for (auto&& f : frames)
			f.m_renderTarget = nullptr;

		// Without oversizing the buffers must match the window exactly, otherwise the
		// compositor scales them.
		capacity = new_capacity;

		desc.BufferDesc.Width = capacity.x;
		desc.BufferDesc.Height = capacity.y;
		m_swapChain->ResizeBuffers(desc.BufferCount,
			desc.BufferDesc.Width,
			desc.BufferDesc.Height,
			desc.BufferDesc.Format,
			desc.Flags);
		if constexpr (kUseSetSourceSize)
			m_swapChain->SetSourceSize(logical_size.x, logical_size.y);
		on_change();
	}
}