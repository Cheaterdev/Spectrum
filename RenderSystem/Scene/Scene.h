#pragma once

class Scene : public scene_object//, public tree_contoller<scene_object>
{


    //	std::set<scene_object*> static_objects;
    //	std::set<scene_object*> dynamic_objects;
public:
    using ptr = s_ptr<Scene>;
    using wptr = w_ptr<Scene>;

    //		std::set<MaterialAsset*> materials;
    Scene() 
    {
        scene = this;
        // controller = this;
     
    }

    Events::Event<scene_object*> on_element_add;
    Events::Event<scene_object*> on_element_remove;
    Events::Event<scene_object*> on_moved;
    Events::Event<scene_object*> on_changed;


    CommonAllocator nodes_allocator;
    std::vector<MeshAssetInstance::node_data> nodes_data;

   
    /*	virtual void on_element_add(scene_object* node)
        {
            if(node->)
        }

        virtual void on_element_remove(scene_object* node) {

        }*/
        /*
          virtual base_tree* get_root() override
          {
              return this;
          }

          virtual scene_object* get_child(scene_object* node, int i) override;

          virtual int get_child_count(scene_object* node) override;
          virtual void add_element(scene_object* parent, scene_object* node)
          {
              auto p = node->get_ptr();
              parent->add_child(p);
          }

          virtual std::string get_name(scene_object*node){ return node->name; }*/

};