#include "DX12/Swapchain12.h"
#include "Application/Application.h"
#include "GUI/GUI.h"

class Window : public DX12::hwnd_provider
{
    HWND hwnd;

    void InitWindow(int width, int height, LPCTSTR name);
protected:
    Window(ivec2 size = {1024, 768}, std::string name = "Spectrum");
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

	InputHandler * input_handler = nullptr;
    //	std::string WindowsName;
public:
    void redraw();
    ivec2 get_size() const;
    virtual	HWND get_hwnd() const { return hwnd; }
    virtual LRESULT MsgProc(MSG msg);

    static void process_messages();
};

class WinErrorLogger : public Log
{
    virtual void crash_error(std::string message, std::string str) override
    {
        std::string msg = message + " at \n" + str + " \n Exit?";

        if (MessageBox(NULL, msg.c_str(), "ERROR", MB_YESNO) == IDNO)
        {
            if (Application::is_good())
                Application::get().shutdown();
        }
    }
};