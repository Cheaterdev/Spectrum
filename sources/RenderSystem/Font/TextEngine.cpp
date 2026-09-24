module TextEngine;

import <windows/windows.h>;

import RenderSystem;
import Graphics;
import skribidi;

namespace Text
{
    namespace
    {
        // The atlas is created at its maximum size: a Skribidi texture that grows
        // changes dimensions, which in D3D12 means a new resource and a stale
        // descriptor in every layout built before the growth. Full textures are
        // handled by Skribidi adding another one instead.
        constexpr int32_t ATLAS_SIZE = 2048;

        struct FontFile
        {
            const char* file;
            uint8_t     family;
        };

        // Segoe UI covers Latin, Greek, Cyrillic, Arabic and Hebrew; the rest fall back.
        // Weights are picked by Skribidi from each file's own metadata.
        constexpr FontFile system_fonts[] = {
            { "segoeuil.ttf", SKB_FONT_FAMILY_DEFAULT },
            { "segoeui.ttf",  SKB_FONT_FAMILY_DEFAULT },
            { "segoeuib.ttf", SKB_FONT_FAMILY_DEFAULT },
            { "seguisym.ttf", SKB_FONT_FAMILY_DEFAULT },
            { "seguiemj.ttf", SKB_FONT_FAMILY_EMOJI },
        };

        skb_weight_t to_skb(Weight weight)
        {
            switch (weight)
            {
            case Weight::Light: return SKB_WEIGHT_LIGHT;
            case Weight::Bold:  return SKB_WEIGHT_BOLD;
            default:            return SKB_WEIGHT_NORMAL;
            }
        }
    }

    struct Engine::Impl
    {
        struct AtlasTexture
        {
            HAL::Texture::ptr texture;
            bool              uploaded = false;
        };

        std::mutex m;

        skb_font_collection_t* fonts      = nullptr;
        skb_rasterizer_t*      rasterizer = nullptr;
        skb_image_atlas_t*     atlas      = nullptr;
        skb_layout_cache_t*    layouts    = nullptr;
        skb_temp_alloc_t*      temp       = nullptr;

        std::vector<AtlasTexture> textures;
        bool pending_upload = false;

        // Caller holds m.
        const skb_layout_t* get_layout(std::string_view utf8, Style style)
        {
            PROFILE(L"text_layout");
            const skb_attribute_t attributes[] = {
                skb_attribute_make_font_size(style.size),
                skb_attribute_make_font_weight(to_skb(style.weight)),
            };

            skb_attribute_set_t attribute_set = {};
            attribute_set.attributes       = attributes;
            attribute_set.attributes_count = (int32_t)std::size(attributes);

            skb_layout_params_t params = {};
            params.font_collection = fonts;

            return skb_layout_cache_get_utf8(layouts, temp, &params, utf8.data(), (int32_t)utf8.size(), attribute_set);
        }

        // Called by Skribidi from inside atlas requests, so already under m.
        static void on_create_texture(skb_image_atlas_t* atlas, uint8_t texture_idx, void* context)
        {
            auto self = static_cast<Impl*>(context);
            const skb_image_t* image = skb_image_atlas_get_texture(atlas, texture_idx);
            if (!image) return;

            const auto format = image->bpp == 4 ? HAL::Format::R8G8B8A8_UNORM : HAL::Format::R8_UNORM;
            HAL::ResourceDesc desc = HAL::ResourceDesc::Tex2D(format, { image->width, image->height }, 1, 1);

            if (self->textures.size() <= texture_idx)
                self->textures.resize(texture_idx + 1);

            auto& t = self->textures[texture_idx];
            t.texture.reset(new HAL::Texture(RenderSystem::get().device(), desc));
            t.texture->resource->set_name("Text::Engine atlas " + std::to_string(texture_idx));
            t.uploaded = false;
        }
    };

