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
            return DefWindowProcW(msg.hwnd, msg.message, msg.wParam, msg.lParam);

        case WM_GETMINMAXINFO:
            LPMINMAXINFO pMaxInfo = (LPMINMAXINFO)msg.lParam;

            if (pMaxInfo)
                pMaxInfo->ptMinTrackSize = MinWindowSize;

            break;
    }

    return DefWindowProcW(msg.hwnd, msg.message, msg.wParam, msg.lParam);
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