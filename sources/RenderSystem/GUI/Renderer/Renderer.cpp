module GUI:Renderer;

import HAL;
import RenderSystem;
import Graphics;

import :Skin;

namespace GUI
{
    namespace
    {
        // Mip 0 of `down` is the snapshot, mips 1..levels the downsample chain.
        // `up` mips 0..levels-1 are the upsample chain; mip 0 is what gets drawn.
        constexpr uint32_t max_blur_levels = 6;

        struct BackdropSlot
        {
            int width = 0;
            int height = 0;
            uint32_t levels = 0;
            std::shared_ptr<HAL::Texture> down;
            std::shared_ptr<HAL::Texture> up;
        };

        class BackdropCache
        {
            std::mutex m;
            std::vector<BackdropSlot> slots;

            // A replaced texture may still be referenced by command lists the
            // GPU hasn't finished; keep the last few alive instead of relying
            // on every list holding its resources.
            std::vector<std::shared_ptr<HAL::Texture>> retired;
            static constexpr size_t retire_depth = 24;

            void retire(std::shared_ptr<HAL::Texture>& t)
            {
                if (t)
                    retired.push_back(std::move(t));

                if (retired.size() > retire_depth)
                    retired.erase(retired.begin(), retired.begin() + (retired.size() - retire_depth));
            }

        public:
            static BackdropCache& get()
            {
                static BackdropCache cache;
                return cache;
            }

            BackdropSlot acquire(HAL::Device& device, uint32_t index, int width, int height)
            {
                std::lock_guard<std::mutex> g(m);

                if (slots.size() <= index)
                    slots.resize(index + 1);

                auto& s = slots[index];

                if (!s.down || s.width != width || s.height != height)
                {
                    retire(s.down);
                    retire(s.up);

                    uint32_t max_levels = 0;
                    for (int side = std::min(width, height); side > 1; side >>= 1)
                        max_levels++;

                    s.levels = std::min(max_blur_levels, max_levels);
                    s.width = width;
                    s.height = height;

                    auto make = [&](const char* name)
                    {
                        auto tex = std::make_shared<HAL::Texture>(device,
                            HAL::ResourceDesc::Tex2D(HAL::Format::R16G16B16A16_FLOAT,
                                { (UINT)width, (UINT)height }, 1, s.levels + 1,
                                HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess),
                            HAL::TextureLayout::UNDEFINED);
                        tex->resource->set_name(name);
                        return tex;
                    };

                    s.down = make("ui_backdrop::down");
                    s.up = make("ui_backdrop::up");
                }

                return s;
            }

            void reset()
            {
                std::lock_guard<std::mutex> g(m);
                slots.clear();
                retired.clear();
            }
        };

        int mip_extent(int size, uint32_t mip)
        {
            return std::max(1, size >> mip);
        }

        // One snapshot texture per group-opacity layer slot, same lifetime
        // handling as the blur textures.
        struct LayerSlot
        {
            int width = 0;
            int height = 0;
            std::shared_ptr<HAL::Texture> tex;
        };

        class LayerCache
        {
            std::mutex m;
            std::vector<LayerSlot> slots;
            std::vector<std::shared_ptr<HAL::Texture>> retired;
            static constexpr size_t retire_depth = 24;

        public:
            static LayerCache& get()
            {
                static LayerCache cache;
                return cache;
            }

            LayerSlot acquire(HAL::Device& device, uint32_t index, int width, int height)
            {
                std::lock_guard<std::mutex> g(m);

                if (slots.size() <= index)
                    slots.resize(index + 1);

                auto& s = slots[index];

                if (!s.tex || s.width != width || s.height != height)
                {
                    if (s.tex)
                        retired.push_back(std::move(s.tex));

                    if (retired.size() > retire_depth)
                        retired.erase(retired.begin(), retired.begin() + (retired.size() - retire_depth));

                    s.width = width;
                    s.height = height;
                    s.tex = std::make_shared<HAL::Texture>(device,
                        HAL::ResourceDesc::Tex2D(HAL::Format::R16G16B16A16_FLOAT,
                            { (UINT)width, (UINT)height }, 1, 1, HAL::ResFlags::ShaderResource),
                        HAL::TextureLayout::UNDEFINED);
                    s.tex->resource->set_name("ui_layer::snapshot");
                }

                return s;
            }

            void reset()
            {
                std::lock_guard<std::mutex> g(m);
                slots.clear();
                retired.clear();
            }
        };
    }

    void Renderer::reset_backdrop()
    {
        BackdropCache::get().reset();
        LayerCache::get().reset();
    }

