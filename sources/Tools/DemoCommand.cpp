#include "Commands.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

import Core;
import HAL;
import RenderSystem;
import GUI;
import Demo;

// spectrum_tool demo [--full] [--density N] [--size WxH]
//
// The Android demo screen (sources/Tools/Demo) in a window on this PC: same UI tree, same scene passes, same
// camera, mouse in place of touch (left-drag looks, wheel changes speed). By default the device is held to
// what the phone's GPU offers -- no mesh shaders, ray queries or sparse textures -- so the code paths are the
// phone's, with validation layers and a debugger available; --full leaves the device alone.

namespace
{
	struct Win32Window : HAL::hwnd_provider
	{
		HWND hwnd = nullptr;
		HWND get_hwnd() const override { return hwnd; }
	};

	struct Input
	{
		bool resized = false;
		bool closed = false;
	};

	Input g_input;

	vec2 client_pos(LPARAM lparam)
	{
		return vec2((float)GET_X_LPARAM(lparam), (float)GET_Y_LPARAM(lparam));
	}

	// The mouse stands in for fingers, through the same touch path the phone uses (user_interface::on_touch_event):
	//   left button   finger 0: press, drag, release
	//   right click   places a second finger that stays down where it was clicked (a click elsewhere moves it)
	//   middle click  lifts that second finger
	// so the dual-stick camera can be driven by one mouse: right-click on the left half to hold the move stick,
	// then drag with the left button on the right half to look around.
	struct TouchEmulation
	{
		bool left_down = false;
		bool second_down = false;
		vec2 second_pos = { 0, 0 };
	} g_touch;

	void send_touch(int flags, int id, vec2 pos)
	{
		if (auto ui = demo::ui())
		{
			GUI::touch_event event;
			event.flags = flags;
			event.touches.push_back({ id, pos.x, pos.y });
			ui->on_touch_event(event);
		}
	}

	void lift_second_finger()
	{
		if (g_touch.second_down)
			send_touch(GUI::touch_event::UP, 1, g_touch.second_pos);
		g_touch.second_down = false;
	}

	LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		switch (message)
		{
		case WM_MOUSEMOVE:
			if (g_touch.left_down)
				send_touch(GUI::touch_event::MOVE, 0, client_pos(lparam));
			return 0;

		case WM_LBUTTONDOWN:
			SetCapture(hwnd);
			g_touch.left_down = true;
			send_touch(GUI::touch_event::DOWN, 0, client_pos(lparam));
			return 0;

		case WM_LBUTTONUP:
			ReleaseCapture();
			if (g_touch.left_down)
				send_touch(GUI::touch_event::UP, 0, client_pos(lparam));
			g_touch.left_down = false;
			return 0;

		case WM_RBUTTONDOWN:
			lift_second_finger();
			g_touch.second_down = true;
			g_touch.second_pos = client_pos(lparam);
			send_touch(GUI::touch_event::DOWN, 1, g_touch.second_pos);
			return 0;

		case WM_MBUTTONDOWN:
			lift_second_finger();
			return 0;

		case WM_MOUSEWHEEL:
		{
			POINT p = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
			ScreenToClient(hwnd, &p);
			if (auto ui = demo::ui())
				ui->mouse_wheel_event_internal(mouse_wheel::VERTICAL, (float)GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA, vec2((float)p.x, (float)p.y));
			return 0;
		}

		case WM_SIZE:
			if (wparam != SIZE_MINIMIZED)
				g_input.resized = true;
			return 0;

		case WM_CLOSE:
		case WM_DESTROY:
			g_input.closed = true;
			PostQuitMessage(0);
			return 0;
		}
		return DefWindowProcW(hwnd, message, wparam, lparam);
	}

	bool create_window(Win32Window& window, int width, int height)
	{
		WNDCLASSW wc{};
		wc.lpfnWndProc = window_proc;
		wc.hInstance = GetModuleHandleW(nullptr);
		wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
		wc.lpszClassName = L"SpectrumDemo";
		RegisterClassW(&wc);

		RECT rect = { 0, 0, width, height };
		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
		window.hwnd = CreateWindowExW(0, wc.lpszClassName, L"Spectrum demo", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, wc.hInstance, nullptr);
		if (window.hwnd)
			ShowWindow(window.hwnd, SW_SHOWNORMAL);   // not the maximized state a hidden parent's startup info can ask for
		return window.hwnd != nullptr;
	}

	// Saves the window's current contents as a 24-bit BMP (PrintWindow asks the compositor for the window's
	// own image, so it works behind other windows). Used by --shot to look at a frame without a screen grab.
	bool save_window_bmp(HWND hwnd, const char* path)
	{
		RECT rect;
		GetClientRect(hwnd, &rect);
		const int w = rect.right, h = rect.bottom;
		if (w <= 0 || h <= 0)
			return false;

		HDC window_dc = GetDC(hwnd);
		HDC dc = CreateCompatibleDC(window_dc);
		HBITMAP bitmap = CreateCompatibleBitmap(window_dc, w, h);
		HGDIOBJ old = SelectObject(dc, bitmap);
		PrintWindow(hwnd, dc, 3 /* PW_CLIENTONLY | PW_RENDERFULLCONTENT */);

		BITMAPINFOHEADER bi{};
		bi.biSize = sizeof(bi);
		bi.biWidth = w;
		bi.biHeight = -h;   // top-down
		bi.biPlanes = 1;
		bi.biBitCount = 24;
		bi.biCompression = BI_RGB;
		const int stride = (w * 3 + 3) & ~3;
		std::vector<unsigned char> pixels((size_t)stride * h);
		GetDIBits(dc, bitmap, 0, h, pixels.data(), reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);

		BITMAPFILEHEADER bf{};
		bf.bfType = 0x4D42;
		bf.bfOffBits = sizeof(bf) + sizeof(bi);
		bf.bfSize = bf.bfOffBits + (DWORD)pixels.size();
		bi.biHeight = h;   // the file stores rows bottom-up: flip below
		std::ofstream file(path, std::ios::binary);
		file.write(reinterpret_cast<const char*>(&bf), sizeof(bf));
		file.write(reinterpret_cast<const char*>(&bi), sizeof(bi));
		for (int y = h - 1; y >= 0; --y)
			file.write(reinterpret_cast<const char*>(pixels.data() + (size_t)y * stride), stride);

		SelectObject(dc, old);
		DeleteObject(bitmap);
		DeleteDC(dc);
		ReleaseDC(hwnd, window_dc);
		return true;
	}

	void hold_to_phone_profile()
	{
		// What the Adreno on the phone does not offer; each is read by Device::init_managers.
		for (const char* name : { "SPECTRUM_DISABLE_MESH_SHADERS", "SPECTRUM_DISABLE_RAY_QUERY", "SPECTRUM_DISABLE_RTX",
		                          "SPECTRUM_DISABLE_TILED_3D", "SPECTRUM_DISABLE_TILED_DEPTH" })
			_putenv_s(name, "1");
	}
}

