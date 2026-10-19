module GUI:StatGraph;
import :Renderer;
import RenderSystem;
import HAL;
 import Graphics;

namespace GUI
{
    namespace Elements
    {
        // ── GPU resource bundle (pimpl) ────────────────────────────────────────
        // NOTE: HAL::Texture is forward-declared in Label.ixx and that declaration
        // leaks through the intra-module import chain, so we can't use
        // HAL::Texture::ptr here. Use a type-erased shared_ptr for lifetime
        // management and HAL::Texture2DView for the GPU handles instead.
        struct stat_graph_gpu
        {
            std::shared_ptr<void> output_owner; // keeps HAL::Texture alive
            HAL::Texture2DView    output_view;  // SRV + UAV handles
            ivec2                 size = {0, 0};
            // Optimized clear value baked into the texture. Recreate if the
            // graph's background colour changes, or the clear would mismatch.
            float4                clear_color = {-1, -1, -1, -1};
        };

        // ── Construction / destruction ─────────────────────────────────────────

        stat_graph::stat_graph()
        {
            docking     = dock::TOP;
            height_size = size_type::FIXED;
            width_size  = size_type::MATCH_PARENT;
            size        = {0, 60};

            // Labels size themselves to their text (label::on_text_changed); layout_labels()
            // then widens them to their slot and positions them, so nothing is left to the
            // dock/size machinery that used to flip between text width and graph width.
            auto make_label = [this](float font, float4 color) -> label::ptr {
                auto l = label::ptr(new label());
                l->docking     = dock::NONE;
                l->x_type      = pos_x_type::LEFT;
                l->y_type      = pos_y_type::TOP;
                l->width_size  = size_type::FIXED;
                l->height_size = size_type::FIXED;
                l->magnet_text = FW1_LEFT | FW1_TOP | FW1_NOWORDWRAP;
                l->font_size   = font;
                l->color       = color;
                l->shadow_color = {0.0f, 0.0f, 0.0f, 0.85f};
                add_child(l);
                return l;
            };

            // Colours follow line_color and are applied in layout_labels.
            header_label = make_label(15.0f, line_color);
            _cur.lbl     = make_label(15.0f, line_color);
            _avg.lbl     = make_label(13.0f, line_color);
            _max.lbl     = make_label(13.0f, line_color);
            _min.lbl     = make_label(13.0f, line_color);

            draw_tex.mul_color = {1, 1, 1, 1};
            draw_tex.add_color = {0, 0, 0, 0};
        }

        stat_graph::~stat_graph() = default; // needs to be in .cpp for unique_ptr<stat_graph_gpu>

        // ── Ring-buffer helpers ────────────────────────────────────────────────

        float stat_graph::get_sample(size_t i) const
        {
            return buffer[(head + capacity - count + i) % capacity];
        }

        void stat_graph::compute_auto_range(float& vmin, float& vmax) const
        {
            vmin = vmax = get_sample(0);
            for (size_t i = 1; i < count; ++i)
            {
                float v = get_sample(i);
                if (v < vmin) vmin = v;
                if (v > vmax) vmax = v;
            }
            float mid       = (vmin + vmax) * 0.5f;
            float half_data = (vmax - vmin) * 0.5f;
            float half_min  = std::max(std::abs(mid), std::abs(vmax)) * 0.05f;
            if (half_min < 1.0f) half_min = 1.0f;
            float half = std::max({half_data, half_min, min_range * 0.5f});
            float pad  = half * 0.15f;
            vmin = mid - half - pad;
            vmax = mid + half + pad;
        }

        void stat_graph::get_range(float& vmin, float& vmax) const
        {
            if (auto_scale)
            {
                vmin = _smooth_min;
                vmax = _smooth_max;
            }
            else
            {
                vmin = fixed_min;
                vmax = fixed_max;
            }
        }

        // ── Public API ─────────────────────────────────────────────────────────

