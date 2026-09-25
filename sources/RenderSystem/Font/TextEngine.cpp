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

        // UI icons drawn in code on a 16x16 grid: no asset files, and as alpha
        // masks they tint like text. Filled polygons with nonzero winding, so a
        // counter-clockwise contour inside a clockwise one is a hole.
        struct IconBuilder
        {
            skb_icon_builder_t b;

            IconBuilder(skb_icon_collection_t* icons, const char* name)
            {
                const skb_icon_handle_t handle = skb_icon_collection_add_icon(icons, name, 16, 16);
                skb_icon_collection_set_is_color(icons, handle, false);
                b = skb_icon_builder_make(icons, handle);
                skb_icon_builder_begin_shape(&b);
            }

            ~IconBuilder()
            {
                skb_icon_builder_fill_color(&b, skb_color_t{ 255, 255, 255, 255 });
                skb_icon_builder_end_shape(&b);
            }

            void polygon(std::initializer_list<skb_vec2_t> points)
            {
                auto it = points.begin();
                skb_icon_builder_move_to(&b, *it);
                for (++it; it != points.end(); ++it)
                    skb_icon_builder_line_to(&b, *it);
                skb_icon_builder_close_path(&b);
            }

            // Circle from four cubic arcs; clockwise unless ccw.
            void circle(float cx, float cy, float r, bool ccw = false)
            {
                const float k = 0.5523f * r;   // cubic approximation of a quarter circle
                const float s = ccw ? -1.0f : 1.0f;
                skb_icon_builder_move_to(&b, { cx + r, cy });
                skb_icon_builder_cubic_to(&b, { cx + r, cy + s * k }, { cx + k, cy + s * r }, { cx, cy + s * r });
                skb_icon_builder_cubic_to(&b, { cx - k, cy + s * r }, { cx - r, cy + s * k }, { cx - r, cy });
                skb_icon_builder_cubic_to(&b, { cx - r, cy - s * k }, { cx - k, cy - s * r }, { cx, cy - s * r });
                skb_icon_builder_cubic_to(&b, { cx + k, cy - s * r }, { cx + r, cy - s * k }, { cx + r, cy });
                skb_icon_builder_close_path(&b);
            }
        };

        void add_builtin_icons(skb_icon_collection_t* icons)
        {
            { IconBuilder i(icons, "chevron_right"); i.polygon({ {5.5f, 3}, {10.5f, 8}, {5.5f, 13}, {4, 11.5f}, {7.5f, 8}, {4, 4.5f} }); }
            { IconBuilder i(icons, "chevron_down");  i.polygon({ {3, 5.5f}, {4.5f, 4}, {8, 7.5f}, {11.5f, 4}, {13, 5.5f}, {8, 10.5f} }); }
            {
                IconBuilder i(icons, "close");
                i.polygon({ {3.5f, 4.9f}, {4.9f, 3.5f}, {12.5f, 11.1f}, {11.1f, 12.5f} });
                i.polygon({ {11.1f, 3.5f}, {12.5f, 4.9f}, {4.9f, 12.5f}, {3.5f, 11.1f} });
            }
            {
                // Disc with a "!" cut out (counter-clockwise inner contours).
                IconBuilder i(icons, "error");
                i.circle(8, 8, 7);
                i.polygon({ {7, 3.5f}, {7, 9.5f}, {9, 9.5f}, {9, 3.5f} });
                i.circle(8, 11.5f, 1.1f, true);
            }
            {
                IconBuilder i(icons, "warning");
                i.polygon({ {8, 1}, {15.5f, 14.5f}, {0.5f, 14.5f} });
                i.polygon({ {7.1f, 5.5f}, {7.1f, 10}, {8.9f, 10}, {8.9f, 5.5f} });
                i.circle(8, 12.2f, 1, true);
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
        skb_icon_collection_t* icons      = nullptr;
        skb_rasterizer_t*      rasterizer = nullptr;
        skb_image_atlas_t*     atlas      = nullptr;
        skb_layout_cache_t*    layouts    = nullptr;
        skb_temp_alloc_t*      temp       = nullptr;

        std::vector<AtlasTexture> textures;
        bool pending_upload = false;

        // Caller holds m.
        // With links, the text becomes plain and link runs; a link run's
        // content_id is its link index + 1 (0 = plain, as Skribidi expects).
        const skb_layout_t* get_layout(std::string_view utf8, Style style, std::span<const Link> links = {})
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

            if (links.empty())
                return skb_layout_cache_get_utf8(layouts, temp, &params, utf8.data(), (int32_t)utf8.size(), attribute_set);

            // Colors suit the UI's dark panels; the underline takes the text paint,
            // so it follows the hover color too.
            const skb_attribute_t link_attributes[] = {
                skb_attribute_make_font_size(style.size),
                skb_attribute_make_font_weight(to_skb(style.weight)),
                skb_attribute_make_font_family(to_skb(style.family)),
                skb_attribute_make_paint_color(SKB_PAINT_TEXT, SKB_PAINT_STATE_DEFAULT, skb_color_t{ 90, 160, 255, 255 }),
                skb_attribute_make_paint_color(SKB_PAINT_TEXT, SKB_PAINT_STATE_HOVER, skb_color_t{ 170, 210, 255, 255 }),
                skb_attribute_make_decoration(SKB_DECORATION_LINE_UNDER, SKB_DECORATION_STYLE_SOLID, 1.0f, 0.0f, SKB_PAINT_TEXT),
            };
            skb_attribute_set_t link_set = {};
            link_set.attributes       = link_attributes;
            link_set.attributes_count = (int32_t)std::size(link_attributes);

            std::vector<skb_content_run_t> runs;
            uint32_t at = 0;
            const uint32_t size = (uint32_t)utf8.size();
            for (size_t li = 0; li < links.size(); ++li)
            {
                const uint32_t begin = std::min(links[li].begin, size);
                const uint32_t end   = std::clamp(links[li].end, begin, size);
                if (begin > at)
                    runs.push_back(skb_content_run_make_utf8(utf8.data() + at, int32_t(begin - at), attribute_set, 0));
                if (end > begin)
                    runs.push_back(skb_content_run_make_utf8(utf8.data() + begin, int32_t(end - begin), link_set, intptr_t(li + 1)));
                at = std::max(at, end);
            }
            if (at < size)
                runs.push_back(skb_content_run_make_utf8(utf8.data() + at, int32_t(size - at), attribute_set, 0));

            return skb_layout_cache_get_from_runs(layouts, temp, &params, runs.data(), (int32_t)runs.size());
        }

        // Appends glyph quads for layout, placed at offset (logical units) and
        // rasterized at scale pixels per logical unit. syntax (optional) holds
        // per-codepoint colors for the whole document, of which this layout's
        // text starts at text_base. Caller holds m.
        void emit_layout(const skb_layout_t* layout, vec2 offset, float scale, std::vector<Quad>& out,
                         const std::vector<uint32_t>* syntax = nullptr, int32_t text_base = 0, intptr_t hovered_content = 0)
        {
            auto run_state = [&](const skb_layout_run_t& run)
                {
                    return run.content_id != 0 && run.content_id == hovered_content ? SKB_PAINT_STATE_HOVER : SKB_PAINT_STATE_DEFAULT;
                };
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
                    const skb_attribute_paint_t paint = skb_attributes_get_paint(SKB_PAINT_TEXT, run_state(run),
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
                        quad.is_color = image->bpp == 4;   // see append_quad
                        out.push_back(quad);
                    }
                }

                // Underlines and the like from decoration attributes (links),
                // colored by their run's paint in its current state.
                const skb_decoration_t* decorations = skb_layout_get_decorations(layout);
                for (int32_t di = line.decorations_range.start; di < line.decorations_range.end; di++)
                {
                    const skb_decoration_t& dec = decorations[di];
                    if (dec.type != SKB_DECORATION_LINE) continue;

                    const skb_layout_run_t& run = runs[dec.line.layout_run_idx];
                    const skb_attribute_paint_t paint = skb_attributes_get_paint(dec.line.paint_tag, run_state(run),
                        skb_layout_get_layout_run_attributes(layout, &run), params->attribute_collection);
                    if (paint.paint_tag != dec.line.paint_tag) continue;

                    const skb_quad_t q = skb_image_atlas_get_decoration_quad(atlas, offset.x + dec.line.x, offset.y + dec.line.y, scale,
                        (skb_decoration_position_t)dec.line.position, (skb_decoration_style_t)dec.line.style,
                        dec.line.length, dec.line.pattern_offset, dec.line.thickness,
                        skb_color_t{ 255, 255, 255, 255 }, SKB_RASTERIZE_ALPHA_MASK);
                    append_quad(q, scale, float4(paint.color.r, paint.color.g, paint.color.b, paint.color.a) / 255.0f, out);
                }
            }
        }

        // Appends an icon or decoration quad (geometry in logical units, bitmap
        // rasterized for scale) in its own color. Caller holds m.
        void append_quad(const skb_quad_t& q, float scale, float4 color, std::vector<Quad>& out)
        {
            if (q.flags & SKB_QUAD_IS_EMPTY) return;

            const skb_image_t* image = skb_image_atlas_get_texture(atlas, q.texture_idx);
            if (!image) return;

            const float tw = (float)image->width, th = (float)image->height;
            const float4 atlas_rect = float4(q.texture.x / tw, q.texture.y / th, (q.texture.x + q.texture.width) / tw, (q.texture.y + q.texture.height) / th);

            const float x0 = std::round(q.geom.x * scale), y0 = std::round(q.geom.y * scale);

            Quad quad;
            quad.rect      = float4(x0, y0, x0 + q.geom.width * scale, y0 + q.geom.height * scale);
            quad.color     = color;
            quad.atlas     = q.texture_idx;
            // From the atlas texture, not SKB_QUAD_IS_COLOR: decoration quads
            // always carry that flag although patterns live in the alpha atlas,
            // which the color path turns into an opaque black box. Skribidi's own
            // renderer also goes by the texture's bpp.
            quad.is_color  = image->bpp == 4;
            quad.has_color = true;

            // Decoration patterns repeat along the quad: uv counts tiles.
            const bool repeats = q.pattern.width != 1.0f || q.pattern.height != 1.0f || q.pattern.x != 0.0f || q.pattern.y != 0.0f;
            if (repeats)
            {
                quad.is_pattern = true;
                quad.tile       = atlas_rect;
                quad.uv         = float4(q.pattern.x, q.pattern.y, q.pattern.x + q.pattern.width, q.pattern.y + q.pattern.height);
            }
            else
                quad.uv = atlas_rect;

            out.push_back(quad);
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

        impl->icons = skb_icon_collection_create();
        add_builtin_icons(impl->icons);

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
        skb_icon_collection_destroy(impl->icons);
        skb_font_collection_destroy(impl->fonts);
    }

    void Engine::build_icon(std::string_view name, vec2 size, Layout& out)
    {
        std::lock_guard<std::mutex> lock(impl->m);

        out.quads.clear();
        out.size = size;

        const std::string name_z(name);
        const skb_icon_handle_t handle = skb_icon_collection_find_icon(impl->icons, name_z.c_str());
        if (!handle || size.x <= 0 || size.y <= 0) return;

        // Largest aspect-correct fit, centred and snapped to whole pixels.
        const skb_vec2_t fit = skb_icon_collection_calc_proportional_size(impl->icons, handle, size.x, size.y);
        const float w = std::floor(fit.x), h = std::floor(fit.y);
        const float x = std::round((size.x - w) * 0.5f), y = std::round((size.y - h) * 0.5f);

        const skb_quad_t q = skb_image_atlas_get_icon_quad(impl->atlas, x, y, 1.0f, impl->icons, handle, w, h,
            skb_color_t{ 255, 255, 255, 255 }, SKB_RASTERIZE_ALPHA_MASK);
        if (q.flags & SKB_QUAD_IS_EMPTY) return;

        const skb_image_t* image = skb_image_atlas_get_texture(impl->atlas, q.texture_idx);
        if (!image) return;

        const float tw = (float)image->width, th = (float)image->height;

        Quad quad;
        quad.rect      = float4(q.geom.x, q.geom.y, q.geom.x + q.geom.width, q.geom.y + q.geom.height);
        quad.uv        = float4(q.texture.x / tw, q.texture.y / th, (q.texture.x + q.texture.width) / tw, (q.texture.y + q.texture.height) / th);
        quad.color     = float4(1, 1, 1, 1);
        quad.atlas     = q.texture_idx;
        quad.is_color  = false;
        quad.has_color = false;
        out.quads.push_back(quad);

        impl->rasterize_missing();
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

    void Engine::build(std::string_view utf8, Style style, Layout& out, std::span<const Link> links, int hovered_link)
    {
        std::lock_guard<std::mutex> lock(impl->m);

        out.quads.clear();
        out.size = {};

        const skb_layout_t* layout = impl->get_layout(utf8, style, links);
        if (!layout) return;

        const skb_rect2_t bounds = skb_layout_get_bounds(layout);
        out.size = vec2(bounds.width, bounds.height);

        impl->emit_layout(layout, vec2(-bounds.x, -bounds.y), 1.0f, out.quads, nullptr, 0, intptr_t(hovered_link + 1));
        impl->rasterize_missing();
    }

    int Engine::hit_link(std::string_view utf8, Style style, std::span<const Link> links, vec2 at)
    {
        if (links.empty()) return -1;

        std::lock_guard<std::mutex> lock(impl->m);
        const skb_layout_t* layout = impl->get_layout(utf8, style, links);
        if (!layout) return -1;

        const skb_rect2_t bounds = skb_layout_get_bounds(layout);
        const skb_layout_content_hit_t hit = skb_layout_hit_test_content(layout, at.x + bounds.x, at.y + bounds.y);
        return int(hit.content_id) - 1;
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

    void Engine::draw(HAL::CommandList::ptr& list, const Layout& layout, vec2 pos, float4 color, sizer clip, vec2 window_size, bool raw_output)
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
                g.atlas      = q.atlas;
                g.is_color   = q.is_color ? 1 : 0;
                g.is_pattern = q.is_pattern ? 1 : 0;
                g.tile       = q.tile;
                gpu_quads.push_back(g);
            }
        }
        if (gpu_quads.empty()) return;

        {
            PROFILE(L"text_draw");
            auto& graphics = list->get_graphics();
            auto formats = graphics.get_formats();
            if (formats.empty()) return;

            graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
            graphics.set_pipeline<PSOS::UI::Text::GlyphRender>(PSOS::UI::Text::GlyphRender::Format(formats[0]));

            // The struct stride and Vulkan's minStorageBufferOffsetAlignment
            // must both hold.
            using GlyphQuad = Table::UI::Text::GlyphQuad;
            const uint32_t align = std::lcm<uint32_t>(sizeof(GlyphQuad), RenderSystem::get().device().get_properties().min_storage_buffer_offset_alignment);
            auto data = list->place_data(sizeof(GlyphQuad) * gpu_quads.size(), align);
            list->write<GlyphQuad>(data, gpu_quads);

            auto view = data.resource->create_view<HAL::StructuredBufferView<GlyphQuad>>(*list,
                HAL::StructuredBufferViewDesc{ (UINT)data.resource_offset, (UINT)data.size, HAL::counterType::NONE });

            Slots::UI::Text::GlyphRender slot;
            slot.GetQuads()    = view;
            slot.GetTextures() = atlas_handles;
            slot.GetRaw_output() = raw_output ? 1 : 0;
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

        // Highlighter output for the current text, one entry per paragraph.
        struct LineSyntax
        {
            uint32_t              version = 0;      // Skribidi's paragraph version: changes with the paragraph's content
            bool                  lexed = false;    // colors match the text (not necessarily state_in)
            uint32_t              state_in = 0;     // lexer state entering the line
            uint32_t              state_out = 0;
            std::vector<uint32_t> colors;           // per codepoint of the paragraph
        };
        std::vector<LineSyntax> syntax_lines;
        bool                    syntax_dirty = true;
        // Lines before lex_from are correct. From there on a line may be
        // unlexed or lexed with a stale incoming state; last_unlexed bounds
        // where the unlexed ones can be (-1: none).
        int32_t                 lex_from = 0;
        int32_t                 last_unlexed = -1;

        // content_size() walks every paragraph; recomputed only after a change.
        vec2 cached_content_size;
        bool content_size_dirty = true;

        // After an edit: lines are matched to the cache from both ends by
        // paragraph version (Skribidi bumps it on any change to the paragraph),
        // the changed middle is replaced by unlexed entries. O(lines) integer
        // compares; no text is read.
        void sync_syntax_lines()
        {
            const int32_t count     = skb_editor_get_paragraph_count(editor);
            const int32_t old_count = (int32_t)syntax_lines.size();
            const skb_rich_text_t* rich = skb_editor_get_rich_text(editor);

            int32_t prefix = 0;
            while (prefix < count && prefix < old_count && syntax_lines[prefix].version == skb_rich_text_get_paragraph_version(rich, prefix))
                ++prefix;
            int32_t suffix = 0;
            while (suffix < count - prefix && suffix < old_count - prefix
                   && syntax_lines[old_count - 1 - suffix].version == skb_rich_text_get_paragraph_version(rich, count - 1 - suffix))
                ++suffix;

            const int32_t old_middle_end = old_count - suffix;
            const int32_t new_middle_end = count - suffix;
            syntax_lines.erase(syntax_lines.begin() + prefix, syntax_lines.begin() + old_middle_end);
            syntax_lines.insert(syntax_lines.begin() + prefix, new_middle_end - prefix, LineSyntax{});
            for (int32_t pi = prefix; pi < new_middle_end; ++pi)
                syntax_lines[pi].version = skb_rich_text_get_paragraph_version(rich, pi);

            // Old indices into the new numbering: the tail shifts, anything in
            // the replaced middle falls back to its start (lex_from) or end
            // (last_unlexed).
            const int32_t shift = count - old_count;
            if (lex_from >= old_middle_end) lex_from += shift;
            lex_from = std::min(lex_from, prefix);

            if (last_unlexed >= old_middle_end) last_unlexed += shift;
            else if (last_unlexed >= prefix)    last_unlexed = prefix - 1;
            if (new_middle_end > prefix)
                last_unlexed = std::max(last_unlexed, new_middle_end - 1);
        }

        // Lexes forward from lex_from until the lines agree with the state
        // flowing into them again, but never past last_needed: an opened block
        // comment at the top of a large file re-lexes only what is on screen,
        // the rest follows as it scrolls into view. Caller holds the engine lock.
        void update_syntax(const Highlighter& highlighter, int32_t last_needed)
        {
            if (syntax_dirty)
            {
                PROFILE(L"text_highlight_sync");
                syntax_dirty = false;
                sync_syntax_lines();
            }

            const int32_t count = (int32_t)syntax_lines.size();
            if (lex_from >= count) return;

            PROFILE(L"text_highlight");
            uint32_t state = lex_from > 0 ? syntax_lines[lex_from - 1].state_out : 0;
            int32_t pi = lex_from;
            for (; pi < count; ++pi)
            {
                LineSyntax& line = syntax_lines[pi];

                // Unchanged text entered in the same state: still correct, and
                // so is everything after it up to the next unlexed line.
                if (line.lexed && line.state_in == state)
                {
                    if (pi > last_unlexed)
                    {
                        pi = count;
                        break;
                    }
                    state = line.state_out;
                    continue;
                }

                if (pi > last_needed) break;

                const skb_text_t* text = skb_editor_get_paragraph_text(editor, pi);
                const uint32_t*   cps  = skb_text_get_utf32(text);
                const int32_t     n    = skb_text_get_utf32_count(text);
                int32_t length = n;
                while (length > 0 && (cps[length - 1] == '\n' || cps[length - 1] == '\r')) --length;

                line.lexed    = true;
                line.state_in = state;
                line.colors.assign(n, 0);
                const std::u32string_view view(reinterpret_cast<const char32_t*>(cps), length);
                line.state_out = highlighter(view, state, std::span<uint32_t>(line.colors.data(), length));
                state = line.state_out;
            }

            lex_from = pi;
            if (lex_from >= count)
                last_unlexed = -1;
        }

        // First paragraph whose extent reaches y (paragraph offsets increase
        // monotonically). Caller holds the engine lock.
        int32_t first_paragraph_reaching(float y) const
        {
            int32_t lo = 0, hi = skb_editor_get_paragraph_count(editor);
            while (lo < hi)
            {
                const int32_t mid = (lo + hi) / 2;
                const float bottom = skb_editor_get_paragraph_offset(editor, mid).y + skb_editor_get_paragraph_advance_y(editor, mid);
                if (bottom < y) lo = mid + 1;
                else            hi = mid;
            }
            return lo;
        }

        std::vector<Diagnostic> diagnostics;
        // Where the last build drew each diagnostic (editor space), for hover.
        struct DiagnosticArea
        {
            float4 rect;
            size_t index;
            bool   whole_line;   // the line's icon + message: tooltip lists every message on it
        };
        std::vector<DiagnosticArea> diagnostic_areas;

        float font_size = 16;
        Style style;

        // Image paragraphs carry their id in the paragraph group tag, flagged
        // by the high bit. Each image gets its own tag: Skribidi collapses the
        // padding between neighbouring paragraphs that share one.
        static constexpr uint32_t image_tag_flag = 0x80000000u;
        static constexpr float    image_gap = 4;   // above the image, logical units
        std::vector<ImagePlacement> image_placements;
        bool                        has_images = false;   // insert_image was ever called; undo can bring them back

        // A paragraph split from an image paragraph (Enter, paste or a text drop
        // inside its line) inherits the image's padding and tag, which would
        // draw the image again. Keeps each tag on its first paragraph and
        // resets the rest. Must run inside the edit's own undo transaction:
        // done later as a step of its own, undo would revert just this fix
        // and it would re-apply at once. Caller holds the engine lock.
        void strip_duplicate_images(skb_temp_alloc_t* temp)
        {
            // Walks every paragraph: not worth it on every Enter in a large
            // file that never had an image.
            if (!has_images) return;

            std::vector<uint32_t> seen;
            for (int32_t pi = 0; pi < skb_editor_get_paragraph_count(editor); pi++)
            {
                const uint32_t tag = skb_attributes_get_group(skb_editor_get_paragraph_attributes(editor, pi), nullptr);
                if (!(tag & image_tag_flag)) continue;

                if (std::find(seen.begin(), seen.end(), tag) == seen.end())
                {
                    seen.push_back(tag);
                    continue;
                }

                const int32_t offset = skb_editor_get_paragraph_global_text_offset(editor, pi);
                const skb_text_range_t paragraph = { { offset, SKB_AFFINITY_TRAILING }, { offset, SKB_AFFINITY_TRAILING } };
                skb_editor_set_paragraph_attribute(editor, temp, paragraph, skb_attribute_make_paragraph_padding(0, 0, 0, 0));
                skb_editor_set_paragraph_attribute(editor, temp, paragraph, skb_attribute_make_group_tag(0));
            }
        }

        // Image paragraphs overlapping [top, bottom]. Caller holds the engine lock.
        void collect_images(float top, float bottom)
        {
            image_placements.clear();
            const int32_t count = skb_editor_get_paragraph_count(editor);
            for (int32_t pi = first_paragraph_reaching(top); pi < count; pi++)
            {
                if (skb_editor_get_paragraph_offset(editor, pi).y > bottom) break;

                const skb_attribute_set_t attributes = skb_editor_get_paragraph_attributes(editor, pi);
                const uint32_t tag = skb_attributes_get_group(attributes, nullptr);
                if (!(tag & image_tag_flag)) continue;

                const skb_attribute_paragraph_padding_t padding = skb_attributes_get_paragraph_padding(attributes, nullptr);
                const skb_vec2_t offset = skb_editor_get_paragraph_offset(editor, pi);
                image_placements.push_back({ tag & ~image_tag_flag, vec2(offset.x + padding.start, offset.y + image_gap), padding.top - image_gap });
            }
        }

        // Wavy underline under each diagnostic's text, one icon at the end of
        // each affected line, for the paragraphs [first, last) only.
        // diagnostics is sorted by line. Caller holds the engine lock.
        void build_diagnostics(Engine::Impl& engine, float scale, std::vector<Quad>& out, int32_t first, int32_t last)
        {
            diagnostic_areas.clear();
            if (diagnostics.empty()) return;
            PROFILE(L"text_diagnostics");

            const float4 error_color   = float4(220, 40, 40, 255) / 255.0f;
            const float4 warning_color = float4(215, 150, 0, 255) / 255.0f;
            auto is_word = [](uint32_t c) { return c == '_' || c > 127 || (c >= '0' && c <= '9') || ((c | 0x20) >= 'a' && (c | 0x20) <= 'z'); };
            auto at = [](int32_t offset) { return skb_text_position_t{ offset, SKB_AFFINITY_TRAILING }; };
            auto by_line = [](const Diagnostic& d, uint32_t line) { return d.line < line; };

            const auto begin = std::lower_bound(diagnostics.begin(), diagnostics.end(), (uint32_t)first, by_line);
            const auto end   = std::lower_bound(begin, diagnostics.end(), (uint32_t)last, by_line);

            for (auto it = begin; it != end; ++it)
            {
                const size_t      di = size_t(it - diagnostics.begin());
                const Diagnostic& d  = *it;
                const int32_t     pi = (int32_t)d.line;

                const float4 color = d.warning ? warning_color : error_color;

                // The line's text without its paragraph separator.
                const skb_text_t* text = skb_editor_get_paragraph_text(editor, pi);
                const uint32_t*   cps  = skb_text_get_utf32(text);
                int32_t count = skb_text_get_utf32_count(text);
                while (count > 0 && (cps[count - 1] == '\n' || cps[count - 1] == '\r')) --count;

                // Compilers report a position; underline the word there (at
                // least one character), or the explicit length. Once per spot:
                // a cascade of errors often repeats one position many times.
                const bool same_spot = it != begin && std::prev(it)->line == d.line
                                       && std::prev(it)->column == d.column && std::prev(it)->length == d.length;
                if (count > 0 && !same_spot)
                {
                    const int32_t start = std::min<int32_t>((int32_t)d.column, count - 1);
                    int32_t end = d.length ? start + (int32_t)d.length : start;
                    if (!d.length)
                        while (end < count && is_word(cps[end])) ++end;
                    end = std::clamp(end, start + 1, count);

                    const int32_t base = skb_editor_get_paragraph_global_text_offset(editor, pi);
                    std::vector<skb_rect2_t> rects;
                    skb_editor_iterate_text_range_bounds(editor, skb_text_range_t{ at(base + start), at(base + end) },
                        [](skb_rect2_t r, void* context) { static_cast<std::vector<skb_rect2_t>*>(context)->push_back(r); },
                        &rects);

                    for (auto& r : rects)
                    {
                        // UNDER places the wave's top at y: tuck it against the
                        // bottom of the text's line box.
                        const skb_quad_t q = skb_image_atlas_get_decoration_quad(engine.atlas, r.x, r.y + r.height - 3.0f, scale,
                            SKB_DECORATION_LINE_UNDER, SKB_DECORATION_STYLE_WAVY, r.width, 0.0f, 1.0f,
                            skb_color_t{ 255, 255, 255, 255 }, SKB_RASTERIZE_ALPHA_MASK);
                        engine.append_quad(q, scale, color, out);
                        diagnostic_areas.push_back({ float4(r.x, r.y, r.x + r.width, r.y + r.height), di, false });
                    }
                }

                // Sorted: the line's first diagnostic places its icon.
                if (it != begin && std::prev(it)->line == d.line)
                    continue;

                const skb_vec2_t  offset = skb_editor_get_paragraph_offset(editor, pi);
                const skb_rect2_t bounds = skb_layout_get_bounds(skb_editor_get_paragraph_layout(editor, pi));
                const float size = std::round(font_size * 0.9f);
                const float x = offset.x + bounds.x + bounds.width + font_size * 0.5f;
                const float y = offset.y + bounds.y + (bounds.height - size) * 0.5f;

                const skb_icon_handle_t icon = skb_icon_collection_find_icon(engine.icons, d.warning ? "warning" : "error");
                const skb_quad_t q = skb_image_atlas_get_icon_quad(engine.atlas, x, y, scale, engine.icons, icon, size, size,
                    skb_color_t{ 255, 255, 255, 255 }, SKB_RASTERIZE_ALPHA_MASK);
                engine.append_quad(q, scale, color, out);

                // The line's first message after the icon (Error Lens style); the
                // rest are counted, and the tooltip over this area shows them all.
                const size_t on_line = size_t(std::find_if(it, end, [&](const Diagnostic& o) { return o.line != d.line; }) - it);
                std::string message = d.message;
                if (on_line > 1)
                    message += "  (+" + std::to_string(on_line - 1) + " more)";

                float area_right = x + size;
                const skb_layout_t* line_layout = skb_editor_get_paragraph_layout(editor, pi);
                const skb_layout_t* message_layout = engine.get_layout(message, Style{ font_size * 0.9f, Weight::Normal, Family::Sans });
                if (message_layout && skb_layout_get_lines_count(message_layout) > 0 && skb_layout_get_lines_count(line_layout) > 0)
                {
                    // Sit on the code line's baseline: centring the smaller text in
                    // the line box leaves it visibly off the code's text line.
                    const skb_rect2_t tb = skb_layout_get_bounds(message_layout);
                    const float code_baseline    = offset.y + skb_layout_get_lines(line_layout)[0].baseline;
                    const float message_baseline = skb_layout_get_lines(message_layout)[0].baseline;
                    const vec2 text_pos = vec2(x + size + font_size * 0.4f - tb.x, code_baseline - message_baseline);

                    const size_t first = out.size();
                    engine.emit_layout(message_layout, text_pos, scale, out);
                    for (size_t qi = first; qi < out.size(); ++qi)
                    {
                        out[qi].color     = float4(color.x, color.y, color.z, 0.85f);
                        out[qi].has_color = true;
                    }
                    area_right = text_pos.x + tb.x + tb.width;
                }

                diagnostic_areas.push_back({ float4(x, y, area_right, y + size), di, true });
            }
        }

        static void on_text_change(skb_editor_t*, skb_editor_text_change_reason_t, void* context)
        {
            auto self = static_cast<Editor::Impl*>(context);
            self->syntax_dirty = true;
            self->content_size_dirty = true;
            self->diagnostics.clear();
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

        impl->font_size = style.size;
        impl->style     = style;
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

        // Enter can split an image paragraph; one undo step with the cleanup.
        // (Skribidi's Enter inserts without amending undo, so it is safe
        // inside a fresh transaction, unlike insert_codepoint.)
        if (key == Key::Enter)
        {
            const int32_t transaction = skb_editor_undo_transaction_begin(impl->editor);
            skb_editor_process_key_pressed(impl->editor, engine.temp, skb_key, mods);
            impl->strip_duplicate_images(engine.temp);
            skb_editor_undo_transaction_end(impl->editor, transaction);
            return;
        }

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

        // Pasted lines can split an image paragraph; see key().
        const int32_t transaction = skb_editor_undo_transaction_begin(impl->editor);
        skb_editor_insert_text_utf8(impl->editor, engine.temp, current_selection, utf8.data(), (int32_t)utf8.size());
        impl->strip_duplicate_images(engine.temp);
        skb_editor_undo_transaction_end(impl->editor, transaction);
    }

    void Editor::set_caret(uint32_t line, uint32_t column)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_t* e = impl->editor;

        const int32_t count = skb_editor_get_paragraph_count(e);
        if (count == 0) return;

        const int32_t pi = std::min<int32_t>((int32_t)line, count - 1);
        const skb_text_t* text = skb_editor_get_paragraph_text(e, pi);
        int32_t length = skb_text_get_utf32_count(text);
        const uint32_t* cps = skb_text_get_utf32(text);
        while (length > 0 && (cps[length - 1] == '\n' || cps[length - 1] == '\r')) --length;

        const int32_t offset = skb_editor_get_paragraph_global_text_offset(e, pi) + std::min<int32_t>((int32_t)column, length);
        const skb_text_position_t pos = { offset, SKB_AFFINITY_TRAILING };
        skb_editor_select(e, skb_text_range_t{ pos, pos });
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
        // Only the line under pos matters.
        const auto rects = selection_rects(pos.y - 1, pos.y + 1);
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

        impl->strip_duplicate_images(engine.temp);
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

        impl->strip_duplicate_images(engine.temp);
        skb_editor_undo_transaction_end(e, transaction);
    }

    void Editor::build(float scale, Layout& out, float visible_top, float visible_bottom)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        out.quads.clear();
        out.size = {};
        skb_editor_t* e = impl->editor;

        const int32_t count   = skb_editor_get_paragraph_count(e);
        const int32_t first   = impl->first_paragraph_reaching(visible_top);
        int32_t       last    = first;   // one past the last visible paragraph
        while (last < count && skb_editor_get_paragraph_offset(e, last).y <= visible_bottom)
            ++last;

        if (highlighter)
            impl->update_syntax(highlighter, last - 1);

        {
            PROFILE(L"text_visible_lines");
            for (int32_t pi = first; pi < last; pi++)
            {
                const skb_vec2_t offset = skb_editor_get_paragraph_offset(e, pi);

                // Colors are per paragraph: text offsets start at 0 there.
                const std::vector<uint32_t>* syntax = highlighter && pi < (int32_t)impl->syntax_lines.size()
                    ? &impl->syntax_lines[pi].colors : nullptr;
                engine.emit_layout(skb_editor_get_paragraph_layout(e, pi), vec2(offset.x, offset.y), scale, out.quads, syntax, 0);
            }
        }

        impl->build_diagnostics(engine, scale, out.quads, first, last);
        impl->collect_images(visible_top, visible_bottom);
        engine.rasterize_missing();
    }

    void Editor::insert_image(vec2 pos, uint32_t id, float height)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_t* e = impl->editor;

        const int32_t count = skb_editor_get_paragraph_count(e);
        if (count == 0) return;

        // The paragraph under pos.
        const int32_t hit = skb_editor_get_text_offset_from_text_position(e, skb_editor_hit_test(e, SKB_MOVEMENT_CARET, pos.x, pos.y));
        int32_t pi = count - 1;
        for (int32_t i = 0; i + 1 < count; i++)
            if (hit < skb_editor_get_paragraph_global_text_offset(e, i + 1)) { pi = i; break; }

        // End of its content, before its own line break.
        const skb_text_t* text = skb_editor_get_paragraph_text(e, pi);
        const uint32_t*   cps  = skb_text_get_utf32(text);
        int32_t length = skb_text_get_utf32_count(text);
        while (length > 0 && (cps[length - 1] == '\n' || cps[length - 1] == '\r')) --length;
        const int32_t split = skb_editor_get_paragraph_global_text_offset(e, pi) + length;

        auto at = [](int32_t offset) { return skb_text_position_t{ offset, SKB_AFFINITY_TRAILING }; };
        impl->has_images = true;
        const int32_t transaction = skb_editor_undo_transaction_begin(e);

        // A line break there makes the next paragraph the image's own (empty)
        // one. insert_paragraph, not insert_codepoint: the latter lets undo
        // amend the previous state, and inside a fresh transaction (no states
        // yet) Skribidi reads undo_states[-1].
        // Inserted text takes the paragraph attributes active at the *caret*,
        // not at the insert position: with the caret still inside an earlier
        // image paragraph, the split line would come out as a copy of that
        // image. Put the caret where the break goes first.
        skb_editor_select(e, skb_text_range_t{ at(split), at(split) });

        // No paragraph attribute here: it would cover both halves of the split,
        // the dropped-on line included. Set on the new paragraph alone below.
        skb_editor_insert_paragraph(e, engine.temp, skb_text_range_t{ at(split), at(split) }, skb_attribute_t{});

        const skb_text_range_t image_paragraph = { at(split + 1), at(split + 1) };
        skb_editor_set_paragraph_attribute(e, engine.temp, image_paragraph,
            skb_attribute_make_paragraph_padding(0, 0, height + Impl::image_gap, 0));
        skb_editor_set_paragraph_attribute(e, engine.temp, image_paragraph,
            skb_attribute_make_group_tag(id | Impl::image_tag_flag));

        // Caret back on the dropped-on line, for the same reason: typing from
        // inside the image paragraph would carry its attributes along.
        skb_editor_select(e, skb_text_range_t{ at(split), at(split) });
        skb_editor_undo_transaction_end(e, transaction);
    }

    std::vector<ImagePlacement> Editor::images() const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        return impl->image_placements;
    }

    void Editor::set_diagnostics(std::vector<Diagnostic> diagnostics)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        // Sorted, so drawing can find the visible lines' diagnostics by binary search.
        std::stable_sort(diagnostics.begin(), diagnostics.end(), [](const Diagnostic& a, const Diagnostic& b)
            { return std::tie(a.line, a.column, a.length) < std::tie(b.line, b.column, b.length); });
        impl->diagnostics = std::move(diagnostics);
    }

    std::string Editor::diagnostic_at(vec2 pos) const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        for (auto& area : impl->diagnostic_areas)
        {
            const float4& r = area.rect;
            if (!(pos.x >= r.x && pos.x < r.z && pos.y >= r.y && pos.y < r.w) || area.index >= impl->diagnostics.size())
                continue;

            if (!area.whole_line)
                return impl->diagnostics[area.index].message;

            // The line's diagnostics are adjacent (sorted); a tooltip taller
            // than the screen helps nobody, so long lists are cut.
            constexpr size_t max_listed = 20;
            const uint32_t line  = impl->diagnostics[area.index].line;
            auto by_line = [](const Diagnostic& d, uint32_t l) { return d.line < l; };
            const auto first = std::lower_bound(impl->diagnostics.begin(), impl->diagnostics.end(), line, by_line);
            const auto last  = std::lower_bound(first, impl->diagnostics.end(), line + 1, by_line);

            std::string all;
            for (auto it = first; it != last && size_t(it - first) < max_listed; ++it)
                all += (all.empty() ? "" : "\n") + it->message;
            if (size_t(last - first) > max_listed)
                all += "\n... " + std::to_string(size_t(last - first) - max_listed) + " more";
            return all;
        }
        return {};
    }

    uint32_t Editor::line_count() const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        return (uint32_t)skb_editor_get_paragraph_count(impl->editor);
    }

    bool Editor::is_empty() const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        const int32_t count = skb_editor_get_paragraph_count(impl->editor);
        return count == 0 || (count == 1 && skb_editor_get_paragraph_text_count(impl->editor, 0) == 0);
    }

    void Editor::build_line_numbers(float scale, float right_edge, Layout& out, float visible_top, float visible_bottom)
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_t* e = impl->editor;

        out.quads.clear();
        out.size = {};

        const int32_t count = skb_editor_get_paragraph_count(e);
        for (int32_t pi = impl->first_paragraph_reaching(visible_top); pi < count; pi++)
        {
            if (skb_editor_get_paragraph_offset(e, pi).y > visible_bottom) break;

            const skb_layout_t* line = skb_editor_get_paragraph_layout(e, pi);
            const skb_layout_t* number = engine.get_layout(std::to_string(pi + 1), impl->style);
            if (!line || !number || skb_layout_get_lines_count(line) == 0 || skb_layout_get_lines_count(number) == 0)
                continue;

            // Right-aligned, on the baseline of the line's first row.
            const skb_vec2_t  offset = skb_editor_get_paragraph_offset(e, pi);
            const skb_rect2_t bounds = skb_layout_get_bounds(number);
            const float baseline = offset.y + skb_layout_get_lines(line)[0].baseline;
            const vec2 pos = vec2(right_edge - bounds.x - bounds.width, baseline - skb_layout_get_lines(number)[0].baseline);

            engine.emit_layout(number, pos, scale, out.quads);
        }

        engine.rasterize_missing();
    }

    vec2 Editor::content_size() const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);

        if (!impl->content_size_dirty)
            return impl->cached_content_size;

        vec2 size = {};
        for (int32_t pi = 0; pi < skb_editor_get_paragraph_count(impl->editor); pi++)
        {
            const skb_vec2_t  offset = skb_editor_get_paragraph_offset(impl->editor, pi);
            const skb_rect2_t bounds = skb_layout_get_bounds(skb_editor_get_paragraph_layout(impl->editor, pi));
            size.x = std::max(size.x, offset.x + bounds.x + bounds.width);
            size.y = std::max(size.y, offset.y + skb_editor_get_paragraph_advance_y(impl->editor, pi));
        }

        impl->cached_content_size = size;
        impl->content_size_dirty  = false;
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

    std::vector<float4> Editor::selection_rects(float visible_top, float visible_bottom) const
    {
        auto& engine = *Engine::get().impl;
        std::lock_guard<std::mutex> lock(engine.m);
        skb_editor_t* e = impl->editor;

        std::vector<float4> rects;
        const int32_t count = skb_editor_get_paragraph_count(e);
        if (count == 0)
            return rects;

        // Clip the selection to the visible lines' text before asking for its
        // bounds: Skribidi walks every cluster of the range it is given.
        const skb_text_range_t selection = skb_editor_get_current_selection(e);
        int32_t start = skb_editor_get_text_offset_from_text_position(e, selection.start);
        int32_t end   = skb_editor_get_text_offset_from_text_position(e, selection.end);
        if (start > end) std::swap(start, end);
        if (start == end)
            return rects;

        const int32_t first = impl->first_paragraph_reaching(visible_top);
        if (first >= count)
            return rects;
        int32_t last = first;
        while (last + 1 < count && skb_editor_get_paragraph_offset(e, last + 1).y <= visible_bottom)
            ++last;

        const int32_t visible_start = skb_editor_get_paragraph_global_text_offset(e, first);
        const int32_t visible_end   = skb_editor_get_paragraph_global_text_offset(e, last) + skb_editor_get_paragraph_text_count(e, last);
        start = std::max(start, visible_start);
        end   = std::min(end, visible_end);
        if (start >= end)
            return rects;

        const skb_text_range_t range = { { start, SKB_AFFINITY_TRAILING }, { end, SKB_AFFINITY_TRAILING } };
        skb_editor_iterate_text_range_bounds(e, range,
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

        // Sorted, not pairwise: a select-all over a large file is thousands of
        // rects. Group by line (vertical centre), sort each group by x, merge in
        // one pass.
        std::sort(rects.begin(), rects.end(), [](const float4& a, const float4& b) { return a.y + a.w < b.y + b.w; });

        std::vector<float4> merged;
        merged.reserve(rects.size());
        for (size_t begin = 0; begin < rects.size(); )
        {
            size_t end = begin + 1;
            while (end < rects.size() && same_line(rects[begin], rects[end])) ++end;

            std::sort(rects.begin() + begin, rects.begin() + end, [](const float4& a, const float4& b) { return a.x < b.x; });
            float4 current = rects[begin];
            for (size_t i = begin + 1; i < end; ++i)
            {
                if (touching(current, rects[i]))
                    current = float4(std::min(current.x, rects[i].x), std::min(current.y, rects[i].y),
                                     std::max(current.z, rects[i].z), std::max(current.w, rects[i].w));
                else
                {
                    merged.push_back(current);
                    current = rects[i];
                }
            }
            merged.push_back(current);
            begin = end;
        }
        return merged;
    }
}