    Engine::Engine() : impl(std::make_unique<Impl>())
    {
        impl->fonts = skb_font_collection_create();

        char windows_dir[MAX_PATH] = {};
        GetWindowsDirectoryA(windows_dir, MAX_PATH);
        const std::string font_dir = std::string(windows_dir) + "\\Fonts\\";

        for (auto& f : system_fonts)
        {
            const std::string path = font_dir + f.file;
            if (!skb_font_collection_add_font(impl->fonts, path.c_str(), f.family, nullptr))
                Log::get() << Log::LEVEL_WARNING << "Text::Engine: failed to load font " << path << Log::endl;
        }

        skb_rasterizer_config_t raster_config = skb_rasterizer_get_default_config();
        impl->rasterizer = skb_rasterizer_create(&raster_config);

        skb_image_atlas_config_t atlas_config = skb_image_atlas_get_default_config();
        atlas_config.init_width  = atlas_config.max_width  = ATLAS_SIZE;
        atlas_config.init_height = atlas_config.max_height = ATLAS_SIZE;
        impl->atlas = skb_image_atlas_create(&atlas_config);
        skb_image_atlas_set_create_texture_callback(impl->atlas, &Impl::on_create_texture, impl.get());

        impl->layouts = skb_layout_cache_create();
        impl->temp    = skb_temp_alloc_create(512 * 1024);
    }

    Engine::~Engine()
    {
        skb_temp_alloc_destroy(impl->temp);
        skb_layout_cache_destroy(impl->layouts);
        skb_image_atlas_destroy(impl->atlas);
        skb_rasterizer_destroy(impl->rasterizer);
        skb_font_collection_destroy(impl->fonts);
    }

    void Engine::begin_frame()
    {
        PROFILE(L"text_begin_frame");
        std::lock_guard<std::mutex> lock(impl->m);
        skb_image_atlas_compact(impl->atlas);
        skb_layout_cache_compact(impl->layouts);
    }

    vec2 Engine::measure(std::string_view utf8, Style style)
    {
        std::lock_guard<std::mutex> lock(impl->m);
        const skb_layout_t* layout = impl->get_layout(utf8, style);
        if (!layout) return {};

        const skb_rect2_t bounds = skb_layout_get_bounds(layout);
        return vec2(bounds.width, bounds.height);
    }

    Caret Engine::caret(std::string_view utf8, Style style, uint32_t offset)
    {
        std::lock_guard<std::mutex> lock(impl->m);
        const skb_layout_t* layout = impl->get_layout(utf8, style);
        if (!layout) return {};

        const skb_rect2_t bounds = skb_layout_get_bounds(layout);
        const skb_caret_info_t info = skb_layout_get_caret_info_at(layout, skb_text_position_t{ (int32_t)offset, SKB_AFFINITY_TRAILING });

        // ascender is negative (above the baseline), descender positive.
        Caret caret;
        caret.center = vec2(info.x - bounds.x, info.y + (info.ascender + info.descender) * 0.5f - bounds.y);
        caret.height = info.descender - info.ascender;
        return caret;
    }

    uint32_t Engine::hit_test(std::string_view utf8, Style style, vec2 at)
    {
        std::lock_guard<std::mutex> lock(impl->m);
        const skb_layout_t* layout = impl->get_layout(utf8, style);
        if (!layout) return 0;

        const skb_rect2_t bounds = skb_layout_get_bounds(layout);
        const skb_text_position_t pos = skb_layout_hit_test(layout, SKB_MOVEMENT_CARET, at.x + bounds.x, at.y + bounds.y);
        return (uint32_t)std::max(skb_layout_get_offset_from_text_position(layout, pos), 0);
    }

