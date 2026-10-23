
import Core;
import GUI;

import HAL;

import <windows/windows.h>;
class Window : public HAL::window_provider
{
	HWND hwnd;
	wchar_t high_surrogate = 0;

	void InitWindow(int width, int height, LPCTSTR name);
	// The window draws its own title bar: the client area covers the whole window, the UI says
	// where the caption, the resize borders and the minimise / maximise / close buttons are
	// (InputHandler::hit_test_window), and Windows keeps the shadow, snapping and animations.
	bool custom_frame = true;
	static key_mods current_key_mods();
protected:
	Window(ivec2 size = { 1024, 768 }, std::string name = "Spectrum");
	~Window();
	/** Window events */

	std::atomic<bool> sizing;
	POINT MinWindowSize;
	ivec2 size;

	virtual	void on_size_begin();
	virtual	void on_size_end();
	virtual void on_destroy();
	virtual	void on_paint();
	virtual	void on_resize(vec2 new_size);

	InputHandler* input_handler = nullptr;
	//	std::string WindowsName;
public:
	void redraw();
	ivec2 get_size() const;
	HWND get_hwnd() const { return hwnd; }
	void* get_native_window() const override { return hwnd; }
	virtual LRESULT MsgProc(MSG msg);

	static void process_messages();

	// Window commands for the UI's own title bar buttons. Posted, so they are safe from the UI thread.
	bool has_custom_frame() const { return custom_frame; }
	void minimize();
	void toggle_maximize();
	void request_close();
	bool is_maximized() const;

	std::wstring get_clipboard_text() const;
	void set_clipboard_text(std::wstring_view text) const;


	static std::vector<std::string> file_open(const std::string& Name, const std::string& StartPath, const std::string& Extension);
};

class WinErrorLogger : public Log
{
	virtual void crash_error(std::string message, std::string_view str) override
	{
		std::string msg = message + " at \n" + std::string(str) + " \n Exit?";

		if (MessageBox(NULL, msg.c_str(), "ERROR", MB_YESNO) == IDNO)
		{
			if (Application::is_good())
				Application::get().shutdown();
		}
	}
};