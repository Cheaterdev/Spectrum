module GUI:StatusBar;
import :Renderer;


GUI::Elements::status_bar::status_bar()
{
    size = { 25, 25 };
    docking = dock::BOTTOM;
    skin = Skin::get().DefaultStatusBar;
}

void GUI::Elements::status_bar::draw(Context& c)
{
    c.renderer->draw(c, skin.Normal, get_render_bounds());
}

void GUI::Elements::status_bar::add_child(base::ptr obj)
{
    base::add_child(obj);
    obj->docking = dock::LEFT;
}