        void stat_graph::push_timed(float v, float dt)
        {
            _sample_pending = v;
            if (sample_interval <= 0.0f) { push(v, dt); return; }
            _sample_accum += dt;
            if (_sample_accum >= sample_interval)
            {
                _sample_accum = std::fmod(_sample_accum, sample_interval);
                push(_sample_pending, dt);
            }
        }

        void stat_graph::push(float v, float dt)
        {
            auto bounds = get_render_bounds();
            if (bounds.w >= 4.0f)
                capacity = (size_t)(bounds.w / 4.0f);

            if (buffer.empty())
            {
                buffer.assign(capacity, 0.0f);
                head  = 0;
                count = 0;
            }
            else if (buffer.size() != capacity)
            {
                size_t keep  = std::min(count, capacity);
                size_t start = (head + buffer.size() - keep) % buffer.size();
                size_t tail  = buffer.size() - start;

                std::vector<float> tmp;
                tmp.reserve(capacity);
                std::ranges::copy(buffer | std::views::drop(start) | std::views::take(keep),
                                  std::back_inserter(tmp));
                std::ranges::copy(buffer | std::views::take(keep > tail ? keep - tail : 0),
                                  std::back_inserter(tmp));
                tmp.resize(capacity, 0.0f);
                buffer.swap(tmp);
                head  = keep % capacity;
                count = keep;
            }
            buffer[head] = v;
            head = (head + 1) % capacity;
            if (count < capacity) ++count;

            // Smooth display range toward actual data range
            if (auto_scale && count > 0)
            {
                float tmin, tmax;
                compute_auto_range(tmin, tmax);
                if (!_range_valid || range_smooth <= 0.0f || dt <= 0.0f)
                {
                    _smooth_min = tmin;
                    _smooth_max = tmax;
                    _range_valid = true;
                }
                else
                {
                    float alpha = 1.0f - std::exp(-range_smooth * dt);
                    _smooth_min += (tmin - _smooth_min) * alpha;
                    _smooth_max += (tmax - _smooth_max) * alpha;
                }
            }

            update_label(dt);
        }

        float stat_graph::current_value() const
        {
            if (count == 0) return 0.0f;
            return buffer[(head + capacity - 1) % capacity];
        }

        float stat_graph::average_value() const
        {
            if (count == 0) return 0.0f;
            float sum = 0.0f;
            for (size_t i = 0; i < count; ++i) sum += get_sample(i);
            return sum / (float)count;
        }

        float stat_graph::min_stored() const
        {
            if (count == 0) return 0.0f;
            float m = get_sample(0);
            for (size_t i = 1; i < count; ++i) m = std::min(m, get_sample(i));
            return m;
        }

        float stat_graph::max_stored() const
        {
            if (count == 0) return 0.0f;
            float m = get_sample(0);
            for (size_t i = 1; i < count; ++i) m = std::max(m, get_sample(i));
            return m;
        }

        void stat_graph::update_label(float dt)
        {
            for (label::ptr l : {header_label, _cur.lbl, _avg.lbl, _max.lbl, _min.lbl})
                l->visible = show_stats;
            if (!show_stats) return;

            // The texts are rewritten a few times a second, not per frame: a value that
            // changes every frame is unreadable, and every rewrite re-lays-out the glyphs.
            constexpr float refresh_interval = 0.25f;
            _label_accum += dt;
            if (dt > 0.0f && _label_accum < refresh_interval) return;
            _label_accum = 0.0f;

            const std::string u   = unit.empty() ? "" : " " + unit;
            auto fmt = [&](const char* prefix, float v) {
                char buf[64];
                snprintf(buf, sizeof(buf), "%s%.1f%s", prefix, v, u.c_str());
                return std::string(buf);
            };

            // Setting the text makes the label size itself to it; the slot only grows.
            auto set = [](readout& r, const std::string& s) {
                r.lbl->text = s;
                r.slot_w = std::max(r.slot_w, vec2(r.lbl->size).x);
            };

            header_label->text = title;
            set(_cur, fmt("", current_value()));
            set(_avg, fmt("avg ", average_value()));
            // Actual data min/max, not the padded display range.
            set(_min, fmt("min ", min_stored()));
            set(_max, fmt("max ", max_stored()));
        }

