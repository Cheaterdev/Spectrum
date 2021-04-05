enum class object_type : int {SCENE, OBJECT, MESH, MESH_NODE, CAMERA};


class Scene;
class scene_object : public tree<scene_object, std::set<std::shared_ptr<scene_object>>, occluder>
{
	using parent_type = tree<scene_object, std::set<std::shared_ptr<scene_object>>, occluder>;

    protected:
    //    scene_object* parent;
      //   bool recalculate_aabb=true;
		 bool need_update_layout = true;

    public:
		//occluder childs_occluder;
		mat4x4 local_transform;
		bool resizes_parent = true;
        void set_local_transform(mat4x4& transform)
        {
            local_transform = transform;
        
			update_layout();
        }
		void update_layout()
		{
			need_update_layout = true;
			scene_object* p = parent;

			while (p && !p->need_update_layout)
			{
				p->need_update_layout = true;
				p = p->parent;
			}

		}
        mat4x4 global_transform;
        using ptr = s_ptr<scene_object>;
        using wptr = w_ptr<scene_object>;
		Scene *scene = nullptr;
    public:
        virtual std::wstring get_name() const override
        {
            return name;
        }

        scene_object();
        std::wstring name;
        virtual object_type get_type();

        virtual ~scene_object();
        std::shared_ptr<scene_object> find_by_name(std::wstring name);

        virtual bool update_transforms();
       

      /*  virtual void calculate_size()
        {
            if (!recalculate_aabb)
                return;

  
			for (auto& c : childs)
                c->calculate_size();

			calculate_childs(childs);
			recalculate_aabb = false;
//			std::get<occluder>(*this).
			/*
            if (childs.size())
            {
                if (primitive)
                    ((AABB*)childs_occluder.primitive.get())->set(primitive.get());
                else
                    ((AABB*)childs_occluder.primitive.get())->set((*childs.begin())->childs_occluder.primitive.get());

                for (auto& c : childs)
                    childs_occluder.primitive->combine(c->childs_occluder.primitive.get(), c->local_transform);
            }

            else
            {
                if (primitive)
                    ((AABB*)childs_occluder.primitive.get())->set(primitive.get());
                else
                {
                    ((AABB*)childs_occluder.primitive.get())->min = { 0, 0, 0 };
                    ((AABB*)childs_occluder.primitive.get())->max = { 0, 0, 0 };
                }
            }
        }
*/
		virtual void on_add(scene_object* parent) override;
		virtual void on_remove() override;

    private:
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            if (Archive::is_loading::value)
                remove_all();

            ar& NVP(name)&NVP(local_transform)&NVP(real_childs)&NVP(boost::serialization::base_object<occluder>(*this));

            if (Archive::is_loading::value)
            {
                childs.insert(real_childs.begin(), real_childs.end());

                for (auto& c : childs)
                {
                    c->on_add(this);
                    base_tree::added_child(c.get());
                }
            }
        }
};
/*
class SceneTree
{

	vec3 min;
	vec3 max;
	vec3 size;// = max - min;

	std::array<SceneTree*,6> childs;
public:

	/*
	void begin();

	void end();
	*


	void add_object(vec3 min, vec3 max)
	{
		vec3 local_min=min-this->min;
		vec3 local_max = max - this->min;


		if (local_max.x < size.x/2)
			childs[0]->add_object(min,max);
		else if (local_min.x > size.x/2)
			childs[1]->add_object(min, max);
		else if (local_max.y < size.y / 2)
			childs[2]->add_object(min, max);
		else if (local_min.y > size.y / 2)
			childs[3]->add_object(min, max);
		else if (local_max.z < size.z / 2)
			childs[4]->add_object(min, max);
		else if (local_min.z > size.z / 2)
			childs[5]->add_object(min, max);

	}

};*/