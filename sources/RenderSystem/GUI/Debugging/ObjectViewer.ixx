export module GUI:Debug.ObjectViewer;
import :ScrollContainer;
import :Image;
import Core;
import :Tree;

import <RenderSystem.h>;

using namespace GUI;

using namespace GUI::Elements;

export  class object_tree_creator
{
public:
	using ptr = s_ptr<object_tree_creator>;
	virtual void init_element(tree_element<member_item, object_tree_creator>* tree, member_item* elem);
};

