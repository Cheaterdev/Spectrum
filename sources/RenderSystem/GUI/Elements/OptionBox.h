#pragma once
#include "CheckBoxText.h"

namespace GUI
{
    namespace Elements
    {
        class option_group : public base
        {
            public:

                using ptr = s_ptr<option_group>;
                using wptr = w_ptr<option_group>;

            protected:
                std::vector<check_box::ptr> all;
                void on_check(check_box::ptr obj);
            public:

                option_group();
                ~option_group()
                {
                    for (auto c : all)
                        c->on_check_internal = nullptr;
                }
                check_box::ptr add_option(check_box::ptr obj);

                check_box_text::ptr create_option(bool value = false);

        };
    }
}