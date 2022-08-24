#include "pch_render.h"
#include "Window.h"
#include "GUI/Renderer/Renderer.h"

void GUI::Elements::window::draw(Graphics::context& c)
{
    // skin->draw(this, c);
    renderer->draw(c, skin.Active, get_render_bounds());
    label_text->color = pressed ? rgba8(200, 200, 200, 255) : rgba8(255, 255, 255, 255);
}

GUI::Elements::window::window() : GUI::Elements::resizable()
{
    margin = { 20, 20, 20, 20 };
    clickable = false;
    clip_child = true;
    label_text.reset(new label());
    label_text->text = "This is a window";
    label_text->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
    label_text->docking = dock::FILL;
    title_bar.reset(new dragger());
    title_bar->size = vec2(0, 25);
    title_bar->docking = dock::TOP;
    //holder.reset(this);
    title_bar->target = this;
    base::add_child(title_bar);
    title_bar->padding = { 5, 0, 0, 5};
    minimal_size = { 50, 50 };
    close_button.reset(new button());
    close_button->size = { 25, 25 };
    close_button->padding = { 0, 0, 0, 0 };
    close_button->background_style = button::view_style::FLAT;
    close_button->get_label()->visible = false;
    close_button->get_image()->visible = true;
    close_button->get_image()->texture = Graphics::Texture::get_resource(Graphics::texure_header(to_path("textures/gui/window_close.png")));
    close_button->get_image()->docking = dock::FILL;
    close_button->get_image()->height_size = size_type::FIXED;
    close_button->get_image()->width_size = size_type::FIXED;
    close_button->get_image()->size = { close_button->get_image()->texture.texture->get_desc().Width, close_button->get_image()->texture.texture->get_desc().Height};
    close_button->padding = { 5, 5, 5, 5 };
    close_button->docking = dock::RIGHT;
    close_button->on_click = [this](button::ptr)
    {
        remove_from_parent();
    };
    title_bar->add_child(close_button);
    title_bar->add_child(label_text);
    contents.reset(new base());
    contents->docking = dock::FILL;
    contents->padding = { 0, 5, 0, 0 };
    base::add_child(contents);
    skin = Skin::get().DefaultWindow;
}

bool GUI::Elements::window::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
    base::on_mouse_action(action, button, pos);
    label_text->pos = pressed ? vec2(1, 1) : vec2(0, 0);
    return true;
}

void GUI::Elements::window::on_bounds_changed(const rect& r)
{
    base::on_bounds_changed(r);
    //label_text->size = vec2(label_text->size->x,80);
}

void GUI::Elements::window::add_child(base::ptr obj)
{
    contents->add_child(obj);
}

void GUI::Elements::window::remove_child(base::ptr obj)
{
    contents->remove_child(obj);
}

void GUI::Elements::window::on_touch()
{
    resizable::on_touch();
   to_front(); 
}




