#pragma once
#include "ScrollContainer.h"

namespace GUI
{

    namespace Elements
    {
        class list_element : public base
        {

            public:
                using ptr = s_ptr<list_element>;
                using wptr = w_ptr<list_element>;
                std::function<void(list_element::ptr)> on_select;
                bool selected = false;


                virtual void draw(Render::context& c) override;

                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

        };



        class list_box : public scroll_container
        {

                std::vector<list_element::ptr > elements;


            public:
                //	class renderer;
                //friend class renderer;
                using ptr = s_ptr<list_box>;
                using wptr = w_ptr<list_box>;
                //	std::function<void(void)> on_click;
                //		bool draw_background;
                list_box();

                label::ptr add_item(std::string elem);
                virtual void draw(Render::context& c) override;

                /*	virtual void on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

                	virtual void on_bounds_changed(const rect &r) override;

                	label::ptr get_label()
                	{
                		return label_text;
                	}*/
        };
    }
}