#include "pch_render.h"
#include "Canvas.h"
#include "Comment.h"
#include "GUI/Elements/Button.h"
#include "GUI/Renderer/Renderer.h"

void GUI::Elements::FlowGraph::comment::draw(Graphics::context& c)
{
    this->renderer->draw(c, Skin::get().FlowComment.Active, get_render_bounds());
}

GUI::Elements::FlowGraph::comment::comment(canvas* canva, ::FlowGraph::window* wnd)
{
    thinkable = true;
    this->wnd = wnd;
    this->canva = canva;
    pos = wnd->pos;
    size = wnd->size;
    margin = { 20, 20, 20, 20 };
    clickable = false;
    clip_child = true;
    label_text.reset(new label());
    label_text->text = "textures ";
    label_text->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
    label_text->docking = dock::FILL;
    label_text->font_size = 24;
    title_bar.reset(new dragger());
    title_bar->size = vec2(0, 25);
    //title_bar->width_size = size_type::MATCH_PARENT;
    title_bar->docking = dock::TOP;
    //holder.reset(this);
    title_bar->target = this;
    //title_bar->draw_helper = true;
    title_bar->on_move = [canva, this](vec2, vec2 dt)
    {
        canva->on_move(this, dt);
    };
    title_bar->on_start_drag = [canva, this]()
    {
        canva->on_start_drag(this);
    };
    title_bar->on_end_drag = [canva, this]()
    {
        inner_components.clear();
    };
    base::add_child(title_bar);
    //title_bar->margin = {};
    title_bar->padding = { 5, 0, 0, 0 };
    minimal_size = { 50, 50 };
    close_button.reset(new button());
    close_button->size = { 25, 25 };
    close_button->padding = { 0, 0, 0, 0 };
    close_button->background_style = button::view_style::NO_BACKGROUND;
    close_button->get_label()->visible = false;
    close_button->get_image()->visible = true;
    close_button->get_image()->texture = Graphics::Texture::get_resource(HAL::texure_header(to_path(L"textures/gui/window_close.png")));
    close_button->get_image()->docking = dock::FILL;
    close_button->padding = { 5, 5, 5, 5 };
    close_button->docking = dock::RIGHT;
    close_button->on_click = [this, wnd](button::ptr)
    {
       wnd->remove();
    };
    title_bar->add_child(close_button);
    title_bar->add_child(label_text);
    //	contents_left->draw_helper = true;
    //contents_right->draw_helper = true;
}


void GUI::Elements::FlowGraph::comment::on_pos_changed(const vec2& r)
{
    base::on_pos_changed(r);
    wnd->pos = pos.get();
}

void GUI::Elements::FlowGraph::comment::on_size_changed(const vec2& r)
{
    base::on_size_changed(r);
    wnd->size = size.get();
}

void GUI::Elements::FlowGraph::comment::on_touch()
{
    base::on_touch();

    if (!selected)
        canva->on_select(nullptr);

   to_front(); 
}

void GUI::Elements::FlowGraph::comment::think(float dt)
{
    pos = wnd->pos;
    size = wnd->size;
}
