export module GUI:StatGraph;
import :Base;
import :Label;

// GPU resources held in pimpl (defined in .cpp; keeps HAL out of this interface)
namespace GUI { namespace Elements { struct stat_graph_gpu; } }

export namespace GUI
{
    namespace Elements
    {
        class stat_graph : public base
        {
        public:
            enum class draw_mode { cs, lines };
        private:
            std::vector<float> buffer;
            size_t head  = 0;
            size_t count = 0;

            float _sample_accum   = 0.0f;
            float _sample_pending = 0.0f;

            float _smooth_min  = 0.0f;
            float _smooth_max  = 0.0f;
            bool  _range_valid = false;

            // Each readout is its own label in a slot that only ever grows, so a
            // digit-count change cannot move its neighbours or resize its box.
            struct readout
            {
                label::ptr lbl;
                float      slot_w = 0.0f;
            };
            label::ptr header_label; // title, left
            readout    _cur, _avg, _max, _min;
            float      _label_accum = 1.0f; // seconds since the texts were last rewritten
            void update_label(float dt);
            void layout_labels(float w, float h);

            float get_sample(size_t i) const;
            void  compute_auto_range(float& vmin, float& vmax) const;
            void  get_range(float& vmin, float& vmax) const;

            // GPU resources (pimpl — HAL types live in .cpp only)
            std::unique_ptr<stat_graph_gpu> gpu;
            GUI::Texture draw_tex;

        public:
            using ptr  = s_ptr<stat_graph>;
            using wptr = w_ptr<stat_graph>;

            ~stat_graph();

            size_t capacity   = 128;
            bool   auto_scale = true;
            float  fixed_min  = 0.0f;
            float  fixed_max  = 100.0f;

            float4 background_color = {0.08f, 0.08f, 0.08f, 1.0f};
            float4 line_color        = {0.30f, 0.85f, 0.30f, 1.00f};
            float4 fill_color        = {0.30f, 0.85f, 0.30f, 0.30f};

            std::string title;
            std::string unit;
            bool show_stats = true;

            float sample_interval = 0.0f;
            float range_smooth    = 3.0f; // exp-decay speed toward target range; 0 = instant
            float min_range       = 0.0f; // minimum span of the auto-scale display range
            draw_mode mode = draw_mode::cs;

            stat_graph();

            void push(float v, float dt = 0.0f);
            void push_timed(float v, float dt);

            float current_value() const;
            float average_value() const;
            float min_stored()    const;
            float max_stored()    const;

            label::ptr get_header_label() { return header_label; }

            virtual void draw(Context& c) override;
            virtual void on_pre_render(Context& c) override;
            virtual void pre_draw(HAL::CommandList::ptr list) override;
        };
    }
}
