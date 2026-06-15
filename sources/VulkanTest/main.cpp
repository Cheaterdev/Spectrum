// VulkanTest — Vulkan backend smoke-test with a live GUI.
// Exercises the full UI stack (FrameGraph, GUI::user_interface, Skin, Font)
// without any 3D rendering.  Acts as the reference app while 3D PSOs are being
// brought up on Vulkan.

import GUI;
import Graphics;   // AssetManager, EngineAssets, Skin
import FrameGraph;
import HAL;
import Core;

#include "Platform/Window.h"

// pass_defaults.h declares the PassDefault<T> specialisations (bodies are
// out-of-line so each app provides its own implementations).
// Profiler.h / UI_Render.h / UIPipeline are NOT re-included here — they are
// already exported by `import FrameGraph;` and re-including them in this TU
// would cause class-redefinition errors (module types vs. header types are
// different entities from MSVC's perspective).
#include "../RenderSystem/FrameGraph/autogen/pass_defaults.h"
using namespace FrameGraph;

// ============================================================================
//  Helpers
// ============================================================================

class tick_timer
{
    std::chrono::time_point<std::chrono::system_clock> last_tick;
public:
    tick_timer() { last_tick = std::chrono::system_clock::now(); }
    double tick()
    {
        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double> dt = now - last_tick;
        last_tick = now;
        return dt.count();
    }
};

class count_meter
{
    double time = 0;
    tick_timer t;
    unsigned int ticks = 0;
    double average = 0;
public:
    bool tick()
    {
        time += t.tick();
        ticks++;
        if (time > 1.0)
        {
            average = ticks / time;
            ticks = 0; time = 0;
            return true;
        }
        return false;
    }
    float get() const { return (float)average; }
};

// ============================================================================
//  VulkanTestApp — window + UI + frame-graph
// ============================================================================

class VulkanTestApp : public Window, public GUI::user_interface
{
    HAL::SwapChain::ptr       swap_chain;
    FrameGraph::Graph         graph;
    Pipelines::UIPipeline     pipeline;

    tick_timer   frame_timer;
    count_meter  fps_meter;
    ivec2        new_size;

    // UI widgets
    GUI::Elements::label::ptr label_fps;
    GUI::Elements::label::ptr label_backend;

    int frame_counter = 0;

public:
    VulkanTestApp()
        : Window({ 1280, 720 }, "Vulkan UI Test")
    {
    	THREAD_SCOPE(GUI);
	
        Window::input_handler = this;

        HAL::swap_chain_desc sc_desc;
        sc_desc.window  = this;
        sc_desc.format  = HAL::Format::B8G8R8A8_UNORM;
        swap_chain = std::make_shared<HAL::SwapChain>(HAL::Device::get(), sc_desc);

        new_size = get_size();
        GUI::user_interface::size = new_size;

        // ---- Build UI tree --------------------------------------------------

        // Background fill
        {
            auto back = std::make_shared<GUI::Elements::image>();
            back->texture         = Skin::get().Fill;
            back->texture.tiled   = true;
            back->width_size      = GUI::size_type::MATCH_PARENT;
            back->height_size     = GUI::size_type::MATCH_PARENT;
            add_child(back);
        }

        // Top info panel
        {
            auto panel = std::make_shared<GUI::base>();
            panel->docking      = GUI::dock::TOP;
            panel->height_size  = GUI::size_type::FIXED;
            panel->size       = {0,40};

            label_backend = std::make_shared<GUI::Elements::label>();
            label_backend->text     = "Vulkan Backend — UI Test";
            label_backend->docking  = GUI::dock::FILL;
            panel->add_child(label_backend);
            add_child(panel);
        }

        // Adapter info (filled after device is up)
        {
            auto& props = HAL::Device::get().get_properties();
            label_backend->text = std::string("Vulkan: ") + (props.name);
        }

        // Status bar at bottom
        {
            auto bar = std::make_shared<GUI::Elements::status_bar>();
            label_fps = std::make_shared<GUI::Elements::label>();
            label_fps->text = "fps: --";
            bar->add_child(label_fps);
            add_child(bar);
        }
    }

    // ---- Render one frame ---------------------------------------------------
    void render()
    {
        ++frame_counter;
        if (frame_counter <= 5)
            Log::get() << "[Render] frame " << frame_counter << Log::endl;

        swap_chain->resize(new_size);
        swap_chain->wait_for_free();

       

        if (fps_meter.tick())
            label_fps->text = std::to_string((int)fps_meter.get()) + " fps  |  "
                + std::to_string(HAL::Device::get().get_vram()) + " MB VRAM";

        GUI::user_interface::size = new_size;
        process_ui((float)frame_timer.tick());

        graph.start_new_frame();
        // Render into our off-screen texture (passed as "swapchain" so the
        // UI pipeline writes to it).  The real swapchain is still cycled via
        // wait_for_free()/present() but won't have any rendered content.
        graph.builder.pass_texture("swapchain", swap_chain->get_current_frame());

        pipeline.add_passes(graph);
        create_graph(graph);       // injects UI_Render pass slots

        graph.setup();
        graph.compile(swap_chain->m_frameIndex);
        graph.render();
        auto fence = graph.commit_command_lists();

     
        // -----------------------------------------------------------------------

        graph.reset();

        if (frame_counter <= 5)
            Log::get() << "[Render] calling present frame " << frame_counter << Log::endl;
        swap_chain->present();
        if (frame_counter <= 5)
            Log::get() << "[Render] present returned frame " << frame_counter << Log::endl;
    }

    void on_resize(vec2 sz) override
    {
        new_size = vec2::max(sz, { 64, 64 });
        GUI::user_interface::on_size_changed(new_size);
    }

    void on_destroy() override
    {
        Application::get().shutdown();
    }
};

// ============================================================================
//  WinMain
// ============================================================================

int APIENTRY WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
    CoInitialize(nullptr);

    // Logging
    Log::create<WinErrorLogger>();
    VSOutputLogger::create();
    FileTXTLogger::create();
    Log::get().set_logging_level(Log::LEVEL_ALL);

    FileSystem::get().register_provider(std::make_shared<native_file_provider>());

    // Device
    HAL::Device::create();

    // Asset manager — needed for Skin textures and font glyphs
    AssetManager::create();
          	Application::create<Application>();
    // App window + UI
    auto app = std::make_shared<VulkanTestApp>();

    // Message + render loop
    bool running = true;
    while (running)
    {
        Window::process_messages();
        if (!Application::is_good()) break;
        app->render();
    }

    // Cleanup
    HAL::Device::get().stop_all();
    Skin::reset();
    Fonts::FontSystem::reset();
    AssetManager::reset();
    HAL::pixel_shader::reset_manager();
    HAL::vertex_shader::reset_manager();
    HAL::compute_shader::reset_manager();
    HAL::Device::reset();

    CoUninitialize();
    return 0;
}
