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

            // Animated look: the fill eases toward value, a soft highlight sweeps
            // along it, and the leading edge glows. Off = a plain flat bar.
            bool animated = true;
            // The eased fill actually drawn, 0..1.
            float displayed() const { return shown; }

            progress_bar();

            virtual void on_pre_render(Context& c) override;
            virtual void draw(Context& c) override;

        private:
            float shown = 0.0f;     // eased value actually drawn
            float sweep = 0.0f;     // 0..1 position of the highlight, wraps
            bool  first = true;     // the first frame snaps instead of easing from 0
        };
    }
}