    bool Renderer::begin_layer(base::Context& c, uint32_t slot_index, rect r, float pad_ui)
    {
        if (!c.target.resource || c.window_size.x <= 0 || c.window_size.y <= 0)
            return false;

        PROFILE(L"ui_layer_begin");

        const ivec2 target_size = c.target.get_size();
        const float k = float(target_size.x) / c.window_size.x;
        const float pad = std::ceil(pad_ui * c.scale * k);

        const float2 lt = (float2(r.pos) + c.offset) * k;
        const float2 rb = lt + float2(r.size) * k;

        int left   = std::max(0, int(std::floor(lt.x - pad)));
        int top    = std::max(0, int(std::floor(lt.y - pad)));
        const int right  = std::min(target_size.x, int(std::ceil(rb.x + pad)));
        const int bottom = std::min(target_size.y, int(std::ceil(rb.y + pad)));
        int width  = right - left;
        int height = bottom - top;

        if (width < 2 || height < 2)
            return false;

        // Same 32 px bucketing as the blur snapshots: elements of similar size
        // share a texture size instead of reallocating.
        constexpr int bucket = 32;
        width  = std::min(target_size.x, (width + bucket - 1) / bucket * bucket);
        height = std::min(target_size.y, (height + bucket - 1) / bucket * bucket);
        left = std::min(left, target_size.x - width);
        top  = std::min(top, target_size.y - height);

        // What the group sits on has to be in the target before it is copied.
        nine_patch.flush(c);

        auto& device = RenderSystem::get().device();
        const LayerSlot slot = LayerCache::get().acquire(device, slot_index, width, height);

        c.command_list->get_copy().copy_texture(slot.tex->resource, ivec3(0, 0, 0),
                                                c.target.resource, ivec3(left, top, 0),
                                                ivec3(width, height, 1));

        LayerRecord record;
        record.left = left;
        record.top = top;
        record.width = width;
        record.height = height;
        record.view = slot.tex->texture_2d();
        record.owner = slot.tex;
        layers.push_back(std::move(record));
        return true;
    }

    void Renderer::end_layer(base::Context& c, float opacity)
    {
        if (layers.empty())
            return;

        const LayerRecord record = std::move(layers.back());
        layers.pop_back();

        const float alpha = 1.0f - std::clamp(opacity, 0.0f, 1.0f);
        if (alpha <= 0.0f)
            return;

        PROFILE(L"ui_layer_end");

        const ivec2 target_size = c.target.get_size();
        const float k = float(target_size.x) / c.window_size.x;

        // Window pixels, clipped like the group's own drawing was.
        const float2 quad_lt = float2::max(float2(record.left / k, record.top / k),
                                           float2(float(c.ui_clipping.left), float(c.ui_clipping.top)));
        const float2 quad_rb = float2::min(float2((record.left + record.width) / k, (record.top + record.height) / k),
                                           float2(float(c.ui_clipping.right), float(c.ui_clipping.bottom)));
        if (quad_rb.x <= quad_lt.x || quad_rb.y <= quad_lt.y)
            return;

        // The group is batched; the copy goes over all of it.
        nine_patch.flush(c);

        auto& graphics = c.command_list->get_graphics();

        Slots::UI::UILayerRestore data;
        data.GetSnapshot()    = record.view.texture2D;
        data.GetQuad()        = float4(quad_lt.x, quad_lt.y, quad_rb.x - quad_lt.x, quad_rb.y - quad_lt.y);
        data.GetWindow_size() = float2(c.window_size.x, c.window_size.y);
        data.GetOrigin()      = float2(float(record.left), float(record.top));
        data.GetAlpha()       = alpha;
        graphics.set(data);

        graphics.set_pipeline<PSOS::UI::UILayerRestore>();
        graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
        graphics.draw(4);
    }

