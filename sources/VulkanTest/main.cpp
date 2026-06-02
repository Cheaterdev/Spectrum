// VulkanTest — minimal clear-screen test for the Vulkan HAL backend.
// No RenderSystem, no AssetManager, no Materials.
// Creates a window, device, swapchain, then clears to blue every frame.

import HAL;
import Core;

import <windows/windows.h>;

// ============================================================================
//  Minimal Win32 window
// ============================================================================

class TestWindow : public HAL::hwnd_provider
{
    HWND m_hwnd = nullptr;
    bool m_running = true;
    ivec2 m_size;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        auto* w = reinterpret_cast<TestWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (msg == WM_DESTROY || msg == WM_CLOSE)
        {
            if (w) w->m_running = false;
            PostQuitMessage(0);
            return 0;
        }
        if (msg == WM_SIZE && w)
            w->m_size = { LOWORD(lp), HIWORD(lp) };
        return DefWindowProc(hwnd, msg, wp, lp);
    }

public:
    explicit TestWindow(int w = 1280, int h = 720)
        : m_size(w, h)
    {
        WNDCLASSEXA wc{};
        wc.cbSize        = sizeof(wc);
        wc.style         = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc   = WndProc;
        wc.hInstance     = GetModuleHandle(nullptr);
        wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = "VulkanTestWnd";
        RegisterClassExA(&wc);

        RECT r{ 0, 0, w, h };
        AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);

        m_hwnd = CreateWindowExA(
            0, "VulkanTestWnd", "Vulkan Clear Screen Test",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            r.right - r.left, r.bottom - r.top,
            nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

        SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }

    ~TestWindow()
    {
        if (m_hwnd) DestroyWindow(m_hwnd);
    }

    HWND get_hwnd() const override { return m_hwnd; }
    bool is_running() const { return m_running; }
    ivec2 get_size() const { return m_size; }

    bool pump_messages()
    {
        MSG msg{};
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) return false;
        }
        return m_running;
    }
};

// ============================================================================
//  WinMain
// ============================================================================

int APIENTRY WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
    // ---- Logging ------------------------------------------------------------
    Log::create();
    VSOutputLogger::create();   // routes Log messages to the VS Output window

    // ---- Window -------------------------------------------------------------
    TestWindow window(1280, 720);

    // ---- File system --------------------------------------------------------
    FileSystem::get().register_provider(std::make_shared<native_file_provider>());

    // ---- HAL device ---------------------------------------------------------
    HAL::Device::create();
    auto& device = HAL::Device::get();

    // ---- Swapchain ----------------------------------------------------------
    HAL::swap_chain_desc sc_desc;
    sc_desc.window = &window;
    sc_desc.format = HAL::Format::B8G8R8A8_UNORM;
    auto swapchain = std::make_shared<HAL::SwapChain>(device, sc_desc);

    // ---- RTV descriptor slots -----------------------------------------------
    // Allocate one slot per swapchain image (must be 1-element each: Handle
    // asserts offset==0 when get_count() is called from set_rtv).
    auto& gpu_data = device.get_static_gpu_data();
    HAL::DescriptorHeapIndex rtv_index{ HAL::DescriptorHeapType::RTV,
                                         HAL::DescriptorHeapFlags::None };
    HAL::Handle rtv_slots[2] = {
        gpu_data.alloc_descriptor<HAL::Handle>(1, rtv_index),
        gpu_data.alloc_descriptor<HAL::Handle>(1, rtv_index),
    };
    HAL::RTVHandle rtvs[2];

    auto& direct_queue = *device.get_queue(HAL::CommandListType::DIRECT);

    // ---- Frame loop ---------------------------------------------------------
    while (window.pump_messages())
    {
        // wait_for_free() paces the CPU and the swapchain image is already
        // acquired (done at the end of present(), just like D3D12).
        swapchain->wait_for_free();

        auto frame_rt = swapchain->get_current_frame();  // TextureResource::ptr
        uint fi = swapchain->m_frameIndex;

        // Refresh the RTV for this slot (backbuffer pointer may change on resize).
        HAL::Views::RenderTarget rtv_view{
            .Resource = frame_rt,
            .Format   = HAL::Format::B8G8R8A8_UNORM,
            .View     = HAL::Views::RenderTarget::Texture2D{ 0, 0 }
        };
        rtv_slots[fi % 2] = rtv_view;   // stores ResourceInfo in the slot
        rtvs[fi % 2] = HAL::RTVHandle(rtv_slots[fi % 2]);

        // ---- Record: set+clear RT (handles transitions internally), then present
        auto list = direct_queue.get_free_list();
        list->begin(L"ClearFrame");

        // Build a minimal CompiledRT with just the swapchain RTV.
        HAL::CompiledRT crt;
        crt.table_rtv = rtvs[fi % 2];

        // set_rtv with ClearColor transitions the resource, sets the handle,
        // and clears to black.  We then transition to PRESENT afterwards.
        list->get_graphics().set_rtv(crt,
            HAL::RTOptions::Default | HAL::RTOptions::ClearColor);

        list->end();
        list->execute();

        swapchain->present();
    }

    // ---- Cleanup ------------------------------------------------------------
    direct_queue.signal_and_wait();
    device.get_queue(HAL::CommandListType::COMPUTE)->signal_and_wait();
    device.get_queue(HAL::CommandListType::COPY)->signal_and_wait();
    device.stop_all();
    HAL::Device::reset();

    return 0;
}
