#pragma once
#include "GUI/GUI.h"
#include "GUI/Skin.h"

namespace GUI
{
    namespace Elements
    {
        class option_group;

        class check_box : public base
        {
                property<bool> checked;

                virtual void draw(Render::context& c) override;

            protected:
                virtual void on_check_changed(const bool& r);
                std::function<void(bool)> on_check_internal;
                friend class option_group;
            public:
                Skin::CheckBox skin;
                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;
                virtual void on_bounds_changed(const rect& r) override;
                bool allow_uncheck = true;

                bool is_checked()
                {
                    return checked.get();
                }

                void set_checked(bool value)
                {
                    checked = value;
                }
                using ptr = s_ptr<check_box>;
                using wptr = w_ptr<check_box>;
                std::function<void(bool)> on_check;
                check_box();

				check_box(Variable<bool> &);
        };
    }




}