        void stat_graph::layout_labels(float w, float h)
        {
            if (!show_stats) return;

            constexpr float pad = 4.0f, gap = 10.0f;
            auto height = [](const label::ptr& l) { return vec2(l->size).y; };

            // Slot widths come from the ratchet, never from the current text, so the
            // positions below are stable between refreshes.
            auto place = [&](const label::ptr& l, float slot_w, float x, float row_y, float row_h) {
                const float lh = height(l);
                l->size = {std::max(slot_w, vec2(l->size).x), lh};
                l->pos  = {x, row_y + (row_h - lh) * 0.5f};
            };

            // Everything lives in one bottom row, all in the graph's own colour:
            // "title value" on the left, "avg min max" on the right.
            const float3 rgb = {line_color.x, line_color.y, line_color.z};
            header_label->color = {rgb.x, rgb.y, rgb.z, 0.85f};
            _cur.lbl->color     = {rgb.x, rgb.y, rgb.z, 1.0f};
            for (readout* r : {&_avg, &_min, &_max})
                r->lbl->color = {rgb.x, rgb.y, rgb.z, 0.85f};

            const float title_w = vec2(header_label->size).x;
            const float row_h   = std::max({height(header_label), height(_cur.lbl), height(_avg.lbl),
                                            height(_min.lbl), height(_max.lbl)});
            const float row_y   = h - row_h - 2.0f;

            place(header_label, title_w, pad, row_y, row_h);
            const float cur_x = pad + title_w + 6.0f;
            place(_cur.lbl, _cur.slot_w, cur_x, row_y, row_h);

            // Right to left, max first; a readout that would run into the left group
            // (or one already dropped) is hidden instead of overlapping.
            const float left_limit = cur_x + _cur.slot_w + gap;
            float x = w - pad;
            bool  fits = true;
            for (readout* r : {&_max, &_min, &_avg})
            {
                x -= r->slot_w;
                fits = fits && x >= left_limit;
                r->lbl->visible = fits;
                if (fits)
                    place(r->lbl, r->slot_w, x, row_y, row_h);
                x -= gap;
            }
        }

        // ── GPU rendering hooks ────────────────────────────────────────────────

        void stat_graph::on_pre_render(Context& c)
        {
            auto bounds = get_render_bounds();
            // Label positions are in layout units; the bounds are pixels.
            const float layout_scale = result_scale * scale;
            layout_labels(bounds.w / layout_scale, bounds.h / layout_scale);

            if (count == 0) return;
            if (bounds.w < 4.0f || bounds.h < 4.0f) return;
            user_ui->pre_draw_infos.emplace_back(this);
        }

