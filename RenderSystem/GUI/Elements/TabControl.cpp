#include "pch.h"


namespace GUI
{
    namespace Elements
    {



        void tab_button::draw(Render::context& c)
        {
            if (is_current() || is_pressed())
                renderer->draw(c, skin.Pressed, get_render_bounds());
            else if (is_hovered())
                renderer->draw(c, skin.Hover, get_render_bounds());
            else
                renderer->draw(c, skin.Normal, get_render_bounds());
        }

        void tab_button::on_drag_start()
        {
            auto ui = user_ui;
            auto o = owner.lock();

            if (o)
                o->remove_button(get_ptr<tab_button>());

            owner.reset();
            ui->add_child(get_ptr());
            // add_child(page);
            size = render_bounds->size;
            pos = render_bounds->pos;
            docking = dock::NONE;
            visible = true;
            // clickable = false;
            clip_child = false;
            run_on_ui([this, ui]
            {

                page->docking = dock::NONE;
                page->size = { 300, 300 };
                page->visible = true;

                page->pos = { 0, size->y };

            });
        }

        void tab_button::on_drag_end()
        {
            visible = true;
            //     clickable = true;
        }

        tab_button::tab_button()
        {
            drag_n_drop_copy = false;
            click_style = click_styles::IMMEDIATELY;
            clip_to_parent = true;
            size = { 50, 50 };
            width_size = size_type::MATCH_CHILDREN;
            get_label()->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
            close_button.reset(new button());
            close_button->size = { 20, 20 };
            close_button->padding = { 0, 0, 0, 0 };
            close_button->background_style = button::view_style::FLAT;
            close_button->get_label()->visible = false;
            close_button->get_image()->visible = true;
            close_button->get_image()->texture = Render::Texture::get_resource(Render::texure_header("textures/gui/window_close.png"));
            close_button->get_image()->docking = dock::FILL;
            // close_button->padding = { 5, 5, 5, 5 };
            close_button->height_size = size_type::SQUARE;
            close_button->docking = dock::LEFT;
            close_button->on_click = [this](button::ptr)
            {
                run_on_ui([this] {owner.lock()->remove_button(get_ptr<tab_button>()); });
            };
            add_child(close_button);
            padding = { 2, 2, 2, 2 };
            get_label()->docking = dock::LEFT;
            set_package("tab_button");
            skin = Skin::get().TabButton;
        }

        void tab_strip::draw(Render::context& c)
        {
        //    renderer->draw(c, skin.Normal, get_render_bounds());
			renderer->draw_container(get_ptr(),c);
        }
		void tab_strip::draw_after(Render::context& c)
		{
		//	    renderer->draw(c, skin.Normal, get_render_bounds());
			//	renderer->draw_container(get_ptr(),c);
			auto bounds = get_render_bounds();
			bounds.y += bounds.h - 3;
			bounds.h = 3;
			renderer->draw_color(c, float4(53, 114, 202,255)/255.0f, bounds);
		}

		inline tab_strip::tab_strip()
		{
			padding = { 0, 0, 0, 3 };
			size = { 25, 25 };
			docking = dock::TOP;
			menu.reset(new menu_list());
			all.reset(new button());
			all->size = size.get();
			all->docking = dock::RIGHT;
			all->get_label()->visible = false;
			all->background_style = button::view_style::FLAT;
			all->get_image()->visible = true;
			all->get_image()->texture = Render::Texture::get_resource(Render::texure_header("textures/gui/down.png"));
			all->get_image()->docking = dock::FILL;
			all->padding = { 5, 5, 5, 5 };
			all->on_click = [this](button::ptr)
			{
				run_on_ui([this]() {    menu->self_open(user_ui); });
				menu->pos = vec2(all->get_render_bounds().pos) + vec2(0, all->get_render_bounds().h);
			};
			add_child(all);
			strip.reset(new base());
			strip->docking = dock::FILL;
			add_child(strip);
			skin = Skin::get().DefaultTabStrip;
		}

		void tab_strip::add_child(base::ptr obj)
        {
            tab_button::ptr but = std::dynamic_pointer_cast<tab_button>(obj);

            if (but)
            {
                strip->add_child(obj);
                buttons.push_back(but);
                obj->docking = dock::LEFT;
                recalculate_tabs();
            }

            else
                base::add_child(obj);
        }

