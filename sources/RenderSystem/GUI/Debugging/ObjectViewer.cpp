module GUI:Debug.ObjectViewer;


import :Label;
import :Button;
import :Tree;
import HAL;

using namespace HAL;

using namespace GUI::Elements;
void object_tree_creator::init_element(tree_element<member_item, object_tree_creator>* tree, member_item* elem)
{
	base::ptr l(new line<member_item, object_tree_creator>(tree));

	base::ptr space(new base);

	space->size = { elem->calculate_depth() * 25,0 };
	space->docking = dock::LEFT;
	l->add_child(space);

	toogle_icon::ptr	open_icon(new toogle_icon());

	open_icon->on_toogle = [tree]() {tree->toogle(); };
	//  open_icon->texture = HAL::Texture::null;
  //    open_icon->size = { 16, 16 }; // size_type::MATCH_PARENT;
	open_icon->docking = dock::LEFT;

	open_icon->visible = elem->get_child_count() > 0;
	l->add_child(open_icon);
	/*    image::ptr icon(new image());
		//
		icon->size = { 16, 16 }; // size_type::MATCH_PARENT;
		icon->docking = dock::LEFT;
		icon->width_size = size_type::FIXED;
		icon->height_size = size_type::FIXED;
		l->add_child(icon);	   */
	label::ptr label_text(new label());
	label_text->docking = dock::LEFT;
	label_text->text = convert(elem->get_name());
	label_text->magnet_text = FW1_TEXT_FLAG::FW1_LEFT | FW1_TEXT_FLAG::FW1_VCENTER | FW1_TEXT_FLAG::FW1_NOWORDWRAP;
	label_text->margin = { 5, 0, 0, 0 };
	l->add_child(label_text);

	if (!elem->type.empty()) {
		label::ptr type_text(new label());
		type_text->docking = dock::LEFT;
		type_text->text = convert(elem->type);
		type_text->magnet_text = FW1_TEXT_FLAG::FW1_LEFT | FW1_TEXT_FLAG::FW1_VCENTER | FW1_TEXT_FLAG::FW1_NOWORDWRAP;
		type_text->margin = { 15, 0, 0, 0 };
		type_text->color = float4(0.6f, 0.6f, 0.60f, 1.0f);
		l->add_child(type_text);

	}


	tree->add_child(l);

	tree->on_toogle = [open_icon](bool v)
		{
			open_icon->toogle(v);
		};

	tree->toogle();
}