    void Renderer::draw_backdrop_blur(base::Context& c, uint32_t slot_index, rect r, const BackdropBlurParams& p)
    {
        // The flat fallback keeps the element readable where there is nothing
        // to snapshot.
        auto flat = [&]
        {
            draw_color(c, float4(p.tint.x, p.tint.y, p.tint.z, 0.85f), r);
        };

        if (!c.target.resource || c.window_size.x <= 0 || c.window_size.y <= 0)
        {
            flat();
            return;
        }

        PROFILE(L"backdrop_blur");

        const ivec2 target_size = c.target.get_size();
        const float k = float(target_size.x) / c.window_size.x;

        // The part of the element that is actually visible.
        const float2 element_lt = float2(r.pos) + c.offset;
        const float2 element_rb = element_lt + float2(r.size);
        const float2 visible_lt = float2::max(element_lt, float2(float(c.ui_clipping.left), float(c.ui_clipping.top)));
        const float2 visible_rb = float2::min(element_rb, float2(float(c.ui_clipping.right), float(c.ui_clipping.bottom)));

        if (visible_rb.x <= visible_lt.x || visible_rb.y <= visible_lt.y)
            return;

        const float radius_px = p.radius * c.scale * k;
        const float pad = std::ceil(radius_px * 1.5f) + 2.0f;

        // Snapshot region in target pixels: the visible rect plus what the
        // blur reaches in from outside it.
        int region_left   = std::max(0, int(std::floor(visible_lt.x * k - pad)));
        int region_top    = std::max(0, int(std::floor(visible_lt.y * k - pad)));
        const int region_right  = std::min(target_size.x, int(std::ceil(visible_rb.x * k + pad)));
        const int region_bottom = std::min(target_size.y, int(std::ceil(visible_rb.y * k + pad)));
        int region_w = region_right - region_left;
        int region_h = region_bottom - region_top;

        if (region_w < 2 || region_h < 2)
        {
            flat();
            return;
        }

        // Round the snapshot up to a multiple of 32 so elements of similar size
        // (and an element being resized) share a texture size instead of
        // reallocating every frame. The extra texels are real target pixels:
        // the region grows right/down, and slides left/up at the target's edge.
        constexpr int bucket = 32;
        region_w = std::min(target_size.x, (region_w + bucket - 1) / bucket * bucket);
        region_h = std::min(target_size.y, (region_h + bucket - 1) / bucket * bucket);
        region_left = std::min(region_left, target_size.x - region_w);
        region_top  = std::min(region_top, target_size.y - region_h);

        // Everything batched so far has to land in the target before it is read.
        nine_patch.flush(c);

        auto& device = RenderSystem::get().device();
        const BackdropSlot slot = BackdropCache::get().acquire(device, slot_index, region_w, region_h);

        auto& list = *c.command_list;

        {
            PROFILE(L"backdrop_copy");
            list.get_copy().copy_texture(slot.down->resource, ivec3(0, 0, 0),
                                         c.target.resource, ivec3(region_left, region_top, 0),
                                         ivec3(region_w, region_h, 1));
        }

        // Whole octaves from the chain depth, the remainder from the sample
        // offset (a Dual Kawase pass at depth N spreads about 2^N * offset texels).
        uint32_t levels = 0;
        float offset = 1.0f;

        // Small radii skip the chain: the composite pass filters the snapshot
        // itself (BackdropDraw::Direct), which saves two compute dispatches and
        // the mip views and UAV transitions around them for every element.
        constexpr float direct_radius_max = 8.0f;
        const bool direct = radius_px >= 0.5f && radius_px <= direct_radius_max;

        if (!direct && radius_px >= 0.5f && slot.levels > 0)
        {
            levels = (uint32_t)std::clamp((int)std::ceil(std::log2(std::max(radius_px, 1.0f) / 2.0f)), 1, (int)slot.levels);
            offset = std::clamp(radius_px / (float(1u << levels) * 1.2f), 0.5f, 3.0f);
        }

        auto down_mip = [&](uint32_t mip) { return slot.down->texture_2d().create_mip(mip, list); };
        auto up_mip   = [&](uint32_t mip) { return slot.up->texture_2d().create_mip(mip, list); };

        HAL::Texture2DView blurred;

        if (levels == 0)
        {
            blurred = down_mip(0);
        }
        else
        {
            auto& compute = list.get_compute();
            compute.set_signature(Layouts::DefaultLayout);

            auto step = [&](const HAL::Texture2DView& src, int src_w, int src_h,
                            const HAL::Texture2DView& dst, int dst_w, int dst_h, bool up)
            {
                compute.set_pipeline<PSOS::UI::BackdropBlur>(PSOS::UI::BackdropBlur::Up.Use(up));

                Slots::UI::BackdropBlur data;
                data.GetSrc() = src.texture2D;
                data.GetDst() = dst.rwTexture2D;
                data.GetDst_size() = uint2(dst_w, dst_h);
                data.GetSrc_texel() = float2(1.0f / src_w, 1.0f / src_h);
                data.GetOffset() = offset;
                compute.set(data);

                compute.dispatch(ivec2(dst_w, dst_h), ivec2(8, 8));
            };

            {
                PROFILE(L"backdrop_blur_down");
                for (uint32_t m = 0; m < levels; m++)
                    step(down_mip(m), mip_extent(region_w, m), mip_extent(region_h, m),
                         down_mip(m + 1), mip_extent(region_w, m + 1), mip_extent(region_h, m + 1), false);
            }

            {
                PROFILE(L"backdrop_blur_up");
                step(down_mip(levels), mip_extent(region_w, levels), mip_extent(region_h, levels),
                     up_mip(levels - 1), mip_extent(region_w, levels - 1), mip_extent(region_h, levels - 1), true);

                for (int m = (int)levels - 2; m >= 0; m--)
                    step(up_mip(m + 1), mip_extent(region_w, m + 1), mip_extent(region_h, m + 1),
                         up_mip(m), mip_extent(region_w, m), mip_extent(region_h, m), true);
            }

            blurred = up_mip(0);
        }

        auto& graphics = list.get_graphics();

        {
            PROFILE(L"backdrop_rebind");
            RT::Frame::SingleColor rt;
            rt.GetColor() = c.target.renderTarget;
            graphics.set_rtv(rt, HAL::RTOptions::Default);
            graphics.set_scissors(c.scissors);

            Slots::UI::DisplayOutput output;
            output.GetUi_scale() = c.ui_scale;
            graphics.set(output);
        }

        {
            PROFILE(L"backdrop_composite");
            Slots::UI::BackdropDraw data;
            data.GetBlurred() = blurred.texture2D;
            data.GetRect() = float4(visible_lt.x, visible_lt.y, visible_rb.x - visible_lt.x, visible_rb.y - visible_lt.y);
            data.GetMap() = float4(k / region_w, k / region_h, -float(region_left) / region_w, -float(region_top) / region_h);
            data.GetWindow_size() = float2(c.window_size.x, c.window_size.y);
            data.GetTint() = p.tint;
            data.GetSaturation() = p.saturation;
            data.GetLuminosity() = p.luminosity;
            data.GetNoise() = p.noise;
            // Binomial taps: sigma = one spacing, and the radius is about two sigma.
            data.GetDirect() = float4(1.0f / region_w, 1.0f / region_h, std::max(radius_px * 0.5f, 0.25f), 0.0f);
            graphics.set(data);

            graphics.set_pipeline<PSOS::UI::BackdropDraw>(PSOS::UI::BackdropDraw::Direct.Use(direct));
            graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
            graphics.draw(4);
        }
    }


