#include "Window.h"


import windows;

void TrackMouse(HWND hwnd)
{
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(TRACKMOUSEEVENT);
    tme.dwFlags = TME_LEAVE; //Type of events to track & trigger.
    tme.dwHoverTime = 0; //How long the mouse has to be in the window to trigger a hover event.
    tme.hwndTrack = hwnd;
    TrackMouseEvent(&tme);
}


namespace
{
    struct dwm_margins
    {
        int left, right, top, bottom;
    };

    // dwmapi is loaded by hand: the windows module doesn't pull in its header or import
    // library, and one function is all it takes.
    void extend_frame_into_client(HWND hwnd)
    {
        HMODULE dwm = LoadLibraryW(L"dwmapi.dll");
        if (!dwm)
            return;

        using extend_fn = HRESULT(WINAPI*)(HWND, const dwm_margins*);
        if (auto extend = reinterpret_cast<extend_fn>(GetProcAddress(dwm, "DwmExtendFrameIntoClientArea")))
        {
            const dwm_margins margins = { 0, 0, 1, 0 };
            extend(hwnd, &margins);
        }

        // Windows 11: ask for the rounded corners explicitly (DWMWA_WINDOW_CORNER_PREFERENCE
        // = 33, DWMWCP_ROUND = 2); older versions don't know the attribute and ignore the call.
        using attribute_fn = HRESULT(WINAPI*)(HWND, DWORD, const void*, DWORD);
        if (auto set_attribute = reinterpret_cast<attribute_fn>(GetProcAddress(dwm, "DwmSetWindowAttribute")))
        {
            const int round = 2;
            set_attribute(hwnd, 33, &round, sizeof(round));
        }
    }

    // GetDpiForWindow needs Windows 10 1607; 96 (100%) when it is not there.
    UINT window_dpi(HWND hwnd)
    {
        using dpi_fn = UINT(WINAPI*)(HWND);
        static const auto get_dpi = reinterpret_cast<dpi_fn>(GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));
        return get_dpi ? get_dpi(hwnd) : 96;
    }

    bool is_window_button_hit(WPARAM hit)
    {
        return hit == HTMINBUTTON || hit == HTMAXBUTTON || hit == HTCLOSE;
    }
}

LRESULT CALLBACK MyWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Recover the pointer to our class, don't forget to type cast it back
    Window* winptr = (Window*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

    // Check if the pointer is NULL and call the Default WndProc
    if (winptr == NULL)
        return DefWindowProcW(hwnd, message, wParam, lParam);
    else
    {
        // Call the Message Handler for my class (MsgProc in my case)
        MSG msg;
        msg.hwnd = hwnd;
        msg.message = message;
        msg.wParam = wParam;
        msg.lParam = lParam;
        return winptr->MsgProc(msg);
    }
}

void Window::InitWindow(int width, int height, LPCTSTR name)
{
    // The project builds as _MBCS, but the window is registered as Unicode
    // explicitly: an ANSI window gets WM_CHAR as code-page bytes, so anything
    // outside the active code page can't be typed.
    const std::wstring wname = convert(std::string_view(name));

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)MyWndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = wname.c_str();
    wc.hIconSm = NULL;
    RegisterClassExW(&wc);
    HWND hWindow = CreateWindowExW((WS_EX_WINDOWEDGE) , wname.c_str(), wname.c_str(), WS_OVERLAPPEDWINDOW, -1, -1, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);
    SetWindowLongPtrW(hWindow, GWLP_USERDATA, (LONG_PTR)this);
    hwnd = hWindow;

    if (custom_frame)
    {
        // Keeps the system shadow and rounded corners with the frame gone: a one pixel
        // frame extension is enough for DWM to keep drawing them.
        extend_frame_into_client(hWindow);

        // Applies WM_NCCALCSIZE (client = the whole window) to the window just created.
        SetWindowPos(hWindow, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    ShowWindow(hWindow, SW_SHOWNORMAL);
    UpdateWindow(hWindow);
}

Window::Window(ivec2 size, std::string name)
{
    sizing = false;
    MinWindowSize = { 200, 100 };
    InitWindow(size.x, size.y, name.c_str());
}

Window::~Window()
{
    if (hwnd)
        on_destroy();
}

void Window::on_destroy()
{
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)0);
    ::DestroyWindow(hwnd);
    hwnd = 0;
}

void Window::on_size_begin()
{
	sizing = true;
}