        void stat_graph::pre_draw(HAL::CommandList::ptr list)
        {
            ivec2 tex_size = ivec2(get_render_bounds().size);
            if (tex_size.x < 4 || tex_size.y < 4) return;

            auto& device = RenderSystem::get().device();

            // ── Lazy-init / resize GPU resources ──────────────────────────────
            if (!gpu)
                gpu = std::make_unique<stat_graph_gpu>();

            bool clear_changed = gpu->clear_color.x != background_color.x
                              || gpu->clear_color.y != background_color.y
                              || gpu->clear_color.z != background_color.z
                              || gpu->clear_color.w != background_color.w;

            if (gpu->size != tex_size || clear_changed)
            {
                // Bake background_color as the optimized clear value — the lines
                // path clears this RT to exactly that colour, and a mismatch
                // costs a slow clear (D3D12 #820).
                auto tex = std::make_shared<HAL::Texture>(device,
                    HAL::ResourceDesc::Tex2D(
                        HAL::Format::R8G8B8A8_UNORM,
                        {(UINT)tex_size.x, (UINT)tex_size.y}, 1, 1,
                        HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess | HAL::ResFlags::RenderTarget),
                    HAL::TextureLayout::UNDEFINED, background_color);
                tex->resource->set_name("stat_graph::output");
                gpu->output_view  = tex->texture_2d();
                gpu->output_owner = tex;
                gpu->size = tex_size;
                gpu->clear_color = background_color;
            }

            // ── Upload samples into transient frame memory ─────────────────────
            std::vector<float> ordered(count);
            for (size_t i = 0; i < count; ++i)
                ordered[i] = get_sample(i);
            auto sample_data = list->place_data(count * sizeof(float), sizeof(float));
            list->write(sample_data, ordered);

            // ── Compute range ──────────────────────────────────────────────────
            float vmin, vmax;
            get_range(vmin, vmax);

            auto samples_view = sample_data.resource->create_view<HAL::StructuredBufferView<float>>(
                *list, HAL::StructuredBufferViewDesc{sample_data.resource_offset, sample_data.size, HAL::counterType::NONE});

            float4 bg_top = background_color;
            float4 bg_bot = float4(
                std::min(background_color.x * 1.4f, 1.0f),
                std::min(background_color.y * 1.4f, 1.0f),
                std::min(background_color.z * 1.4f, 1.0f),
                background_color.w);

            if (mode == draw_mode::lines)
            {
                // ── Graphics LINE_STRIP ────────────────────────────────────────
                auto& graphics = list->get_graphics();
                graphics.set_signature(Layouts::DefaultLayout);

                RT::Frame::SingleColor rt;
                rt.GetColor() = gpu->output_view.renderTarget;
                graphics.set_rtv(rt, HAL::RTOptions::Default | HAL::RTOptions::ClearColor,
                                 0, 0, background_color);

                Slots::Dev::StatGraphLine sl;
                sl.GetCount()     = (uint32_t)count;
                sl.GetWidth()     = (uint32_t)tex_size.x;
                sl.GetHeight()    = (uint32_t)tex_size.y;
                sl.GetVmin()      = vmin;
                sl.GetVmax()      = vmax;
                sl.GetLineColor()  = line_color;
                sl.GetFillColor()  = fill_color;
                sl.GetBgTop()      = bg_top;
                sl.GetBgBot()      = bg_bot;
                sl.GetSamples()   = samples_view.structuredBuffer;

                graphics.set(sl);
                graphics.set_pipeline<PSOS::Dev::StatGraphLines>();
                graphics.set_topology(HAL::PrimitiveTopologyType::LINE, HAL::PrimitiveTopologyFeed::STRIP);
                graphics.draw((uint32_t)count);
            }
            else
            {
                // ── Compute shader with gradient fill ──────────────────────────
                auto& compute = list->get_compute();
                compute.set_signature(Layouts::DefaultLayout);

                Slots::Dev::StatGraph sg;
                sg.GetCount()     = (uint32_t)count;
                sg.GetWidth()     = (uint32_t)tex_size.x;
                sg.GetHeight()    = (uint32_t)tex_size.y;
                sg.GetVmin()      = vmin;
                sg.GetVmax()      = vmax;
                sg.GetLineColor() = line_color;
                sg.GetFillTop()   = fill_color;
                sg.GetFillBot()   = float4(fill_color.x, fill_color.y, fill_color.z, 0.0f);
                sg.GetBgTop()     = bg_top;
                sg.GetBgBot()     = bg_bot;
                sg.GetSamples()   = samples_view.structuredBuffer;
                sg.GetOutput()    = gpu->output_view.rwTexture2D;

                compute.set(sg);
                compute.set_pipeline<PSOS::Dev::StatGraph>();
                compute.dispatch(tex_size, ivec2(8, 8));
            }

            // ── Update draw handle ─────────────────────────────────────────────
            draw_tex.texture = gpu->output_view;
            draw_tex.tc      = vec4{0, 0, 1, 1};
        }

        // ── Draw ───────────────────────────────────────────────────────────────

        void stat_graph::draw(Context& c)
        {
            rect b = get_render_bounds();

            if (gpu && gpu->output_owner && draw_tex.texture.texture2D)
            {
                // GPU texture is ready — draw it
                c.renderer->draw(c, draw_tex, b);
            }
            else
            {
                // Fallback: plain background until GPU texture is available
                c.renderer->draw_color(c, background_color, b);
            }
        }
    }
}
