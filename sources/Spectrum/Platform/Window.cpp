#include "pch.h"
#include "Window.h"

#include <ShlObj.h>
#include <Shobjidl.h>
#include <Commdlg.h>
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
    Window* winptr = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    // Check if the pointer is NULL and call the Default WndProc
    if (winptr == NULL)
        return DefWindowProc(hwnd, message, wParam, lParam);
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
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)MyWndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = name;
    wc.hIconSm = NULL;
    RegisterClassEx(&wc);
    HWND hWindow = CreateWindowEx((WS_EX_WINDOWEDGE) , name, name, WS_OVERLAPPEDWINDOW, -1, -1, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);
    SetWindowLongPtr(hWindow, GWLP_USERDATA, (LONG_PTR)this);
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
    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)0);
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
{}
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
			if(input_handler)input_handler->key_action_event((long)msg.wParam);
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
            return DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);

        case WM_GETMINMAXINFO:
            LPMINMAXINFO pMaxInfo = (LPMINMAXINFO)msg.lParam;

            if (pMaxInfo)
                pMaxInfo->ptMinTrackSize = MinWindowSize;

            break;
    }

    return DefWindowProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);
}

void Window::process_messages()
{
    MSG msg;

    while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
    {
        // Translate and dispatch the message
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
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
		memset(FilterBuffer, 0, sizeof(FilterBuffer));
		memcpy(FilterBuffer, Extension.c_str(), std::min(Extension.size(), 512_t));

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