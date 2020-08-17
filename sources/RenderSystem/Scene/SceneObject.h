enum class object_type : int {SCENE, OBJECT, MESH, MESH_NODE, CAMERA};


/*
class tree_contoller_base
{
	std::vector<tree_listener*> listeners;

public:
    virtual void on_element_add(base_tree* node, base_tree* add_node) = 0;
    virtual void on_element_remove(base_tree* node, base_tree* remove_node) = 0;
    virtual base_tree* get_root() = 0;
    virtual base_tree* get_child(base_tree*, int i) = 0;
    virtual int get_child_count(base_tree*) = 0;

    virtual void add_element(base_tree* parent, base_tree* node) = 0;
	virtual std::string get_name(base_tree*){ return "some node"; }

};

template<class T>
class tree_contoller : public tree_contoller_base
{
    virtual void on_element_add(base_tree* node, base_tree* add_node)
    {
        on_element_add(static_cast<T*>(node), static_cast<T*>(add_node));
    }

    virtual void on_element_remove(base_tree* node, base_tree* remove_node)
    {
        on_element_remove(static_cast<T*>(node), static_cast<T*>(remove_node));
    }

    virtual base_tree* get_child(base_tree* node, int i) { return get_child(static_cast<T*>(node), i); };
    virtual int get_child_count(base_tree* node) { return get_child_count(static_cast<T*>(node)); }
    virtual void add_element(base_tree* parent, base_tree* node)
    {
        add_element(static_cast<T*>(parent), static_cast<T*>(node));
    }
	virtual std::string get_name(base_tree* node){ return     get_name(static_cast<T*>(node)); }

public:
    virtual void on_element_add(T* node, T* add_node) = 0;
    virtual void on_element_remove(T* node, T* remove_node) = 0;

    virtual T* get_child(T*, int) = 0;
    virtual int get_child_count(T*) = 0;

	virtual std::string get_name(T*){ return "some node"; }

    virtual void add_element(T* parent, T* node) = 0;
};*/
/*
class SceneTree : public tree<scene_object, std::set<std::shared_ptr<scene_object>>>, public occluder
{


};
*/
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