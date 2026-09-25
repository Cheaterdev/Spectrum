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
            { "seguili.ttf",  SKB_FONT_FAMILY_DEFAULT },
            { "segoeui.ttf",  SKB_FONT_FAMILY_DEFAULT },
            { "segoeuii.ttf", SKB_FONT_FAMILY_DEFAULT },
            { "segoeuib.ttf", SKB_FONT_FAMILY_DEFAULT },
            { "segoeuiz.ttf", SKB_FONT_FAMILY_DEFAULT },
            { "seguisym.ttf", SKB_FONT_FAMILY_DEFAULT },
            { "consola.ttf",  SKB_FONT_FAMILY_MONOSPACE },
            { "consolai.ttf", SKB_FONT_FAMILY_MONOSPACE },
            { "consolab.ttf", SKB_FONT_FAMILY_MONOSPACE },
            { "consolaz.ttf", SKB_FONT_FAMILY_MONOSPACE },
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

        uint8_t to_skb(Family family)
        {
            return family == Family::Mono ? SKB_FONT_FAMILY_MONOSPACE : SKB_FONT_FAMILY_DEFAULT;
        }

        float4 unpack_rgba8(uint32_t c)
        {
            return float4(float(c & 0xFF), float((c >> 8) & 0xFF), float((c >> 16) & 0xFF), float(c >> 24)) / 255.0f;
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
                skb_attribute_make_font_family(to_skb(style.family)),
            };

            skb_attribute_set_t attribute_set = {};
            attribute_set.attributes       = attributes;
            attribute_set.attributes_count = (int32_t)std::size(attributes);

            skb_layout_params_t params = {};
            params.font_collection = fonts;

            return skb_layout_cache_get_utf8(layouts, temp, &params, utf8.data(), (int32_t)utf8.size(), attribute_set);
        }

        // Appends glyph quads for layout, placed at offset (logical units) and
        // rasterized at scale pixels per logical unit. syntax (optional) holds
        // per-codepoint colors for the whole document, of which this layout's
        // text starts at text_base. Caller holds m.
        void emit_layout(const skb_layout_t* layout, vec2 offset, float scale, std::vector<Quad>& out,
                         const std::vector<uint32_t>* syntax = nullptr, int32_t text_base = 0)
        {
            PROFILE(L"text_request_glyphs");
            const skb_layout_params_t* params   = skb_layout_get_params(layout);
            const skb_layout_line_t*   lines    = skb_layout_get_lines(layout);
            const skb_layout_run_t*    runs     = skb_layout_get_layout_runs(layout);
            const skb_glyph_t*         glyphs   = skb_layout_get_glyphs(layout);
            const skb_cluster_t*       clusters = skb_layout_get_clusters(layout);

            for (int32_t li = 0; li < skb_layout_get_lines_count(layout); li++)
            {
                const auto& line = lines[li];
                for (int32_t ri = line.layout_run_range.start; ri < line.layout_run_range.end; ri++)
                {
                    const auto& run = runs[ri];
                    if (run.type != SKB_CONTENT_RUN_UTF8 && run.type != SKB_CONTENT_RUN_UTF32)
                        continue;

                    // Color precedence: the run's own paint attribute, then the
                    // syntax color of the glyph's text, then the draw() tint.
                    float4 run_color(1, 1, 1, 1);
                    const skb_attribute_paint_t paint = skb_attributes_get_paint(SKB_PAINT_TEXT, SKB_PAINT_STATE_DEFAULT,
                        skb_layout_get_layout_run_attributes(layout, &run), params->attribute_collection);
                    const bool run_painted = paint.paint_tag == SKB_PAINT_TEXT;
                    if (run_painted)
                        run_color = float4(paint.color.r, paint.color.g, paint.color.b, paint.color.a) / 255.0f;

                    for (int32_t gi = run.glyph_range.start; gi < run.glyph_range.end; gi++)
                    {
                        const auto& glyph = glyphs[gi];

                        float4 glyph_color = run_color;
                        bool   has_color   = run_painted;
                        if (syntax && !run_painted)
                        {
                            const size_t text_offset = size_t(text_base + clusters[glyph.cluster_idx].text_offset);
                            if (text_offset < syntax->size() && (*syntax)[text_offset] != 0)
                            {
                                glyph_color = unpack_rgba8((*syntax)[text_offset]);
                                has_color   = true;
                            }
                        }
                        const skb_quad_t q = skb_image_atlas_get_glyph_quad(atlas,
                            glyph.offset_x + offset.x, glyph.offset_y + offset.y, scale,
                            fonts, run.font_handle, glyph.gid, run.font_size,
                            skb_color_t{ 255, 255, 255, 255 }, SKB_RASTERIZE_ALPHA_MASK);

                        if (q.flags & SKB_QUAD_IS_EMPTY)
                            continue;

                        const skb_image_t* image = skb_image_atlas_get_texture(atlas, q.texture_idx);
                        if (!image) continue;

                        const float w = (float)image->width;
                        const float h = (float)image->height;

                        // geom is in logical units; the bitmap was rasterized for scale.
                        float x0 = q.geom.x * scale, y0 = q.geom.y * scale;
                        const float gw = q.geom.width * scale, gh = q.geom.height * scale;

                        // Glyphs sit at subpixel pen positions: snap the ones drawn 1:1
                        // with their bitmap to whole pixels so they sample texel centers.
                        // Scaled ones (non-integer size) stay fractional and rely on
                        // bilinear filtering instead.
                        if (std::abs(gw - q.texture.width) < 0.01f && std::abs(gh - q.texture.height) < 0.01f)
                        {
                            x0 = std::round(x0);
                            y0 = std::round(y0);
                        }

                        Quad quad;
                        quad.rect     = float4(x0, y0, x0 + gw, y0 + gh);
                        quad.uv       = float4(q.texture.x / w, q.texture.y / h, (q.texture.x + q.texture.width) / w, (q.texture.y + q.texture.height) / h);
                        quad.color     = glyph_color;
                        quad.has_color = has_color;
                        quad.atlas    = q.texture_idx;
                        quad.is_color = (q.flags & SKB_QUAD_IS_COLOR) != 0;
                        out.push_back(quad);
                    }
                }
            }
        }

        // Caller holds m.
        void rasterize_missing()
        {
            PROFILE(L"text_rasterize");
            if (skb_image_atlas_rasterize_missing_items(atlas, temp, rasterizer))
                pending_upload = true;
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

        const skb_rect2_t bounds = skb_layout_get_bounds(layout);
        out.size = vec2(bounds.width, bounds.height);

        impl->emit_layout(layout, vec2(-bounds.x, -bounds.y), 1.0f, out.quads);
        impl->rasterize_missing();
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
                g.color    = q.has_color ? float4(q.color.x, q.color.y, q.color.z, q.color.w * color.w) : color;
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

    // ------------------------------------------------------------------------
    //  Editor
    // ------------------------------------------------------------------------

    namespace
    {
        // SKB_CURRENT_SELECTION / SKB_CURRENT_SELECTION_END are macros and don't
        // cross the module boundary; these are their values.
        constexpr skb_text_position_t current_selection_end = { std::numeric_limits<int32_t>::min(), SKB_AFFINITY_NONE };
        constexpr skb_text_range_t    current_selection     = { current_selection_end, current_selection_end };
    }

    struct Editor::Impl
    {
        skb_editor_t* editor = nullptr;

        // Referenced by the editor's attribute sets, so they live as long as it does.
        skb_attribute_t paragraph_attributes[3];
        skb_attribute_t layout_attributes[1];

        bool changed         = false;
        bool suppress_change = false;

        // Highlighter output for the current text; rebuilt lazily on change.
        std::vector<uint32_t> syntax_colors;
        bool                  syntax_dirty = true;

        static void on_text_change(skb_editor_t*, skb_editor_text_change_reason_t, void* context)
        {
            auto self = static_cast<Editor::Impl*>(context);
            self->syntax_dirty = true;
            if (!self->suppress_change)
                self->changed = true;
        }

        static bool rejects(uint32_t codepoint, int32_t, int32_t, void* context)
        {
            return !(*static_cast<std::function<bool(char32_t)>*>(context))(codepoint);
        }

        static void on_input(skb_editor_t*, skb_rich_text_t* input_text, skb_text_range_t, void* context)
        {
            auto filter = static_cast<std::function<bool(char32_t)>*>(context);
            if (*filter)
                skb_rich_text_remove_if(input_text, &Impl::rejects, filter);
        }
    };

    Editor::Editor(Style style) : impl(std::make_unique<Impl>())
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        impl->paragraph_attributes[0] = skb_attribute_make_font_size(style.size);
        impl->paragraph_attributes[1] = skb_attribute_make_font_weight(to_skb(style.weight));
        impl->paragraph_attributes[2] = skb_attribute_make_font_family(to_skb(style.family));

        // Tab stops every four spaces of this style (Skribidi draws tabs only
        // with a positive increment). Falls back to a monospace-ish estimate if
        // the measured layout trims trailing whitespace.
        float tab_width = 0;
        if (const skb_layout_t* spaces = engine.get_layout("    ", style))
            tab_width = skb_layout_get_bounds(spaces).width;
        if (tab_width <= 0)
            tab_width = style.size * 0.55f * 4;
        impl->layout_attributes[0] = skb_attribute_make_tab_stop_increment(tab_width);

        skb_editor_params_t params = {};
        params.font_collection = engine.fonts;
        params.paragraph_attributes.attributes       = impl->paragraph_attributes;
        params.paragraph_attributes.attributes_count = (int32_t)std::size(impl->paragraph_attributes);
        params.layout_attributes.attributes          = impl->layout_attributes;
        params.layout_attributes.attributes_count    = (int32_t)std::size(impl->layout_attributes);
        params.caret_mode      = SKB_CARET_MODE_SIMPLE;   // Windows-style: one grapheme per step
        params.max_undo_levels = 100;

        impl->editor = skb_editor_create(&params);
        skb_editor_set_on_text_change_callback(impl->editor, &Impl::on_text_change, impl.get());
        skb_editor_set_input_filter_callback(impl->editor, &Impl::on_input, &filter);

        // A new editor has zero paragraphs, and skb_rich_layout_get_text_range_bounds
        // (unlike its siblings) doesn't guard that case: it reads paragraphs[-1].
        // set_text always creates one, even for empty text.
        impl->suppress_change = true;
        skb_editor_set_text_utf8(impl->editor, engine.temp, "", 0);
        impl->suppress_change = false;
    }

    Editor::~Editor()
    {
        // An editor outliving Engine::reset() (graphics shutdown) must not
        // bring it back: get() would re-create the engine and its atlas textures
        // after the device is gone. The editor owns no engine resources itself.
        if (!Engine::is_good())
        {
            skb_editor_destroy(impl->editor);
            return;
        }

        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_destroy(impl->editor);
    }

    void Editor::set_text(std::string_view utf8)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        impl->suppress_change = true;
        skb_editor_set_text_utf8(impl->editor, engine.temp, utf8.data(), (int32_t)utf8.size());
        impl->suppress_change = false;
    }

    std::string Editor::get_text() const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        std::string text(skb_editor_get_text_utf8_count(impl->editor), '\0');
        skb_editor_get_text_utf8(impl->editor, text.data(), (int32_t)text.size());
        return text;
    }

    bool Editor::take_changed()
    {
        return std::exchange(impl->changed, false);
    }

    void Editor::key(Key key, bool shift, bool ctrl)
    {
        skb_editor_key_t skb_key = SKB_KEY_NONE;
        switch (key)
        {
        case Key::Left:      skb_key = SKB_KEY_LEFT; break;
        case Key::Right:     skb_key = SKB_KEY_RIGHT; break;
        case Key::Up:        skb_key = SKB_KEY_UP; break;
        case Key::Down:      skb_key = SKB_KEY_DOWN; break;
        case Key::Home:      skb_key = SKB_KEY_HOME; break;
        case Key::End:       skb_key = SKB_KEY_END; break;
        case Key::Backspace: skb_key = SKB_KEY_BACKSPACE; break;
        case Key::Delete:    skb_key = SKB_KEY_DELETE; break;
        case Key::Enter:     skb_key = SKB_KEY_ENTER; break;
        }

        const uint32_t mods = (shift ? SKB_MOD_SHIFT : 0) | (ctrl ? SKB_MOD_CONTROL : 0);

        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_process_key_pressed(impl->editor, engine.temp, skb_key, mods);
    }

    void Editor::insert(char32_t codepoint)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_insert_codepoint(impl->editor, engine.temp, current_selection, codepoint);
    }

    void Editor::insert(std::string_view utf8)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_insert_text_utf8(impl->editor, engine.temp, current_selection, utf8.data(), (int32_t)utf8.size());
    }

    bool Editor::has_selection() const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        const skb_text_range_t selection = skb_editor_get_current_selection(impl->editor);
        return skb_editor_get_text_offset_from_text_position(impl->editor, selection.start)
            != skb_editor_get_text_offset_from_text_position(impl->editor, selection.end);
    }

    std::string Editor::get_selected_text() const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        const skb_text_range_t selection = skb_editor_get_current_selection(impl->editor);
        std::string text(skb_editor_get_text_utf8_count_in_range(impl->editor, selection), '\0');
        skb_editor_get_text_utf8_in_range(impl->editor, selection, text.data(), (int32_t)text.size());
        return text;
    }

    void Editor::delete_selection()
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_remove(impl->editor, engine.temp, current_selection);
    }

    void Editor::select_all()
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_select_all(impl->editor);
    }

    void Editor::select_none()
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_select_none(impl->editor);
    }

    void Editor::undo()
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_undo(impl->editor, engine.temp);
    }

    void Editor::redo()
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_redo(impl->editor, engine.temp);
    }

    bool Editor::can_undo() const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        return skb_editor_can_undo(impl->editor);
    }

    bool Editor::can_redo() const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        return skb_editor_can_redo(impl->editor);
    }

    void Editor::mouse_click(vec2 pos, bool shift, double time_seconds)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_process_mouse_click(impl->editor, pos.x, pos.y, shift ? SKB_MOD_SHIFT : 0, time_seconds);
    }

    void Editor::mouse_drag(vec2 pos)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_process_mouse_drag(impl->editor, pos.x, pos.y);
    }

    bool Editor::selection_contains(vec2 pos) const
    {
        const auto rects = selection_rects();
        return std::any_of(rects.begin(), rects.end(), [&](const float4& r)
            {
                return pos.x >= r.x && pos.x < r.z && pos.y >= r.y && pos.y < r.w;
            });
    }

    Caret Editor::caret_at(vec2 pos) const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        const skb_text_position_t at = skb_editor_hit_test(impl->editor, SKB_MOVEMENT_CARET, pos.x, pos.y);
        const skb_caret_info_t info = skb_editor_get_caret_info_at(impl->editor, at);

        Caret caret;
        caret.center = vec2(info.x, info.y + (info.ascender + info.descender) * 0.5f);
        caret.height = info.descender - info.ascender;
        return caret;
    }

    void Editor::move_selection(vec2 pos, bool copy)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_t* e = impl->editor;

        const skb_text_range_t selection = skb_editor_get_current_selection(e);
        int32_t start = skb_editor_get_text_offset_from_text_position(e, selection.start);
        int32_t end   = skb_editor_get_text_offset_from_text_position(e, selection.end);
        if (start > end) std::swap(start, end);
        if (start == end) return;

        int32_t drop = skb_editor_get_text_offset_from_text_position(e,
            skb_editor_hit_test(e, SKB_MOVEMENT_CARET, pos.x, pos.y));

        // Dropping a move back into its own span changes nothing.
        if (!copy && drop >= start && drop <= end) return;

        std::string moved(skb_editor_get_text_utf8_count_in_range(e, selection), '\0');
        skb_editor_get_text_utf8_in_range(e, selection, moved.data(), (int32_t)moved.size());

        const int32_t moved_count = end - start;
        auto at = [](int32_t offset) { return skb_text_position_t{ offset, SKB_AFFINITY_TRAILING }; };

        const int32_t transaction = skb_editor_undo_transaction_begin(e);

        if (!copy)
        {
            skb_editor_remove(e, engine.temp, current_selection);
            // Removal shifts everything after the old span left.
            if (drop > end)
                drop -= moved_count;
        }

        skb_editor_select(e, skb_text_range_t{ at(drop), at(drop) });
        skb_editor_insert_text_utf8(e, engine.temp, current_selection, moved.data(), (int32_t)moved.size());
        skb_editor_select(e, skb_text_range_t{ at(drop), at(drop + moved_count) });

        skb_editor_undo_transaction_end(e, transaction);
    }

    void Editor::drop_text(vec2 pos, std::string_view utf8)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_t* e = impl->editor;

        const int32_t drop = skb_editor_get_text_offset_from_text_position(e,
            skb_editor_hit_test(e, SKB_MOVEMENT_CARET, pos.x, pos.y));
        auto at = [](int32_t offset) { return skb_text_position_t{ offset, SKB_AFFINITY_TRAILING }; };

        const int32_t count_before = skb_editor_get_text_utf32_count(e);
        const int32_t transaction  = skb_editor_undo_transaction_begin(e);

        skb_editor_select(e, skb_text_range_t{ at(drop), at(drop) });
        skb_editor_insert_text_utf8(e, engine.temp, current_selection, utf8.data(), (int32_t)utf8.size());

        // Measured rather than counted from utf8: the input filter may have
        // dropped some of it.
        const int32_t inserted = skb_editor_get_text_utf32_count(e) - count_before;
        skb_editor_select(e, skb_text_range_t{ at(drop), at(drop + inserted) });

        skb_editor_undo_transaction_end(e, transaction);
    }

    void Editor::build(float scale, Layout& out)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        out.quads.clear();
        out.size = {};
        skb_editor_t* e = impl->editor;

        if (highlighter && impl->syntax_dirty)
        {
            PROFILE(L"text_highlight");
            std::u32string text(skb_editor_get_text_utf32_count(e), U'\0');
            skb_editor_get_text_utf32(e, reinterpret_cast<uint32_t*>(text.data()), (int32_t)text.size());

            impl->syntax_colors.assign(text.size(), 0);
            highlighter(text, impl->syntax_colors);
            impl->syntax_dirty = false;
        }
        const std::vector<uint32_t>* syntax = highlighter ? &impl->syntax_colors : nullptr;

        for (int32_t pi = 0; pi < skb_editor_get_paragraph_count(e); pi++)
        {
            const skb_vec2_t offset = skb_editor_get_paragraph_offset(e, pi);
            engine.emit_layout(skb_editor_get_paragraph_layout(e, pi), vec2(offset.x, offset.y), scale, out.quads,
                syntax, skb_editor_get_paragraph_global_text_offset(e, pi));
        }

        engine.rasterize_missing();
    }

    vec2 Editor::content_size() const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        vec2 size = {};
        for (int32_t pi = 0; pi < skb_editor_get_paragraph_count(impl->editor); pi++)
        {
            const skb_vec2_t  offset = skb_editor_get_paragraph_offset(impl->editor, pi);
            const skb_rect2_t bounds = skb_layout_get_bounds(skb_editor_get_paragraph_layout(impl->editor, pi));
            size.x = std::max(size.x, offset.x + bounds.x + bounds.width);
            size.y = std::max(size.y, offset.y + skb_editor_get_paragraph_advance_y(impl->editor, pi));
        }
        return size;
    }

    void Editor::toggle_bold()
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_toggle_attribute(impl->editor, engine.temp, current_selection, skb_attribute_make_font_weight(SKB_WEIGHT_BOLD));
    }

    void Editor::toggle_italic()
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_toggle_attribute(impl->editor, engine.temp, current_selection, skb_attribute_make_font_style(SKB_STYLE_ITALIC));
    }

    void Editor::set_color(float4 color)
    {
        const skb_color_t c = {
            (uint8_t)std::clamp(color.x * 255.0f, 0.0f, 255.0f), (uint8_t)std::clamp(color.y * 255.0f, 0.0f, 255.0f),
            (uint8_t)std::clamp(color.z * 255.0f, 0.0f, 255.0f), (uint8_t)std::clamp(color.w * 255.0f, 0.0f, 255.0f) };

        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_set_attribute(impl->editor, engine.temp, current_selection,
            skb_attribute_make_paint_color(SKB_PAINT_TEXT, SKB_PAINT_STATE_DEFAULT, c));
    }

    void Editor::clear_color()
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_clear_attribute(impl->editor, engine.temp, current_selection,
            skb_attribute_make_paint_color(SKB_PAINT_TEXT, SKB_PAINT_STATE_DEFAULT, skb_color_t{}));
    }

    Caret Editor::caret() const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        const skb_caret_info_t info = skb_editor_get_caret_info_at(impl->editor, current_selection_end);

        // ascender is negative (above the baseline), descender positive.
        Caret caret;
        caret.center = vec2(info.x, info.y + (info.ascender + info.descender) * 0.5f);
        caret.height = info.descender - info.ascender;
        return caret;
    }

    std::vector<float4> Editor::selection_rects() const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        std::vector<float4> rects;
        if (skb_editor_get_paragraph_count(impl->editor) == 0)
            return rects;

        skb_editor_iterate_text_range_bounds(impl->editor, current_selection,
            [](skb_rect2_t r, void* context)
            {
                static_cast<std::vector<float4>*>(context)->emplace_back(r.x, r.y, r.x + r.width, r.y + r.height);
            },
            &rects);

        // A line with several styles is several layout runs, and Skribidi emits
        // a rect per run segment: they overlap at style boundaries and differ in
        // height (bold/italic ascenders). Drawn translucent, every overlap would
        // show darker. Merge rects on the same line that overlap or touch.
        auto same_line = [](const float4& a, const float4& b)
            {
                const float overlap = std::min(a.w, b.w) - std::max(a.y, b.y);
                return overlap > 0.5f * std::min(a.w - a.y, b.w - b.y);
            };
        auto touching = [](const float4& a, const float4& b)
            {
                return a.x <= b.z + 0.5f && b.x <= a.z + 0.5f;
            };

        for (bool merged = true; merged; )
        {
            merged = false;
            for (size_t i = 0; i < rects.size() && !merged; ++i)
                for (size_t j = i + 1; j < rects.size() && !merged; ++j)
                    if (same_line(rects[i], rects[j]) && touching(rects[i], rects[j]))
                    {
                        rects[i] = float4(std::min(rects[i].x, rects[j].x), std::min(rects[i].y, rects[j].y),
                                          std::max(rects[i].z, rects[j].z), std::max(rects[i].w, rects[j].w));
                        rects.erase(rects.begin() + j);
                        merged = true;
                    }
        }
        return rects;
    }
}
