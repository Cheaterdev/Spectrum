export module GUI:ProgressBar;
import :Base;

export namespace GUI
{
    namespace Elements
    {
        class progress_bar : public base
        {
        public:
            using ptr  = s_ptr<progress_bar>;
            using wptr = w_ptr<progress_bar>;

            property<float> value;                      // 0..1
            float4 track_color = {0.15f, 0.15f, 0.15f, 1.0f};
            float4 fill_color  = {0.25f, 0.55f, 1.0f,  1.0f};

            Events::Event<float> on_change;

            progress_bar();

            virtual void draw(Context& c) override;
        };
    }
}