        void tab_strip::remove_child(base::ptr obj)
        {
            tab_button::ptr but = std::dynamic_pointer_cast<tab_button>(obj);

            if (but)
            {
                strip->remove_child(obj);
                buttons.erase(but);
                recalculate_tabs();
            }

            else
                base::remove_child(obj);
        }


        void tab_strip::on_bounds_changed(const rect& r)
        {
            base::on_bounds_changed(r);
            recalculate_tabs();
        }

        bool tab_strip::can_accept(drag_n_drop_package::ptr package)
        {
            return package->name == "tab_button";
        }

        bool tab_strip::on_drop(drag_n_drop_package::ptr e, vec2 p)
        {
            run_on_ui([this, e, p]()
            {
                auto elem = e->element.lock();
                tab_button::ptr but = elem->get_ptr<tab_button>();
                owner->add_button(but);
                float w = get_render_bounds().x;
				size_t best = -1;
                float len = abs(get_render_bounds().x - p.x);
                auto& strip_buttons = strip->get_childs();
                strip_buttons.erase(elem);
                buttons.erase(elem->get_ptr<tab_button>());

                for (size_t i = 0; i < buttons.size(); i++)
                {
                    auto bounds = buttons[i]->get_render_bounds();
                    w += bounds.w;
                    float l = abs(w - p.x);

                    if (l < len)
                    {
                        len = l;
                        best = i;
                    }
                }

                strip_buttons.insert(strip_buttons.begin() + (best + 1), elem);
                buttons.insert(buttons.begin() + (best + 1), elem->get_ptr<tab_button>());
                recalculate_tabs();
                strip->set_update_layout();
            });
            //elem->on_add(strip.get());
            return true;
        }

        tab_button::ptr tab_control::add_page(std::string str, base::ptr obj /*= nullptr*/)
        {
            if (!obj)
                obj.reset(new base());

            tab_button::ptr but(new tab_button());
            but->get_label()->text = str;
            but->page = obj;
            return add_button(but);
        }
        tab_button::ptr tab_control::add_button(tab_button::ptr but)
        {
            if (!but) return nullptr;

            auto old_owner = but->owner.lock();

            if (old_owner.get() == this)
            {
                strip->recalculate_tabs();
                return but;
            }

            if (old_owner)
                old_owner->remove_button(but);

            but->owner = get_ptr<tab_control>();
            but->on_click = [this](button::ptr but)
            {
                tab_button::ptr tab_but = std::static_pointer_cast<tab_button>(but);
                show_page(tab_but);
            };
            strip->add_child(but);
            but->page->visible = false;
            //    if (!cur_page)
            show_page(but);
            run_on_ui([this, but]()
            {
                contents->add_child(but->page);
            });
            strip->recalculate_tabs();
            return but;
        }
		inline void tab_control::remove_button(tab_button::ptr b)
		{
			strip->remove_child(b);
			strip->recalculate_tabs();
			strip->set_update_layout();

			if (cur_page == b->page)
				show_page(strip->get_first_button());

			if (remove_if_empty && strip->buttons.size() == 0)
				remove_from_parent();

			if (b->page)
				b->page->remove_from_parent();
		}
        void tab_control::show_page(tab_button::ptr tab_but)
        {
            if (strip->current == tab_but)
                return;

            run_on_ui([this, tab_but]()
            {
                if (cur_page)
                    cur_page->visible = false;

                strip->current = tab_but;

                if (tab_but)
                    cur_page = tab_but->page;
                else
                    cur_page = nullptr;

                if (cur_page)
                {
                    cur_page->visible = true;
                    cur_page->docking = dock::FILL;
                }

                strip->recalculate_tabs();
            });
        }

        tab_control::tab_control()
        {
            strip.reset(new tab_strip());
            strip->owner = this;
            add_child(strip);
            contents.reset(new base);
            contents->docking = dock::FILL;
            add_child(contents);
        }

        void tab_control::draw(Render::context& c)
        {
            //  renderer->draw_container(get_ptr(), c);
        }

    }
}