int command_demo(const CommandArgs& args)
{
	bool phone_profile = true;
	float density = 2.6f;
	int width = 540, height = 1200;
	std::string shot_path;
	int shot_after = 240;   // frames, so the scene and the graphs have settled

	for (size_t i = 0; i < args.size(); ++i)
	{
		if (args[i] == "--full")
			phone_profile = false;
		else if (args[i] == "--shot" && i + 1 < args.size())
			shot_path = args[++i];
		else if (args[i] == "--frames" && i + 1 < args.size())
			shot_after = std::stoi(args[++i]);
		else if (args[i] == "--density" && i + 1 < args.size())
			density = std::stof(args[++i]);
		else if (args[i] == "--size" && i + 1 < args.size())
			std::sscanf(args[++i].c_str(), "%dx%d", &width, &height);
		else
		{
			std::printf("demo: unknown argument '%s'\n", args[i].c_str());
			return 1;
		}
	}

	if (phone_profile)
		hold_to_phone_profile();

	if (!RenderSystem::create())
	{
		std::printf("demo: no suitable GPU device\n");
		return 2;
	}
	auto& device = RenderSystem::get().device();

	Win32Window window;
	if (!create_window(window, width, height))
	{
		std::printf("demo: could not create the window\n");
		return 3;
	}

	demo::set_density(density);

	std::shared_ptr<HAL::SwapChain> swap;
	std::shared_ptr<HAL::TextureResource> target;

	auto make_surface = [&]
	{
		HAL::swap_chain_desc desc{};
		desc.format = HAL::Format::R8G8B8A8_UNORM;   // the phone's surface: SDR, the scene rendered FP16 and converted
		desc.window = &window;
		desc.fullscreen = nullptr;
		desc.stereo = false;
		swap = std::make_shared<HAL::SwapChain>(device, desc);

		const auto& dims = swap->get_current_frame()->get_desc().as_texture().Dimensions;
		target = demo::make_target(device, ivec2((int)dims.x, (int)dims.y));
		demo::surface_changed();
	};

	make_surface();

	int frame = 0;
	MSG message;
	while (!g_input.closed)
	{
		while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT)
				g_input.closed = true;
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
		if (g_input.closed)
			break;

		if (g_input.resized)
		{
			g_input.resized = false;
			device.wait_idle();
			target.reset();
			swap.reset();
			make_surface();
		}

		demo::draw_frame(device, *swap, target, frame++);

		// --shot: after N frames save the window as a BMP and quit (for looking at the result without a screen grab).
		if (!shot_path.empty() && frame == shot_after)
		{
			device.wait_idle();
			const bool ok = save_window_bmp(window.hwnd, shot_path.c_str());
			std::printf("demo: %s %s\n", ok ? "saved" : "could not save", shot_path.c_str());
			break;
		}
	}

	device.wait_idle();
	target.reset();
	swap.reset();
	return 0;
}
