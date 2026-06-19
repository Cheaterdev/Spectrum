export module GUI:StatGraph;
import :Base;
import :Label;

export namespace GUI
{
    namespace Elements
    {
        class stat_graph : public base
        {
            std::vector<float> buffer;
            size_t head  = 0;
            size_t count = 0;

            label::ptr header_label;

            void update_label();

            float get_sample(size_t i) const; // i=0 oldest, i=count-1 newest

        public:
            using ptr  = s_ptr<stat_graph>;
            using wptr = w_ptr<stat_graph>;

            // Ring buffer capacity (number of samples retained)
            size_t capacity = 128;

            // Scale: auto fits visible range to buffer contents; fixed uses fixed_min/fixed_max
            bool  auto_scale = true;
            float fixed_min  = 0.0f;
            float fixed_max  = 100.0f;

            float4 background_color = {0.08f, 0.08f, 0.08f, 1.0f};
            float4 line_color        = {0.30f, 0.85f, 0.30f, 1.00f};
            float4 fill_color        = {0.30f, 0.85f, 0.30f, 0.18f};

            std::string title;
            std::string unit;

            bool show_stats = true; // overlay label with current + average

            stat_graph();

            void push(float v);

            float current_value() const;
            float average_value() const;
            float min_stored()    const;
            float max_stored()    const;

            label::ptr get_header_label() { return header_label; }

            virtual void draw(Context& c) override;
        };
    }
}
