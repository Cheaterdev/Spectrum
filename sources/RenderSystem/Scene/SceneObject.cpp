#include "pch.h"




scene_object::~scene_object()
{
}

object_type scene_object::get_type()
{
    return object_type::OBJECT;
}

std::shared_ptr<scene_object> scene_object::find_by_name(std::wstring name)
{
    if (name == this->name) return get_ptr();

    for (auto& c : childs)
    {
        auto res = c->find_by_name(name);

        if (res) return res;
    }

    return nullptr;
}

void scene_object::debug_draw(debug_drawer& drawer)
{
    occluder::debug_draw(drawer);

  //  childs_occluder.debug_draw(drawer);
    //childs_occluder.primitive
    for (auto& c : childs)
        c->debug_draw(drawer);
}

bool scene_object::update_transforms()
{

//	if (!need_update_layout) return false;

	mat4x4 new_transform;

    if (parent)
		new_transform = local_transform * parent->global_transform;
    else
		new_transform = local_transform;

	bool changed = global_transform != new_transform;

	global_transform = new_transform;

	for (auto& c : childs)
	{
		if (changed)
			c->need_update_layout = true;

		c->update_transforms();
	}
       



	if (changed) {
		apply_transform(global_transform);
//	calculate_size();

	
	}

	calculate_childs(childs);

	return changed;

}

scene_object::scene_object()
{
    local_transform.identity();
    parent = nullptr;
//    childs_occluder.primitive.reset(new AABB());
 //   occluder::mat = &global_transform;
//    childs_occluder.mat = &global_transform;
  //  recalculate_aabb = true;
}

 void scene_object::on_add(scene_object* parent) 
{
	parent_type::on_add(parent);
	auto scene = parent->scene;
	if (scene)
	{
		iterate([scene](scene_object * object) {
			object->scene = scene;
			scene->on_element_add(object);
		return true;		
	});
	}
	update_layout();

}


 void scene_object::on_remove() 
{
//	 auto scene = parent->scene;
	 if (scene)
	{
		iterate([this](scene_object * object) {
				scene->on_element_remove(object);
				object->scene = nullptr;

			return true;
		});
	}

	 scene = nullptr;
	parent_type::on_remove();
}

/*
void Scene::on_element_add(scene_object* node, scene_object* add_node)
{
	//throw std::exception("The method or operation is not implemented.");
}

void Scene::on_element_remove(scene_object * node, scene_object* remove_node)
{
	//throw std::exception("The method or operation is not implemented.");
}

scene_object* Scene::get_child(scene_object*node, int i)
{
	auto it = node->get_childs().begin();
	std::advance(it, i);
		return it->get();
}

int Scene::get_child_count(scene_object*node)
{
	return node->get_childs().size();
}
*/

tree_item_listener::~tree_item_listener()
{
    if (tree)
        tree->unregister_listener(this);
}
