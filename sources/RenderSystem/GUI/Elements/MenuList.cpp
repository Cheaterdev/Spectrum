module GUI:MenuList;
import :Renderer;

// TEMP-POPUPLOG: see Base.cpp.
namespace
{
    void popup_log(const std::string& line)
    {
        static std::mutex log_mutex;
        std::lock_guard<std::mutex> guard(log_mutex);
        std::ofstream out("popup_log.temp", std::ios::app);
        out << line << "\n";
    }
}


void GUI::Elements::menu_list::draw(Context& c)
{
    // The blurred backdrop (drawn by draw_before) replaces the skinned
    // container; without a target the container stays.
    if (backdrop_active(c))
        return;

    c.renderer->draw_container(get_ptr(), c);
}

GUI::Elements::menu_list::menu_list(bool vertical)
{
    this->vertical = vertical;
    // Popups blur what is behind them and sit slightly recessed; the menu strip is a flat bar.
    if (vertical)
    {
        add_effect<BackdropBlur>();
        add_effect<Shadow>();
        auto& light = add_effect<LightOverlay>();
        light.inner = true;
        light.radius = 0.0f;
    }
    padding = { 1, 1, 1, 1 };
    docking = GUI::dock::NONE;
    if (!vertical)
    {
        docking = dock::TOP;
        size = { 25, 25 };
    }
    else
    {

       // max_size = { 0, 300 };
        width_size = size_type::MATCH_CHILDREN;
        // Height is NOT MATCH_CHILDREN here: it's driven by auto_size (below),
        // which reads contents->scaled_size directly. filled->clamp_to_parent
        // clamps filled's height to *this* element's (possibly window-clamped,
        // see clamp_to_parent below) bounds, so filled's height can't also be
        // the source menu_list derives its own height from -- that'd be
        // circular (filled clamped to menu_list, menu_list sized from filled)
        // and get stuck at whatever tiny size happened to seed the loop.
        contents->width_size = size_type::MATCH_CHILDREN;
        contents->height_size = size_type::MATCH_CHILDREN;

        filled->width_size = size_type::MATCH_CHILDREN;
        filled->height_size = size_type::MATCH_CHILDREN;
          filled->docking= dock::NONE;
        filled->clamp_to_parent = ParentClamp::HEIGHT;
        auto_size = true;

        filled->x_type = pos_x_type::LEFT;
        if (hor) base::remove_child(hor);
       // filled->docking = GUI::dock::TOP;
      //  contents->docking = GUI::dock::NONE;
    }
	
    clamp_to_parent = ParentClamp::ALL;
    // size = { padding->left + padding->right, padding->top + padding->bottom };
}

void GUI::Elements::menu_list::close_menus()
{
    popup_log("  menu_list::close_menus vertical=" + std::to_string(vertical) + " in tree=" + std::to_string(get_parent() != nullptr) + " fading_out=" + std::to_string(fading_out));   // TEMP-POPUPLOG
    if (vertical)self_close();
}

bool GUI::Elements::menu_list::need_open_on_hover()
{
    return true;
}

void GUI::Elements::menu_list::make_fixed_width()
{
    width_size = size_type::MATCH_CHILDREN;
    // See the vertical-menu constructor for why height isn't MATCH_CHILDREN here.
    contents->width_size = size_type::MATCH_PARENT_CHILDREN;
    contents->height_size = size_type::MATCH_CHILDREN;

    filled->width_size = size_type::MATCH_PARENT_CHILDREN;
    filled->height_size = size_type::MATCH_CHILDREN;

    filled->clamp_to_parent = ParentClamp::HEIGHT;
    auto_size = true;
    filled->docking = dock::NONE;
    filled->x_type = pos_x_type::LEFT;
    if (hor) base::remove_child(hor);
}

bool GUI::Elements::menu_list::is_menu_component()
{
    return true;
}

bool GUI::Elements::menu_list::is_menu_open()
{
    for (auto c : elements)
    {
        if (c->is_menu_open())
            return true;
    }

    return false;
}

bool GUI::Elements::menu_list::is_self_open()
{
    return get_parent() != nullptr;
}