    void Engine::build(std::string_view utf8, Style style, Layout& out)
    {
        std::lock_guard<std::mutex> lock(impl->m);

        out.quads.clear();
        out.size = {};

        const skb_layout_t* layout = impl->get_layout(utf8, style);
        if (!layout) return;

        {
            PROFILE(L"text_request_glyphs");
            const skb_rect2_t bounds = skb_layout_get_bounds(layout);
            out.size = vec2(bounds.width, bounds.height);

            const skb_layout_line_t* lines  = skb_layout_get_lines(layout);
            const skb_layout_run_t*  runs   = skb_layout_get_layout_runs(layout);
            const skb_glyph_t*       glyphs = skb_layout_get_glyphs(layout);

            for (int32_t li = 0; li < skb_layout_get_lines_count(layout); li++)
            {
                const auto& line = lines[li];
                for (int32_t ri = line.layout_run_range.start; ri < line.layout_run_range.end; ri++)
                {
                    const auto& run = runs[ri];
                    if (run.type != SKB_CONTENT_RUN_UTF8 && run.type != SKB_CONTENT_RUN_UTF32)
                        continue;

                    for (int32_t gi = run.glyph_range.start; gi < run.glyph_range.end; gi++)
                    {
                        const auto& glyph = glyphs[gi];
                        const skb_quad_t q = skb_image_atlas_get_glyph_quad(impl->atlas,
                            glyph.offset_x - bounds.x, glyph.offset_y - bounds.y, 1.0f,
                            impl->fonts, run.font_handle, glyph.gid, run.font_size,
                            skb_color_t{ 255, 255, 255, 255 }, SKB_RASTERIZE_ALPHA_MASK);

                        if (q.flags & SKB_QUAD_IS_EMPTY)
                            continue;

                        const skb_image_t* image = skb_image_atlas_get_texture(impl->atlas, q.texture_idx);
                        if (!image) continue;

                        const float w = (float)image->width;
                        const float h = (float)image->height;

                        // Glyphs placed at subpixel pen positions: snap the ones drawn
                        // 1:1 with their bitmap to whole pixels so they sample texel
                        // centers. Scaled ones (non-integer font size) stay fractional
                        // and rely on bilinear filtering instead.
                        float x0 = q.geom.x, y0 = q.geom.y;
                        if (std::abs(q.geom.width - q.texture.width) < 0.01f && std::abs(q.geom.height - q.texture.height) < 0.01f)
                        {
                            x0 = std::round(x0);
                            y0 = std::round(y0);
                        }

                        Quad quad;
                        quad.rect     = float4(x0, y0, x0 + q.geom.width, y0 + q.geom.height);
                        quad.uv       = float4(q.texture.x / w, q.texture.y / h, (q.texture.x + q.texture.width) / w, (q.texture.y + q.texture.height) / h);
                        quad.atlas    = q.texture_idx;
                        quad.is_color = (q.flags & SKB_QUAD_IS_COLOR) != 0;
                        out.quads.push_back(quad);
                    }
                }
            }
        }

        {
            PROFILE(L"text_rasterize");
            if (skb_image_atlas_rasterize_missing_items(impl->atlas, impl->temp, impl->rasterizer))
                impl->pending_upload = true;
        }
    }

    bool Engine::has_pending_upload()
    {
        std::lock_guard<std::mutex> lock(impl->m);
        return impl->pending_upload;
    }

    void Engine::upload(HAL::CommandList::ptr& list)
    {
        PROFILE(L"text_atlas_upload");
        std::lock_guard<std::mutex> lock(impl->m);
        if (!impl->pending_upload) return;

        for (int32_t i = 0; i < skb_image_atlas_get_texture_count(impl->atlas); i++)
        {
            const skb_image_t* image = skb_image_atlas_get_texture(impl->atlas, i);
            auto& t = impl->textures[i];

            skb_rect2i_t dirty = skb_image_atlas_get_and_reset_texture_dirty_bounds(impl->atlas, i);

            // A new GPU texture is undefined outside what gets written; upload the
            // whole CPU image once so a quad edge can never sample garbage.
            if (!t.uploaded)
            {
                dirty = { 0, 0, image->width, image->height };
                t.uploaded = true;
            }
            else if (dirty.width <= 0 || dirty.height <= 0)
                continue;

            const auto* src = image->buffer + dirty.y * image->stride_bytes + dirty.x * image->bpp;
            list->get_copy().update_texture(t.texture->resource,
                ivec3(dirty.x, dirty.y, 0), ivec3(dirty.width, dirty.height, 1),
                HAL::calc_subresource(0, 0, 0, 1, 1),
                reinterpret_cast<const char*>(src), image->stride_bytes);
        }

        impl->pending_upload = false;
    }