    void Renderer::draw_shadow(base::Context& c, rect r, const ShadowStyle& s)
    {
        if (s.color.w <= 0.0f || c.window_size.x <= 0 || c.window_size.y <= 0)
            return;

        PROFILE(L"ui_shadow");

        // Same space as the backdrop composite: render bounds + draw offset,
        // in window pixels; style sizes are UI pixels at scale 1.
        const float k      = c.scale;
        const float sigma  = std::max(s.blur * k, 0.5f);
        const float spread = s.spread * k;
        const float2 offset = s.offset * k;

        const float4 element = float4(r.x + c.offset.x, r.y + c.offset.y, r.w, r.h);
        const float4 shape   = float4(element.x + offset.x - spread, element.y + offset.y - spread,
                                      element.z + 2 * spread, element.w + 2 * spread);
        // 3 sigma covers >99.8% of the falloff.
        const float reach = 3.0f * sigma + 1.0f;

        // Clipped to the parent's clip rect like the backdrop composite: the
        // shadow reaches outside the element, and the scissor alone can be
        // wider than the parent (it would spill over neighbouring UI).
        const float2 quad_lt = float2::max(float2(shape.x - reach, shape.y - reach),
                                           float2(float(c.ui_clipping.left), float(c.ui_clipping.top)));
        const float2 quad_rb = float2::min(float2(shape.x + shape.z + reach, shape.y + shape.w + reach),
                                           float2(float(c.ui_clipping.right), float(c.ui_clipping.bottom)));
        if (quad_rb.x <= quad_lt.x || quad_rb.y <= quad_lt.y)
            return;
        const float4 quad = float4(quad_lt.x, quad_lt.y, quad_rb.x - quad_lt.x, quad_rb.y - quad_lt.y);

        // Everything batched so far goes under the shadow.
        nine_patch.flush(c);

        auto& graphics = c.command_list->get_graphics();

        Slots::UI::UIShadow data;
        data.GetQuad()        = quad;
        data.GetElement()     = element;
        data.GetShape()       = shape;
        data.GetColor()       = s.color;
        data.GetWindow_size() = float2(c.window_size.x, c.window_size.y);
        data.GetRadius()      = s.radius * k;
        data.GetSigma()       = sigma;
        graphics.set(data);

        graphics.set_pipeline<PSOS::UI::UIShadow>();
        graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
        graphics.draw(4);
    }

