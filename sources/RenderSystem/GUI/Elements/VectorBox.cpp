module GUI:VectorBox;

namespace GUI
{
    namespace Elements
    {
        vector_box::vector_box(int components)
        {
            static constexpr const char* names[] = { "X", "Y", "Z", "W" };
            static const float4 tints[] = {
                float4(200, 60, 60, 255) / 255.0f,
                float4(60, 160, 60, 255) / 255.0f,
                float4(60, 100, 210, 255) / 255.0f,
                float4(110, 110, 110, 255) / 255.0f,
            };

            components = std::clamp(components, 2, 4);
            for (int i = 0; i < components; i++)
            {
                auto name = std::make_shared<label>();
                name->text  = names[i];
                name->color = tints[i];
                name->magnet_text = FW1_CENTER | FW1_VCENTER;
                name->margin = { i ? 6.0f : 0.0f, 0, 2, 0 };
                add_child(name);

                auto box = std::make_shared<float_box>();
                box->size = { 60, box->size->y };
                box->on_value_change = [this](float) { on_value_change(get_value()); };
                add_child(box);
                boxes.push_back(box);
            }
        }

        float4 vector_box::get_value() const
        {
            float4 v = { 0, 0, 0, 0 };
            for (size_t i = 0; i < boxes.size(); i++)
                v[i] = boxes[i]->get_value();
            return v;
        }

        void vector_box::set_value(float4 v)
        {
            for (size_t i = 0; i < boxes.size(); i++)
                boxes[i]->set_value(v[i]);
        }

        void vector_box::set_range(float min, float max)
        {
            for (auto& box : boxes)
            {
                box->min = min;
                box->max = max;
                box->set_value(box->get_value());   // re-clamp
            }
        }

        void vector_box::set_step(float step)
        {
            for (auto& box : boxes)
                box->step = step;
        }
    }
}
