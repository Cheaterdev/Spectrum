export module GUI:SplitView;
import :Base;

export namespace GUI
{
    namespace Elements
    {
        // Two panels with a draggable divider between them. The split is kept
        // as a ratio, so it survives resizing; each panel keeps at least
        // min_panel logical units. Put content into first() and second().
        class split_view : public base
        {
                class divider;
                base::ptr first_panel, second_panel;
                s_ptr<divider> bar;

                void set_ratio_from(vec2 pos);   // pos in window pixels

            public:
                using ptr  = s_ptr<split_view>;
                using wptr = w_ptr<split_view>;

                // true: side by side, divider vertical; false: stacked.
                const bool horizontal;

                float ratio         = 0.5f;   // first panel's share of the space
                float min_panel     = 40;
                float divider_width = 5;

                // After a drag moved the divider. UI thread.
                std::function<void(float)> on_ratio_change;

                explicit split_view(bool horizontal = true);

                base::ptr first() const  { return first_panel; }
                base::ptr second() const { return second_panel; }

                void set_ratio(float r);

                virtual void update_childs_layout(sizer& r, float scale) override;
        };
    }
}
