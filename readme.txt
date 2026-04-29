================================================================================
  SPECTRUM — PROJECT OVERVIEW
  (compiled from active development context)
================================================================================


--------------------------------------------------------------------------------
  HIGH-LEVEL ARCHITECTURE
--------------------------------------------------------------------------------

Spectrum is a C++20 real-time rendering engine built on top of Direct3D 12.
It is structured as several layers:

  HAL/          — Hardware Abstraction Layer over D3D12 (command lists, PSOs,
                  resources, ray tracing acceleration structures, work graphs)

  RenderSystem/ — The engine's rendering module (C++20 module: "Graphics").
                  Contains the frame graph, scene management, effects, assets,
                  materials, and all render pass logic.

  SIGParser/    — A code-generation tool that reads .sig files and emits C++
                  headers (pass structs, slot bindings, PSO declarations,
                  pipeline classes, pass_defaults.h) via Jinja2 templates.

  Spectrum/     — The application layer (main.cpp and friends). Wires the
                  engine together, owns the window/swap chain, and hosts any
                  pass implementations that are too app-specific for RenderSystem.


--------------------------------------------------------------------------------
  C++20 MODULE STRUCTURE
--------------------------------------------------------------------------------

The rendering module is named "Graphics" and is composed of many interface
partitions (export module Graphics:Foo) aggregated in Graphics.ixx.

Convention observed in the project:
  - .ixx files are exported interface partitions (export module Graphics:Foo)
  - Implementation bodies for a partition live either in a matching .cpp
    (module Graphics:Foo) or inline in the .ixx itself.
  - A handful of "system" partitions (e.g. RTXPassSystem, SceneSystem,
    PreSceneSystem) exist solely to provide PassDefault<> implementations
    and are exported so the linker sees them.

Key partitions:
  :FrameGraphContext  — shared per-frame context structs (see below)
  :Context            — SlotID enum, layout helpers
  :MeshRenderer       — GBuffer, GBufferViewDesc, MeshRenderContext
  :Scene              — Scene, SceneObject
  :RTX                — RTX singleton (ray tracing dispatch helpers)
  :PSSM               — Parallel-split shadow maps
  :VoxelGI            — Voxel global illumination
  :Sky / :BRDF / :SMAA / :FSR / :BlueNoise / :ShadowDenoiser /
  :ReflectionDenoiser — individual effect systems


--------------------------------------------------------------------------------
  FRAME GRAPH SYSTEM  (module: FrameGraph)
--------------------------------------------------------------------------------

The frame graph is a task-based, resource-managed render graph.

Graph::setup()  — all registered passes run their setup() callback; resources
                  are declared (builder.create / builder.need); inactive passes
                  return false and are culled.

Graph::render() — surviving passes run their render() callback in order;
                  resources are in their declared states.

Key types:
  TaskBuilder     — passed to setup(); used to declare resource dependencies.
  FrameContext    — passed to render(); provides get_list(), access to graph
                    contexts, pass->GetPassIndex().
  Pass            — base for all passes; carries pass_index (slot index for
                    multi-instance passes), GetPassIndex().
  Graph::get_context<T>()  — type-keyed store for per-frame shared data;
                              lives for the duration of one graph execution.

Pass registration:
  graph.add_library_pass<T>(setup, render, flags)          — single pass
  graph.add_library_pass<T>(index, setup, render, flags)   — indexed slot
    The indexed overload resolves T::Names[index] as the pass name and stamps
    pass->pass_index = index.

PassFlags:  General | Compute | Required


--------------------------------------------------------------------------------
  SIG FILES AND CODE GENERATION
--------------------------------------------------------------------------------

.sig files define the data contracts for GPU resources, PSOs, slots, and passes.
SIGParser reads them and emits C++ via Jinja2 templates.

Key .sig constructs:

  struct Foo { ... }          — GPU-visible buffer layout; generates slot/table
                                binding code.
  GraphicsPSO / ComputePSO / WorkgraphPSO / RaytracePSO  — pipeline state
                                objects; generates PSOS:: entries and wrappers.
  PassNode Foo { ... }        — declares a render pass and its resource handles.
                                Generates Passes::Foo, Passes::Foo::Context.

PassNode options:
  [Static]       — pass has no pipeline member; setup/render come from
                   PassDefault<Passes::Foo>; pipeline.jinja emits an
                   unconditional PassDefault<> call.
  [Multiple = N] — pass can be instanced up to N times; generates MaxCount,
                   Names[N], setup_funcs[N], render_funcs[N].
  [Static] + [Multiple] — unconditional loop over MaxCount slots using
                           PassDefault<>; setup gates inactive slots via a
                           counter in a graph context.
  [Flags = X]    — sets PassDefault<>::flags (e.g. Compute, Required).

Generated files (autogen/):
  pass/<Name>.h              — Passes::Name struct with Context, handle fields,
                               setup_func / render_func members (if not Static).
  pass/<Pipeline>.pipeline.h — Pipelines::Pipeline class with pass members and
                               add_passes(Graph&).
  pass_defaults.h            — PassDefault<> specialization declarations for
                               every [Static] pass. Bodies are written by hand
                               in separate .ixx files.

Templates (SIGParser/templates/cpp/):
  pipeline.jinja      — generates the pipeline class; handles Static,
                        Multiple, Static+Multiple branches.
  pass_defaults.jinja — generates pass_defaults.h declarations from [Static]
                        passes; the implementation bodies are NOT generated.
  pass.jinja          — generates per-pass structs.
  slot.jinja / layout.jinja / pso.jinja / etc.


