#include "pch.h"
#include "StatusBar.h"
#include "../Renderer/Renderer.h"

void GUI::Elements::status_bar::draw(DX12::context& c)
{
    renderer->draw(c, skin.Normal, get_render_bounds());
}

void GUI::Elements::status_bar::add_child(base::ptr obj)
{
    base::add_child(obj);
    obj->docking = dock::LEFT;
}
