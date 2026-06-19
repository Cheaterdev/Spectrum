module GUI:VerticalLayout;
import :Renderer;

sizer GUI::Elements::layouts::vertical::update_layout(sizer r, float scale)
{
    return base::update_layout(r, scale);
}

void GUI::Elements::layouts::vertical::on_bounds_changed(const rect& r)
{
    base::on_bounds_changed(r);
}

GUI::Elements::layouts::vertical::vertical()
{
    size        = {0, 0};
    width_size  = size_type::MATCH_CHILDREN;
    height_size = size_type::MATCH_CHILDREN;
}

void GUI::Elements::layouts::vertical::add_child(base::ptr obj)
{
    base::add_child(obj);
    obj->docking = dock::TOP;
}
