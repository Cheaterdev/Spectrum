module;
#include <comdef.h>

module HAL:SwapChain;
import:Device;
import HAL;


import d3d12;

namespace HAL
{
	SwapChain::SwapChain(Device& device, swap_chain_desc c_desc) :device(device)
	{

		RECT r;
		GetClientRect(c_desc.window->get_hwnd(), &r);
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = r.right - r.left;
		swapChainDesc.Height = r.bottom - r.top;
		swapChainDesc.Format = ::to_native(c_desc.format);
		swapChainDesc.Stereo = c_desc.stereo && HAL::Adapters::get().get_factory()->IsWindowedStereoEnabled();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI::USAGE_RENDER_TARGET_OUTPUT | DXGI::USAGE_BACK_BUFFER;
		swapChainDesc.BufferCount = 2 + static_cast<int>(swapChainDesc.Stereo);
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

		//swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		
		ComPtr<IDXGISwapChain1> swapChain;
		HRESULT res = HAL::Adapters::get().get_factory()->CreateSwapChainForHwnd(
			device.get_queue(CommandListType::DIRECT)->get_native().Get(),
			c_desc.window->get_hwnd(),
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

		hwnd = c_desc.window->get_hwnd();
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
		device.get_queue(CommandListType::DIRECT)->run([native, this](){
			PROFILE(L"Present")
					native->Present(1, 0);
			});
	
		frames[m_frameIndex].fence_event = device.get_queue(CommandListType::DIRECT)->signal();

		m_frameIndex = 1-m_frameIndex;
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
		if (!GetWindowRect(hwnd, &wr))
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

		if (size.x == desc.BufferDesc.Width)
			if (size.y == desc.BufferDesc.Height)
				return;

		//	device.get_queue(CommandListType::DIRECT)->sync();

		{
			for (auto type : magic_enum::enum_values<CommandListType>())
				device.get_queue(type)->signal_and_wait();
		}

		for (auto&& f : frames)
			f.m_renderTarget = nullptr;

		desc.BufferDesc.Width = size.x;
		desc.BufferDesc.Height = size.y;
		m_swapChain->ResizeBuffers(desc.BufferCount,
			desc.BufferDesc.Width,
			desc.BufferDesc.Height,
			desc.BufferDesc.Format,
			desc.Flags);
		on_change();
	}
}