    void Engine::draw(HAL::CommandList::ptr& list, const Layout& layout, vec2 pos, float4 color, sizer clip, vec2 window_size)
    {
        if (layout.quads.empty()) return;

        std::vector<HLSL::Texture2D<float4>> atlas_handles;
        {
            std::lock_guard<std::mutex> lock(impl->m);
            atlas_handles.reserve(impl->textures.size());
            for (auto& t : impl->textures)
                atlas_handles.emplace_back(t.texture->texture_2d().texture2D);
        }

        std::vector<Table::UI::Text::GlyphQuad> gpu_quads;
        {
            PROFILE(L"text_build_quads");
            gpu_quads.reserve(layout.quads.size());

            // Atlas glyphs are rasterized for whole-pixel placement; point sampling
            // at a fractional offset would drop or double texel columns.
            pos = vec2(std::round(pos.x), std::round(pos.y));

            auto to_ndc_x = [&](float x) { return 2.0f * x / window_size.x - 1.0f; };
            auto to_ndc_y = [&](float y) { return 1.0f - 2.0f * y / window_size.y; };

            for (auto& q : layout.quads)
            {
                float x0 = q.rect.x + pos.x, y0 = q.rect.y + pos.y;
                float x1 = q.rect.z + pos.x, y1 = q.rect.w + pos.y;
                float u0 = q.uv.x, v0 = q.uv.y, u1 = q.uv.z, v1 = q.uv.w;

                if (x1 <= clip.left || x0 >= clip.right || y1 <= clip.top || y0 >= clip.bottom)
                    continue;

                // Clip the quad and move its uvs by the same fraction.
                const float du = (u1 - u0) / (x1 - x0);
                const float dv = (v1 - v0) / (y1 - y0);
                if (x0 < clip.left)   { u0 += (clip.left - x0) * du;   x0 = clip.left; }
                if (x1 > clip.right)  { u1 -= (x1 - clip.right) * du;  x1 = clip.right; }
                if (y0 < clip.top)    { v0 += (clip.top - y0) * dv;    y0 = clip.top; }
                if (y1 > clip.bottom) { v1 -= (y1 - clip.bottom) * dv; y1 = clip.bottom; }

                Table::UI::Text::GlyphQuad g;
                g.pos      = float4(to_ndc_x(x0), to_ndc_y(y0), to_ndc_x(x1), to_ndc_y(y1));
                g.uv       = float4(u0, v0, u1, v1);
                g.color    = color;
                g.atlas    = q.atlas;
                g.is_color = q.is_color ? 1 : 0;
                gpu_quads.push_back(g);
            }
        }
        if (gpu_quads.empty()) return;

        {
            PROFILE(L"text_draw");
            auto& graphics = list->get_graphics();
            graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
            graphics.set_pipeline<PSOS::UI::Text::GlyphRender>();

            // Same alignment rule as the old font path: the struct stride and
            // Vulkan's minStorageBufferOffsetAlignment must both hold.
            using GlyphQuad = Table::UI::Text::GlyphQuad;
            const uint32_t align = std::lcm<uint32_t>(sizeof(GlyphQuad), RenderSystem::get().device().get_properties().min_storage_buffer_offset_alignment);
            auto data = list->place_data(sizeof(GlyphQuad) * gpu_quads.size(), align);
            list->write<GlyphQuad>(data, gpu_quads);

            auto view = data.resource->create_view<HAL::StructuredBufferView<GlyphQuad>>(*list,
                HAL::StructuredBufferViewDesc{ (UINT)data.resource_offset, (UINT)data.size, HAL::counterType::NONE });

            Slots::UI::Text::GlyphRender slot;
            slot.GetQuads()    = view;
            slot.GetTextures() = atlas_handles;
            graphics.set(slot);

            graphics.draw(UINT(gpu_quads.size() * 6), 0);
        }
    }
}
