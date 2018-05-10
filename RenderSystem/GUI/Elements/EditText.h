#pragma once
#include <boost/functional/hash.hpp>

namespace GUI
{
    namespace Elements
    {
        class edit_cursor : public base
        {

                float time;
                virtual void draw(Render::context& c) override;

            public:



                using ptr = s_ptr<edit_cursor>;
                using wptr = w_ptr<edit_cursor>;

                edit_cursor();
        };

        class edit_text : public base
        {


            private:


                std::vector<long> keys;
                std::mutex m;
                void process_keys();
            protected:
                label::ptr label_text;
                edit_cursor::ptr label_cursor;

                unsigned int cursor_pos;
                std::string text;

                void update_caret();
            public:


                using ptr = s_ptr<edit_text>;
                using wptr = w_ptr<edit_text>;

                edit_text();

                virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

                virtual void on_key_action(long key) override;

                void draw(Render::context& c);

                virtual void on_mouse_enter(vec2 pos) override;

                virtual void on_mouse_leave(vec2 pos) override;


                //virtual void draw(Render::context &c) override;
        };




    }




}
