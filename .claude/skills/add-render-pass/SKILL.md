---
name: add-render-pass
description: Add a new render or compute pass to the FrameGraph end to end — the .sig PassNode and PSO declaration, code generation, the setup/render implementation, the HLSL shader, and pipeline registration. Use this skill whenever adding a new rendering effect, post-process, compute dispatch, shadow, or GBuffer stage, whenever a new PassNode or ComputePSO/GraphicsPSO is needed, or when an existing pass needs new resource reads/writes wired through the FrameGraph. Also use it when a newly added pass never executes, since that is usually a pipeline registration or setup-return problem rather than a bug in the render body.
---

# Adding a FrameGraph pass

A pass is declared in a `.sig` file and implemented in C++, with the generator
producing the glue between them. Getting the declaration right matters more
than the render body — the declaration is what the FrameGraph uses to schedule
the pass and to compute its barriers, so a wrong read/write flag produces
validation errors or corrupt results that look like shader bugs.

Read `sources/RenderSystem/Effects/Sky.cpp` alongside `sources/SIGParser/sigs/sky.sig`
before starting. Between them they show both wiring styles, a graphics PSO and
several compute PSOs, resource creation, and per-mip view handling — it is the
best single reference in the tree.

## 1. Declare in a `.sig` file

Put the declaration in an existing `.sig` that matches the subsystem, or a new
one in `sources/SIGParser/sigs/`.

**Binding struct** — the shader-visible parameters. `[Bind = DefaultLayout::InstanceN]`
selects the root-signature slot; distinct structs bound in the same pass need
distinct instance slots:

```
[Bind = DefaultLayout::Instance0]
struct MyEffectData
{
    float4 params;
    Texture2D<float> depthBuffer;
    RWTexture2D<float4> result;
}
```

**PSO** — `compute = <name>` refers to `workdir/shaders/<name>.hlsl`, and
`[EntryPoint = X]` selects the function within it. One shader file can back
several PSOs through different entry points:

```
ComputePSO MyEffectCompute
{
    root = DefaultLayout;

    [EntryPoint = CS]
    compute = my_effect;
}
```

**PassNode** — the FrameGraph declaration. Each member is a resource the pass
touches; `[Write]` marks the ones it produces:

```
[Compute]
PassNode MyEffect
{
    Texture GBuffer_Depth;
    [Write] Texture ResultTexture;
}
```

Resource names are matched **across passes by name** — that is how the graph
links a producer to its consumers. Use the existing name for an existing
resource; introducing a new spelling silently creates an unrelated resource.

### PassNode flags, and the one that decides your implementation style

- `[Compute]` — may run on the async compute queue. Whether it actually does is
  decided by `[Async]` at the pipeline registration site, not here.
- `[Required]` — never culled, even if nothing consumes its output.
- `[Static]` — **this flag decides how you implement the pass.** It generates a
  `PassDefault<Passes::X>` specialization declaring `setup` and `render`, which
  you then define out-of-line. Without `[Static]` no such specialization exists
  and the pass must be wired at runtime by assigning `setup_func`/`render_func`.
  See `sources/SIGParser/templates/cpp/pass_defaults.jinja` for the exact rule.

Choose `[Static]` when the pass is self-contained. Choose runtime wiring when
the pass needs state owned by a C++ object — loaded textures, cached history
buffers, persistent settings.

## 2. Regenerate

Use the `sig-regen` skill. In short: run the generator from `sources/SIGParser`,
then run `generate_project.bat` because a new `PassNode` and PSO create new
files that the projects don't yet list.

## 3. Implement setup and render

**setup** declares resource intent and returns whether the pass should run this
frame. Returning `false` culls it — a pass that never executes is usually a
`setup` returning `false`, not a broken render body.

```cpp
bool PassDefault<Passes::MyEffect>::setup(
    Passes::MyEffect::Context& data, TaskBuilder& builder)
{
    builder.need(data.GBuffer_Depth, ResourceFlags::ComputeRead);
    builder.need(data.ResultTexture, ResourceFlags::UnorderedAccess);
    return true;
}
```

- `builder.need(...)` declares use of a resource that already exists.
- `builder.create(handle, desc, flags)` declares a resource this pass produces.
- `ResourceFlags` are in `sources/RenderSystem/FrameGraph/FrameGraph.Base.ixx`:
  `PixelRead`, `ComputeRead`, `DSRead`, `UnorderedAccess`, `RenderTarget`,
  `DepthStencil`, `CopyDest`, `CopySource`, plus `Static` (persists across
  frames rather than being transient) and `Required`.

The flags must match what the pass actually does. They are the input to barrier
computation, so declaring `ComputeRead` on something the shader writes produces
a validation error whose message points at the barrier, not at the declaration.

**render** records the work:

```cpp
void PassDefault<Passes::MyEffect>::render(
    Passes::MyEffect::Context& data, FrameContext& context)
{
    auto& compute = context.get_list()->get_compute();

    context.graph->set_slot(SlotID::FrameInfo, compute);

    {
        Slots::MyEffectData params;
        params.GetDepthBuffer() = data.GBuffer_Depth->texture2D;
        params.GetResult()      = data.ResultTexture->rwTexture2D;
        compute.set(params);
    }

    compute.set_pipeline<PSOS::MyEffectCompute>();
    compute.dispatch(context.graph->get_context<ViewportInfo>().frame_size, ivec2{ 16, 16 });
}
```

Add `PROFILE(L"my_effect_dispatch")` scopes around logically distinct phases —
setup versus dispatch versus per-mip loops — per the profiling convention in
`CLAUDE.md`.

### A binding trap worth knowing

`builder.need()` wires the owning pass's own `data.X` FrameGraph handle;
`pass_texture()` does not. Using a `pass_texture()`-declared handle through
`data.X` inside that same pass's `render()` dereferences null and crashes with
no D3D12 output at all. In that pass, use the owning `HAL::Texture` directly.

## 4. Write the shader

Create `workdir/shaders/<name>.hlsl` matching the PSO's `compute =`/`vertex =`/
`pixel =` value, with a function named by `[EntryPoint = ...]`. Include the
generated binding header so the struct layout stays in sync with the `.sig`.

## 5. Register in a pipeline

A declared pass does nothing until it appears in a `Pipeline` block. Order in
the block is execution order; `[Async]` before an entry lets it overlap on the
compute queue:

```
Pipeline AssetPipeline
{
    ...
    [Async]
    MyEffect;
    ...
}
```

Add it to every pipeline that should run it — `AssetPipeline` and the pipeline
in `test.sig` are separate graphs and adding to one does not affect the other.
Then regenerate again, since the pipeline block changed.

## 6. Verify

Use the `build-and-validate` skill. Confirm both that the pass runs and that it
introduced no new D3D12 errors — a new pass with mismatched resource flags
typically shows up as a per-frame repeating barrier error rather than as
anything visible on screen.