void Window::on_size_end()
{
	sizing = false;
}

void Window::on_paint()
{
}
void  Window::on_resize(vec2 size)
{
}
LRESULT Window::MsgProc(MSG msg)
{
    switch (msg.message)
    {
        case WM_SIZE:
            size = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
			on_resize(size);
            break;
        case WM_MOUSEMOVE:
        {
            vec2 mouse_pos = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
			if (input_handler)input_handler->mouse_move_event(mouse_pos);
            TrackMouse(hwnd);
            break;
        }

        case WM_MOUSELEAVE:
        {
            if (!GetCapture())
            {
                vec2 mouse_pos = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
				if (input_handler)input_handler->mouse_action_event(mouse_action::CANCEL, mouse_button::LEFT, mouse_pos);
				if (input_handler)input_handler->mouse_action_event(mouse_action::CANCEL, mouse_button::RIGHT, mouse_pos);
				if (input_handler)input_handler->mouse_action_event(mouse_action::CANCEL, mouse_button::MIDDLE, mouse_pos);
            }

            //  return 0;
            break;
        }

        case WM_LBUTTONDOWN:
        {
            vec2 mouse_pos = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
			if (input_handler)input_handler->mouse_action_event(mouse_action::DOWN, mouse_button::LEFT, mouse_pos);
            break;
        }

        case WM_LBUTTONUP:
        {
            vec2 mouse_pos = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
			if (input_handler)input_handler->mouse_action_event(mouse_action::UP, mouse_button::LEFT, mouse_pos);
            break;
        }

        case WM_MBUTTONDOWN:
        {
            vec2 mouse_pos = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
			if (input_handler)input_handler->mouse_action_event(mouse_action::DOWN, mouse_button::MIDDLE, mouse_pos);
            break;
        }

        case WM_MBUTTONUP:
        {
            vec2 mouse_pos = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
			if (input_handler)input_handler->mouse_action_event(mouse_action::UP, mouse_button::MIDDLE, mouse_pos);
            break;
        }

        case WM_RBUTTONDOWN:
        {
            vec2 mouse_pos = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
			if (input_handler)input_handler->mouse_action_event(mouse_action::DOWN, mouse_button::RIGHT, mouse_pos);
            break;
        }

        case WM_RBUTTONUP:
        {
            vec2 mouse_pos = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
			if (input_handler)input_handler->mouse_action_event(mouse_action::UP, mouse_button::RIGHT, mouse_pos);
            break;
        }

        case WM_MOUSEWHEEL:
        {
            POINT pos = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
            ScreenToClient(hwnd, &pos);
			if (input_handler)input_handler->mouse_wheel_event(mouse_wheel::VERTICAL, static_cast<float>(GET_WHEEL_DELTA_WPARAM(msg.wParam)) / WHEEL_DELTA, vec2(pos.x, pos.y));
            break;
        }

        case  WM_KEYDOWN:
        {
			if(input_handler)input_handler->key_action_event(key_action::DOWN, (long)msg.wParam, current_key_mods());
            break;
        }

        case  WM_KEYUP:
        {
			if(input_handler)input_handler->key_action_event(key_action::UP, (long)msg.wParam, current_key_mods());
            break;
        }

        case WM_CHAR:
        {
            const wchar_t unit = (wchar_t)msg.wParam;

            // Characters outside the BMP arrive as two WM_CHARs (surrogate pair).
            if (IS_HIGH_SURROGATE(unit))
            {
                high_surrogate = unit;
                break;
            }

            char32_t ch = unit;
            if (IS_LOW_SURROGATE(unit))
            {
                if (!high_surrogate)
                    break;
                ch = 0x10000 + ((char32_t(high_surrogate) - 0xD800) << 10) + (char32_t(unit) - 0xDC00);
            }
            high_surrogate = 0;

            if (input_handler)input_handler->char_event(ch);
            break;
        }

        case WM_ERASEBKGND:
        {
            //	  on_paint();
            return true;
        }

        case  WM_ENTERSIZEMOVE:
        {
            on_size_begin();
            break;
        }

        case  WM_EXITSIZEMOVE:
        {
            on_size_end();
            break;
        }

        case WM_PAINT:
            on_paint();
            break;

        case WM_CLOSE:
            on_destroy();
            return 0;

        case WM_SETCURSOR:
            // The default handler sets the class arrow on every mouse move, which then fights the
            // cursor the UI picks (the edit box beam flickered to an arrow). Own the client area.
            if (LOWORD(msg.lParam) == HTCLIENT && input_handler)
            {
                input_handler->apply_cursor();
                return TRUE;
            }
            return DefWindowProcW(msg.hwnd, msg.message, msg.wParam, msg.lParam);

        case WM_NCCALCSIZE:
            if (custom_frame && msg.wParam)
            {
                // The client area is the whole window: no system title bar or borders.
                auto* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg.lParam);

                if (IsZoomed(hwnd))
                {
                    // A maximised window overhangs the monitor by its invisible frame, whose
                    // size depends on the DPI of the monitor it is on. The work area of that
                    // monitor (the screen minus the taskbar) is what it should fill.
                    MONITORINFO info = { sizeof(MONITORINFO) };
                    if (GetMonitorInfoW(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &info))
                    {
                        RECT work = info.rcWork;

                        // An auto-hiding taskbar leaves the work area equal to the monitor, and a
                        // window covering every pixel would keep the taskbar from coming back:
                        // one pixel off the bottom (the usual place) leaves it a hot edge.
                        if (EqualRect(&info.rcWork, &info.rcMonitor))
                            work.bottom -= 1;

                        params->rgrc[0] = work;
                    }
                }

                return 0;
            }
            break;

        case WM_NCHITTEST:
            if (custom_frame)
            {
                POINT point = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
                ScreenToClient(hwnd, &point);

                RECT client;
                GetClientRect(hwnd, &client);

                // The resize borders are ours to report now; none while maximised.
                // 6 px at 100% (the system's own is about 8, which here would eat into the tab
                // bar's top row), scaled by the DPI of this window's monitor.
                const int border = IsZoomed(hwnd) ? 0 : std::max(4, MulDiv(6, int(window_dpi(hwnd)), 96));
                const bool at_left   = point.x < border;
                const bool at_right  = point.x >= client.right - border;
                const bool at_top    = point.y < border;
                const bool at_bottom = point.y >= client.bottom - border;

                if (at_top && at_left)     return HTTOPLEFT;
                if (at_top && at_right)    return HTTOPRIGHT;
                if (at_bottom && at_left)  return HTBOTTOMLEFT;
                if (at_bottom && at_right) return HTBOTTOMRIGHT;
                if (at_top)                return HTTOP;
                if (at_bottom)             return HTBOTTOM;
                if (at_left)               return HTLEFT;
                if (at_right)              return HTRIGHT;

                if (input_handler)
                {
                    switch (input_handler->hit_test_window(vec2(float(point.x), float(point.y))))
                    {
                        case window_hit::caption:  return HTCAPTION;
                        case window_hit::minimize: return HTMINBUTTON;
                        case window_hit::maximize: return HTMAXBUTTON;
                        case window_hit::close:    return HTCLOSE;
                        default: break;
                    }
                }

                return HTCLIENT;
            }
            break;

        // The window buttons are in the non-client area as far as Windows is concerned
        // (so Windows 11 shows its snap layouts over the maximise button), but the UI draws
        // and handles them: the pointer is forwarded as ordinary mouse events.
        // Over the caption and the resize borders too: no client mouse move arrives there, so
        // without this the last element hovered stays lit (the first tab, when the pointer
        // enters the title bar from the left).
        case WM_NCMOUSEMOVE:
            if (custom_frame && input_handler)
            {
                POINT point = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
                ScreenToClient(hwnd, &point);
                input_handler->mouse_move_event(vec2(float(point.x), float(point.y)));

                TRACKMOUSEEVENT track = { sizeof(TRACKMOUSEEVENT), TME_LEAVE | TME_NONCLIENT, hwnd, 0 };
                TrackMouseEvent(&track);
            }
            break;

        case WM_NCMOUSELEAVE:
            if (custom_frame && input_handler)
                input_handler->mouse_move_event(vec2(-10000.0f, -10000.0f));   // nothing is hovered any more
            break;

        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONUP:
            if (custom_frame && input_handler && is_window_button_hit(msg.wParam))
            {
                POINT point = { GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) };
                ScreenToClient(hwnd, &point);
                input_handler->mouse_action_event(msg.message == WM_NCLBUTTONDOWN ? mouse_action::DOWN : mouse_action::UP,
                                                  mouse_button::LEFT, vec2(float(point.x), float(point.y)));
                return 0;
            }
            break;

        case WM_GETMINMAXINFO:
            LPMINMAXINFO pMaxInfo = (LPMINMAXINFO)msg.lParam;

            if (pMaxInfo)
                pMaxInfo->ptMinTrackSize = MinWindowSize;

            break;
    }

    return DefWindowProcW(msg.hwnd, msg.message, msg.wParam, msg.lParam);
}

