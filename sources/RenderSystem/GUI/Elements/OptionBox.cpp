#include "pch_render.h"
#include "OptionBox.h"
#include "CheckBoxText.h"

GUI::Elements::check_box_text::ptr GUI::Elements::option_group::create_option(bool value /*= false*/)
{
    check_box_text::ptr result(new check_box_text());
    result->docking = dock::TOP;
    result->get_check()->set_checked(value);
    add_option(result->get_check());
    return result;
}

GUI::Elements::check_box::ptr GUI::Elements::option_group::add_option(check_box::ptr obj)
{
    all.push_back(obj);
    obj->allow_uncheck = false;
    obj->on_check_internal = [this, obj](bool value) {if (value)on_check(obj); };
    obj->skin =  Skin::get().DefaultOptionBox;
    return obj;
}

GUI::Elements::option_group::option_group()
{
}

void GUI::Elements::option_group::on_check(check_box::ptr obj)
{
    for (auto c : all)
    {
        if (c != obj)
            c->set_checked(false);
    }
}
