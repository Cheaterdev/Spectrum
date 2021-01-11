#include "pch.h"
#include "Tree.h"
#include "../Renderer/Renderer.h"

class other_all: public GUI::base
{
    public:
        GUI::Elements::tree_element* owner;

        other_all(GUI::Elements::tree_element* owner)
        {
            this->owner = owner;
        }

        virtual void remove_child(ptr obj) override
        {
            base::remove_child(obj);

            if (childs.size() == 0)
                owner->on_empty();
        }


};


GUI::Elements::tree_element::ptr GUI::Elements::tree_element::add_tree(tree_creator::ptr creator, base_tree* elem)
{
    tree_element::ptr res(new tree_element(creator, elem));
    other->add_child(res);
//	res->padding = {25,0,0,0};
//	open_icon->visible = true;
  // open_icon->texture = tex_opened;
    // icon->texture = DX12::Texture::get_resource(DX12::texure_header("textures/gui/tree_folder.png"));
    // icon->size = { icon->texture.texture->get_desc().Width, icon->texture.texture->get_desc().Height };
    res->main_tree = main_tree;
    return res;
}

GUI::Elements::tree_element::tree_element(tree_creator::ptr creator, base_tree* elem)
{
    this->creator = creator;
     height_size = size_type::MATCH_CHILDREN;
    width_size = size_type::MATCH_PARENT;
    size = { 100, 0 };
    x_type = pos_x_type::LEFT;
    docking = dock::TOP;
    creator->init_element(this, elem);
    other.reset(new other_all(this));
    other->docking = dock::TOP;
    other->height_size = size_type::MATCH_CHILDREN;
    other->width_size = size_type::MATCH_PARENT;
    other->x_type = pos_x_type::LEFT;
    other->padding = {0, 2, 0, 2};
    add_child(other);
}
/*
bool GUI::Elements::tree_element::on_drop(GUI::drag_n_drop_package::ptr p, vec2)
{
	auto elem = dynamic_cast<line*>(p->element.lock().get());

	this->node->add_child(elem->owner->node);
	return true;
}

bool GUI::Elements::tree_element::can_accept(GUI::drag_n_drop_package::ptr p)
{

	bool is_tree = p->element.lock().get() != this&&p->name == "tree_item";

		if (!is_tree)
			return false;

		auto elem = dynamic_cast<line*>(p->element.lock().get());
		return !this->node->is_parent(elem->owner->node);
}*/

void GUI::Elements::tree_element::toogle()
{
    other->visible = !other->visible.get();

	on_toogle(other->visible);
//     if (other->get_childs().size())
 //       open_icon->texture = other->visible.get() ? tex_opened : tex_closed;
}

void GUI::Elements::tree_element::tree_on_remove()
{
    remove_from_parent();
}

void GUI::Elements::tree_element::tree_added_child(base_tree* child)
{
    run_on_ui([this, child]() {init(creator, child);});
}

GUI::Elements::tree_element::ptr GUI::Elements::tree::add_tree(base_tree* elem)
{
    tree_element::ptr res(new tree_element(creator, elem));
    contents->add_child(res);
    res->main_tree = get_ptr<GUI::Elements::tree>();
    return res;
}

GUI::Elements::tree::tree(tree_creator::ptr creator)
{
    contents->size = { 20, 00 };
    contents->width_size = size_type::MATCH_PARENT;
    contents->height_size = size_type::MATCH_CHILDREN;
    x_type = pos_x_type::LEFT;
    this->creator = creator;
}

bool GUI::Elements::toogle_icon::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
    if (button == mouse_button::LEFT)
        if (action == mouse_action::UP)
            owner->toogle();

    return true;
}

GUI::Elements::toogle_icon::toogle_icon(GUI::Elements::tree_element* owner)
{
    this->owner = owner;
	size = { 16,16 };
	width_size = size_type::FIXED;
	height_size = size_type::FIXED;

    clickable = true;

	tex_closed = DX12::Texture::get_resource(DX12::texure_header("textures/gui/tree_closed.png"));
	tex_opened = DX12::Texture::get_resource(DX12::texure_header("textures/gui/tree_opened.png"));
	texture = tex_opened;
	owner->on_toogle.register_handler(this, [this](bool v) {
		if (v) texture = tex_opened;
		else
			texture = tex_closed;
	});
}

void GUI::Elements::line::on_drop_leave(GUI::drag_n_drop_package::ptr)
{
    dropping = false;
}

void GUI::Elements::line::on_drop_enter(GUI::drag_n_drop_package::ptr)
{
    dropping = true;
}

void GUI::Elements::line::draw(DX12::context& c)
{
    if (dropping)
		renderer->draw_color(c, float4(153, 114, 202, 255) / 255.0f, get_render_bounds());
    else if (owner->selected)
		renderer->draw_color(c, float4(53, 114, 202, 255) / 255.0f, get_render_bounds());
}

bool GUI::Elements::line::can_accept(GUI::drag_n_drop_package::ptr p)
{
    bool is_tree = p->name == "tree_item";

    if (!is_tree)
        return false;

    auto elem = dynamic_cast<line*>(p->element.lock().get());
    return !owner->node->is_parent(elem->owner->node);
}

bool GUI::Elements::line::on_drop(GUI::drag_n_drop_package::ptr p, vec2)
{
    auto elem = dynamic_cast<line*>(p->element.lock().get());
    owner->node->add_child(elem->owner->node);
    return true;
}

bool GUI::Elements::line::need_drag_drop()
{
    return owner->main_tree.lock()->draggable;
}

bool GUI::Elements::line::on_mouse_action(mouse_action action, mouse_button button, vec2 pos)
{
    if (button == mouse_button::LEFT)
        if (action == mouse_action::DOWN)
            owner->main_tree.lock()->on_select_internal(owner->get_ptr<tree_element>());

    return true;
}

GUI::Elements::line::line(GUI::Elements::tree_element* owner)
{
    this->owner = owner;
    docking = GUI::dock::TOP;
    height_size = GUI::size_type::MATCH_CHILDREN;
    width_size = GUI::size_type::MATCH_PARENT;
    x_type = GUI::pos_x_type::LEFT;
    set_package("tree_item");
}


void GUI::Elements::tree_creator::init_element(tree_element* tree, base_tree* elem)
{
    base::ptr l(new line(tree));

	base::ptr space(new base);

	space->size= { elem->calculate_depth() * 25,0 };
	space->docking = dock::LEFT;
	l->add_child(space);

    toogle_icon::ptr	open_icon(new toogle_icon(tree));
  //  open_icon->texture = DX12::Texture::null;
//    open_icon->size = { 16, 16 }; // size_type::MATCH_PARENT;
    open_icon->docking = dock::LEFT;

    open_icon->visible = elem->get_child_count()>0;
    l->add_child(open_icon);
/*    image::ptr icon(new image());
    //
    icon->size = { 16, 16 }; // size_type::MATCH_PARENT;
    icon->docking = dock::LEFT;
    icon->width_size = size_type::FIXED;
    icon->height_size = size_type::FIXED;
    l->add_child(icon);*/
    label::ptr label_text(new label());
    label_text->docking = dock::LEFT;
    label_text->text = convert(elem->get_name());
    label_text->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
    label_text->margin = { 5, 0, 0, 0 };
    l->add_child(label_text);
    tree->add_child(l);
}


void  GUI::Elements::tree::draw(DX12::context& c)
{
	renderer->draw_container(get_ptr(), c);
	//    if (is_hovered())
//	renderer->draw(c, Skin::get().TabButton.Normal, get_render_bounds());
}