void Window::minimize()
{
    PostMessageW(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

void Window::toggle_maximize()
{
    PostMessageW(hwnd, WM_SYSCOMMAND, IsZoomed(hwnd) ? SC_RESTORE : SC_MAXIMIZE, 0);
}

void Window::request_close()
{
    PostMessageW(hwnd, WM_CLOSE, 0, 0);
}

bool Window::is_maximized() const
{
    return hwnd && IsZoomed(hwnd);
}

void Window::process_messages()
{
    MSG msg;

    // W variants: the A pump would convert WM_CHAR back to code-page bytes.
    while (PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

std::wstring Window::get_clipboard_text() const
{
    std::wstring result;
    if (!OpenClipboard(hwnd))
        return result;

    if (HANDLE data = GetClipboardData(CF_UNICODETEXT))
    {
        if (auto text = static_cast<const wchar_t*>(GlobalLock(data)))
        {
            result = text;
            GlobalUnlock(data);
        }
    }

    CloseClipboard();
    return result;
}

void Window::set_clipboard_text(std::wstring_view text) const
{
    // Needs a real owner window: after OpenClipboard(nullptr), EmptyClipboard
    // leaves the clipboard ownerless and SetClipboardData fails.
    if (!OpenClipboard(hwnd))
        return;

    EmptyClipboard();

    const size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    if (HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, bytes))
    {
        auto dst = static_cast<wchar_t*>(GlobalLock(mem));
        std::memcpy(dst, text.data(), text.size() * sizeof(wchar_t));
        dst[text.size()] = 0;
        GlobalUnlock(mem);

        if (!SetClipboardData(CF_UNICODETEXT, mem))
            GlobalFree(mem);
    }

    CloseClipboard();
}

key_mods Window::current_key_mods()
{
    key_mods mods;
    mods.shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    mods.ctrl  = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    mods.alt   = (GetKeyState(VK_MENU) & 0x8000) != 0;
    return mods;
}

void Window::redraw()
{
    if (!sizing)
        on_paint();
}

ivec2 Window::get_size() const
{
    return size;
}


std::vector<std::string> Window::file_open(const std::string& Name, const std::string& StartPath, const std::string& Extension)
{
	std::vector<std::string> result;

	char Filestring[256];
	std::string returnstring;
	char FilterBuffer[512];
	{
		std::memset(FilterBuffer, 0, sizeof(FilterBuffer));
		std::memcpy(FilterBuffer, Extension.c_str(), std::min(Extension.size(), 512_t));

		for (int i = 0; i < 512; i++)
		{
			if (FilterBuffer[i] == '|')
				FilterBuffer[i] = 0;
		}
	}
    OPENFILENAMEA opf = { 0 };
	opf.hwndOwner = 0;
	opf.lpstrFilter = FilterBuffer;
	opf.lpstrCustomFilter = 0;
	opf.nMaxCustFilter = 0L;
	opf.nFilterIndex = 1L;
	opf.lpstrFile = Filestring;
	opf.lpstrFile[0] = '\0';
	opf.nMaxFile = 256;
	opf.lpstrFileTitle = 0;
	opf.nMaxFileTitle = 50;
	opf.lpstrInitialDir = StartPath.c_str();
	opf.lpstrTitle = Name.c_str();
	opf.nFileOffset = 0;
	opf.nFileExtension = 0;
	opf.lpstrDefExt = "*.*";
	opf.lpfnHook = NULL;
	opf.lCustData = 0;
	opf.Flags = (OFN_NOCHANGEDIR | OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT);//&~OFN_ALLOWMULTISELECT;
	opf.lStructSize = sizeof(OPENFILENAME);

	if (GetOpenFileNameA(&opf))
	{
		if (opf.nFileExtension)
		{
			result.push_back(opf.lpstrFile);
		}
		else
		{
			char* start = opf.lpstrFile;

			std::string dir = start;
			start += dir.size() + 1;
			dir += "\\";
		
			while (*start)
			{
				std::string file = start;
				start += file.size()+1;
				result.push_back(dir+file);
			}
		}
		
}
	return result;
}