void GUI::Elements::menu_list::self_open(user_interface* ui)
{
    popup_log("    menu_list::self_open in tree=" + std::to_string(get_parent() != nullptr));   // TEMP-POPUPLOG
    // Closed a moment ago and still fading out: bring the same popup back
    // instead of adding it a second time.
    if (get_parent())
    {
        cancel_close();
        return;
    }

    ui->add_child(get_ptr());

    // Every popup (submenus, context menus, dropdowns) fades in as one group:
    // the blur, the shadow and the items together.
    layer_opacity = 0.0f;
    fade_to(1.0f, ui->fade_in_duration);
}

void GUI::Elements::menu_list::self_close()
{
    popup_log("    menu_list::self_close user_ui=" + std::to_string(user_ui != nullptr) + " fades=" + std::to_string(user_ui && user_ui->fade_enabled) + " in tree=" + std::to_string(get_parent() != nullptr) + " fading_out=" + std::to_string(fading_out));   // TEMP-POPUPLOG

    // Without fades (or a tree to draw it in) the popup just goes.
    if (!user_ui || !user_ui->fade_enabled || !get_parent())
    {
        remove_from_parent();
        return;
    }

    if (fading_out)
        return;

    fading_out = true;
    fade_to(0.0f, user_ui->fade_out_duration);

    // fade_to dropped any earlier callback; set ours after it.
    on_fade_end = [self = get_ptr<menu_list>()]()
    {
        popup_log("    menu_list fade ended: removing from the UI");   // TEMP-POPUPLOG
        self->fading_out = false;
        self->remove_from_parent();
        self->layer_opacity = 1.0f;
    };
}

void GUI::Elements::menu_list::cancel_close()
{
    fading_out = false;
    fade_to(1.0f, user_ui ? float(user_ui->fade_in_duration) : 0.1f);
}

void GUI::Elements::menu_list::remove_all()
{
    contents->remove_all();
    elements.clear();

    if (!vertical)
    {
        docking = dock::TOP;
        size = { 25, 25 };
    }

    else
        size = { padding->left + padding->right, padding->top + padding->bottom };
}

GUI::Elements::menu_list_element::menu_list_element(bool on_strip)
{
    auto& light = add_effect<LightOverlay>();
    light.reveal = true;
    light.border = true;
    light.ripple = true;
    light.radius = 0.0f;
    this->on_strip = on_strip;
}

GUI::Elements::menu_strip::menu_strip() : menu_list(false)
{
    draw_icon = false;
}

bool GUI::Elements::menu_strip::need_open_on_hover()
{
    return is_menu_open();
}


namespace GUI
{

    void Elements::menu_list_element::draw(Context& c)
    {
            if (is_hovered())
				c.renderer->draw_color(c, float4(53, 114, 202, 255) / 255.0f, get_render_bounds());
			else
				c.renderer->draw_color(c, float4(5, 5, 5, 150) / 255.0f, get_render_bounds());

    }

    bool Elements::menu_list_element::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
    {
        base::on_mouse_action(action, button, pos);

        //		if (on_select)
        //		on_select(std::static_pointer_cast<menu_list_element>(get_ptr()));
        if (button == mouse_button::LEFT && action == mouse_action::DOWN)
        {
            if (menu)
            {
                if (menu->get_parent())
                    close_menu();
                else
                    open_menu();
            }

            else
            {

                    user_ui->close_menus();

                if (on_click)
                    on_click(get_ptr<menu_list_element>());
            }
        }

        return true;
    }

    std::shared_ptr<Elements::menu_list> Elements::menu_list_element::get_menu()
    {
        if (!menu)
            menu.reset(new Elements::menu_list());

        return menu;
    }

    void Elements::menu_list_element::close_menus()
    {
        base::close_menus();
        close_menu();
    }

    bool Elements::menu_list_element::is_menu_open()
    {
        if (!menu)
            return false;

        return menu->get_parent() != nullptr;
    }

    void Elements::menu_list_element::open_menu()
    {
        if (menu)
      
        {

            // Fades in, or comes back if it is still fading out from a close.
            menu->self_open(user_ui);

            rect p = get_render_bounds();

            if (!on_strip)
                menu->pos = vec2(p.x + p.w, p.y);
            else
                menu->pos = vec2(p.x, p.y + p.h);


        }
    }

    void Elements::menu_list_element::close_menu()
    {
        if (menu)

        {
            menu->close_menus();

            // Fades out, then leaves the tree (immediately when fades are off).
            menu->self_close();


        }

    }
	
    void Elements::menu_list_element::on_mouse_enter(vec2 pos)
    {
        if (on_hover)
            on_hover(get_ptr<menu_list_element>());
    }

}