--------------------------------------------------------------------------------
  PASSDEFAULT PATTERN
--------------------------------------------------------------------------------

PassDefault<T> is a traits struct declared in pass_defaults.h (auto-generated)
and implemented by hand in partition .ixx files.

  template<> struct PassDefault<Passes::Foo>
  {
      static constexpr bool enabled = true;
      static constexpr FrameGraph::PassFlags flags = ...;
      static bool setup(Passes::Foo::Context&, TaskBuilder&);
      static void render(Passes::Foo::Context&, FrameContext&);
  };

Rules:
  - setup() returns false to cull the pass for this frame.
  - render() is only called if setup() returned true.
  - Both functions access shared data exclusively through graph contexts
    (get_context<T>()); no captured state, no 'this'.
  - The pipeline class calls these directly; no lambda wiring in main.cpp.

Implementation file locations:
  PreSceneSystem.ixx   → PassDefault<Passes::PreScene>
  SceneSystem.ixx      → PassDefault<Passes::Scene>
  RTXPassSystem.ixx    → PassDefault<Passes::RTXPass>
  GUI/Base.cpp         → PassDefault<Passes::UI_Render>
  main.cpp             → PassDefault<Passes::ResultCreation>,
                          PassDefault<Passes::CopyPrev>,
                          PassDefault<Passes::Profiler>
  (FSR, CubeMapDownsample, CubeMapEnviromentProcessor live in their
   respective effect files)


--------------------------------------------------------------------------------
  PER-FRAME GRAPH CONTEXTS
--------------------------------------------------------------------------------

Populated once per frame before graph setup, consumed by any pass:

  ViewportInfo     { ivec2 frame_size; ivec2 upscale_size; }
  TimeInfo         { float time; float totalTime; }
  SkyInfo          { float3 sunDir; }
  SceneInfo        { main_renderer::ptr renderer; Scene::ptr scene; }
  CameraInfo       { camera* cam; }
  UIContext        { vector<draw_info> draw_infos; uint32_t setup_counter;
                     float dt; vec2 scaled_size;
                     Handlers::Texture result_texture_handler; }


--------------------------------------------------------------------------------
  MAIN PIPELINE  (Pipelines::MainPipeline)
--------------------------------------------------------------------------------

Defined in test.sig, generated into MainPipeline.pipeline.h.
Pass execution order (from test.sig comments):

  PreScene                          [Static]
  BlueNoise
  Voxelize
  PSSM_Global, PSSM_Cascade[N]
  CubeSky
  CubeMapDownsample                 [Static]
  CubeMapEnviromentProcessor        [Static]
  Lighting
  Mipmapping
  Scene                             [Static]
  RTXPass                           [Static, Compute]
  ResultCreation                    [Static]
  PSSM_GenerateMask, PSSM_Combine
  VoxelScreen, VoxelCombine
  ScreenReflection
  ReflectionDenoiser_Reproject
  ReflCombine
  VoxelDebug
  Sky
  stencil_renderer_before/after
  SMAA
  FSR                               [Static, Compute]
  CopyPrev                          [Static, Compute]
  Profiler                          [Static, Required]
  UI_Render[16]                     [Static, Multiple=16]


--------------------------------------------------------------------------------
  UI_RENDER PASS  (multi-slot static)
--------------------------------------------------------------------------------

UI_Render is [Static] + [Multiple=16]; up to 16 slots render draw_info batches
in parallel.

Slot assignment uses a deterministic formula:
  per_thread = max(64, (draw_infos.size() + 7) / 8)
  slot k handles draw_infos[ k*per_thread .. min((k+1)*per_thread, size) )

setup() uses UIContext::setup_counter (incremented each call, reset each frame)
to know which slot it is; returns false when slot*per_thread >= size (culls
remaining slots).

render() uses context.pass->GetPassIndex() to recover the slot index without
parsing the pass name.


--------------------------------------------------------------------------------
  RTX / RAY TRACING
--------------------------------------------------------------------------------

RTX class (singleton) wraps the ray tracing PSO (MainRTX) and dispatches
raygen shaders (Shadow, Reflection, Indirect via ShadowPass, ColorPass).

RTXPass (static, compute) runs a work-graph-based screen-space shadow pass
(Bend Screen Space Shadows library). It reads:
  - GBuffer (albedo, normals, depth, specular, speed, prev depth)
  - SceneInfo.scene->raytrace_scene->raytracing_handle
  - SkyInfo.sunDir  (sun direction, normalized)
  - CameraInfo.cam->get_view_proj()

The TLAS (raytrace_scene) is updated each frame in PassDefault<PreScene>::render
via scene.raytrace_scene->update(...), followed by RTX::get().prepare().


--------------------------------------------------------------------------------
  KNOWN AREAS FOR FUTURE IMPROVEMENT
--------------------------------------------------------------------------------

1. pass_defaults.h is auto-generated for declarations but implementation bodies
   must still be written by hand in separate .ixx files — this is intentional
   and by design (the generator cannot know the logic).

2. Several passes still wire setup/render as lambdas in main.cpp capturing
   'this' (triangle_drawer): PSSM, Sky, VoxelGI, SMAA, BlueNoise,
   stencil_renderer, Lighting, Mipmapping, etc. These are candidates for
   [Static] conversion as their required data moves into graph contexts.

3. SceneSystem and PreSceneSystem are exported interface partitions even though
   they export no types — they exist only to provide PassDefault bodies.
   A single aggregating partition could collect all such implementations.

================================================================================
