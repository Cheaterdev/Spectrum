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
        };

        // ── Construction / destruction ─────────────────────────────────────────

        stat_graph::stat_graph()
        {
            docking     = dock::TOP;
            height_size = size_type::FIXED;
            width_size  = size_type::MATCH_PARENT;
            size        = {0, 60};

            header_label.reset(new label());
            header_label->docking     = dock::NONE;
            header_label->x_type      = pos_x_type::LEFT;
            header_label->y_type      = pos_y_type::TOP;
            header_label->pos         = {4, 2};
            header_label->width_size  = size_type::MATCH_PARENT;
            header_label->height_size = size_type::FIXED;
            header_label->size        = {0, 14};
            header_label->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
            header_label->color       = {0.9f, 0.9f, 0.9f, 0.85f};
            add_child(header_label);

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
            float half = std::max(half_data, half_min);
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

            update_label();
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

        void stat_graph::update_label()
        {
            header_label->visible = show_stats;
            if (!show_stats) return;

            char buf[128];
            float cur = current_value();
            float avg = average_value();

            const char* u   = unit.empty() ? "" : unit.c_str();
            const char* usp = unit.empty() ? "" : " ";

            if (title.empty())
                snprintf(buf, sizeof(buf), "%.1f%s%s  avg %.1f%s%s", cur, usp, u, avg, usp, u);
            else
                snprintf(buf, sizeof(buf), "%s: %.1f%s%s  avg %.1f%s%s",
                    title.c_str(), cur, usp, u, avg, usp, u);

            header_label->text = buf;
        }

        // ── GPU rendering hooks ────────────────────────────────────────────────

        void stat_graph::on_pre_render(Context& c)
        {
            if (count == 0) return;
            auto bounds = get_render_bounds();
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

            if (gpu->size != tex_size)
            {
                auto tex = std::make_shared<HAL::Texture>(device,
                    HAL::ResourceDesc::Tex2D(
                        HAL::Format::R8G8B8A8_UNORM,
                        {(UINT)tex_size.x, (UINT)tex_size.y}, 1, 0,
                        HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess | HAL::ResFlags::RenderTarget));
                tex->resource->set_name("stat_graph::output");
                gpu->output_view  = tex->texture_2d();
                gpu->output_owner = tex;
                gpu->size = tex_size;
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

                RT::SingleColor rt;
                rt.GetColor() = gpu->output_view.renderTarget;
                graphics.set_rtv(rt, HAL::RTOptions::Default | HAL::RTOptions::ClearColor,
                                 0, 0, background_color);

                Slots::StatGraphLine sl;
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
                graphics.set_pipeline<PSOS::StatGraphLines>();
                graphics.set_topology(HAL::PrimitiveTopologyType::LINE, HAL::PrimitiveTopologyFeed::STRIP);
                graphics.draw((uint32_t)count);
            }
            else
            {
                // ── Compute shader with gradient fill ──────────────────────────
                auto& compute = list->get_compute();
                compute.set_signature(Layouts::DefaultLayout);

                Slots::StatGraph sg;
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
                compute.set_pipeline<PSOS::StatGraph>();
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
