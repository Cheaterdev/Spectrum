module GUI:OptionBox;
import :Renderer;
import :CheckBoxText;

GUI::Elements::check_box_text::ptr GUI::Elements::option_group::create_option(bool value)
{
    check_box_text::ptr result(new check_box_text());
    result->docking = GUI::dock::TOP;
    result->get_check()->set_checked(value);
    add_option(result->get_check());
    return result;
}

GUI::Elements::toggle_switch::ptr GUI::Elements::option_group::add_option(toggle_switch::ptr obj)
{
    all.push_back(obj);
    obj->allow_uncheck = false;
    obj->on_toggle = [this, obj](bool value) { if (value) on_check(obj); };
    return obj;
}

GUI::Elements::option_group::option_group()
{
}

GUI::Elements::option_group::~option_group()
{
}

void GUI::Elements::option_group::on_check(toggle_switch::ptr obj)
{
    for (auto& c : all)
    {
        if (c != obj)
            c->set_checked(false);
    }
}