    void Renderer::draw_fx(base::Context& c, rect r, const FxParams& p)
    {
        const bool shine = p.shine_phase >= 0.0f && p.shine_strength > 0.0f;
        const bool ripple = p.ripple_phase >= 0.0f && p.ripple_strength > 0.0f;
        if ((p.reveal_strength <= 0.0f && p.border_strength <= 0.0f && !shine && !ripple
             && p.focus_strength <= 0.0f && p.inner_strength <= 0.0f)
            || c.window_size.x <= 0 || c.window_size.y <= 0)
            return;

        PROFILE(L"ui_fx");

        const float k = c.scale;
        const float4 shape = float4(r.x + c.offset.x, r.y + c.offset.y, r.w, r.h);

        // Clipped to the parent like the shadow: the scissor alone can be wider.
        const float2 quad_lt = float2::max(float2(shape.x, shape.y),
                                           float2(float(c.ui_clipping.left), float(c.ui_clipping.top)));
        const float2 quad_rb = float2::min(float2(shape.x + shape.z, shape.y + shape.w),
                                           float2(float(c.ui_clipping.right), float(c.ui_clipping.bottom)));
        if (quad_rb.x <= quad_lt.x || quad_rb.y <= quad_lt.y)
            return;

        // The element's own fill is batched; the light goes over it.
        nine_patch.flush(c);

        auto& graphics = c.command_list->get_graphics();

        Slots::UI::UIFx data;
        data.GetQuad()        = float4(quad_lt.x, quad_lt.y, quad_rb.x - quad_lt.x, quad_rb.y - quad_lt.y);
        data.GetShape()       = shape;
        data.GetReveal()      = float4(p.cursor.x, p.cursor.y, p.reveal_size * k, p.reveal_strength);
        data.GetBorder()      = float4(p.border_size * k, p.border_strength, p.border_width * k, 0.0f);
        data.GetShine()       = float4(shine ? p.shine_phase : -1.0f, p.shine_width, p.shine_strength, 0.0f);
        data.GetRipple()      = float4(p.ripple_origin.x, p.ripple_origin.y, ripple ? p.ripple_phase : -1.0f, p.ripple_strength);
        data.GetFocus()       = float4(p.focus_width * k, p.focus_strength, 0.0f, 0.0f);
        data.GetFocus_color() = float4(p.focus_color.x, p.focus_color.y, p.focus_color.z, 1.0f);
        data.GetInner()       = float4(p.inner_size * k, p.inner_strength, p.inner_offset * k, 0.0f);
        data.GetLight()       = float4(p.light_color.x, p.light_color.y, p.light_color.z, 1.0f);
        data.GetWindow_size() = float2(c.window_size.x, c.window_size.y);
        data.GetRadius()      = p.radius * k;
        graphics.set(data);

        graphics.set_pipeline<PSOS::UI::UIFx>();
        graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
        graphics.draw(4);
    }

    void Renderer::draw_area(base::ptr obj, base::Context& c)
    {
        draw(c, area_tex, obj->get_render_bounds());
    }

    void Renderer::draw_container(base::ptr obj, base::Context& c)
    {
        draw(c, container_tex, obj->get_render_bounds());
    }


    void Renderer::draw_virtual(base::ptr obj, base::Context& c)
    {
        draw(c, virtual_tex, obj->get_render_bounds());
    }


    void Renderer::draw_color(base::Context& c, float4 color, rect r)
    {
        nine_patch.draw_color(c, color, color, r);
    }

    void Renderer::draw_color(base::Context& c, float4 left, float4 right, rect r)
    {
        nine_patch.draw_color(c, left, right, r);
    }

    void Renderer::start()
    {
        nine_patch.counter = 0;
    }

    void Renderer::set(base::Context& c)
    {
    }
	void Renderer::flush(base::Context& c)
	{
		nine_patch.flush(c);
	}
    Renderer::Renderer()
    {
        area_tex = Skin::get().Edit;
        virtual_tex = Skin::get().Virtual;
        container_tex = Skin::get().Background;
    }

    void Renderer::draw(base::Context& c, HAL::PipelineState::ptr state, rect r)
    {
        nine_patch.draw(c, state, r);
    }

    void Renderer::draw(base::Context& c, GUI::Texture& item, rect r)
    {
        nine_patch.draw(c, item, r);
    }

}
