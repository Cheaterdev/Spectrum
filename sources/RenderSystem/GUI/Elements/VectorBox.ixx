export module GUI:VectorBox;
import :Base;
import :HorizontalLayout;
import :Label;
import :FloatBox;

export namespace GUI
{
    namespace Elements
    {
        // 2-4 float_boxes in a row, each behind an X/Y/Z/W label tinted like
        // the axis gizmo. Components past `components` read and write as 0.
        class vector_box : public layouts::horizontal
        {
                std::vector<float_box::ptr> boxes;

            public:
                using ptr  = s_ptr<vector_box>;
                using wptr = w_ptr<vector_box>;

                // Fires with the whole vector when any component commits. UI thread.
                Events::Event<float4> on_value_change;

                explicit vector_box(int components = 3);

                int            size_count() const { return (int)boxes.size(); }
                float_box::ptr component(int i) const { return boxes[i]; }

                float4 get_value() const;
                // Doesn't fire on_value_change.
                void   set_value(float4 v);

                // Applied to every component.
                void set_range(float min, float max);
                void set_step(float step);
        };
    }
}
