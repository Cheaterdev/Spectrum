export module Graphics:Occluder;

import Core;

export class occluder
{
	vec3 min;
	vec3 max;
    public:
     
		Primitive::ptr primitive_global;
		Primitive::ptr primitive_childs;

        virtual bool is_inside(const Frustum& f);

		void set_primitive(Primitive::ptr primitive);
		template<class T>
		void calculate_childs(const T &container)
		{
			if (container.empty()) return;

			if (!primitive_childs)
				primitive_childs = std::make_shared<AABB>();

			auto it = container.begin();
			((AABB*)primitive_childs.get())->set((*it)->primitive_global.get());

			for (; it != container.end(); ++it)
			{
				if((*it)->resizes_parent)
				primitive_childs->combine((*it)->primitive_global.get());

			}

			min = primitive_childs->get_min();
			max = primitive_childs->get_max();
			if (primitive_global)
			{
				min = vec3::min(primitive_global->get_min(), min);
				max = vec3::max(primitive_global->get_max(), max);
			}
		}
		void apply_transform(mat4x4 & t);

		vec3 get_min();

		vec3 get_max();

	
		/*

		vec3 get_childs_min()
		{
			return primitive_childs->get_min();
		}

		vec3 get_childs_max()
		{
			return primitive_childs->get_max();
		}

		*/
		/*


		bool is_inside(const Frustum& f)
		{
			 if (!primitive)
				 return true;

			return intersect(f, primitive.get(), global_transform);

		}*/
		bool is_childs_inside(const Frustum& f);


    private:
		Primitive::ptr primitive;

		SERIALIZE()
        {
            ar& NVP(primitive);
			IF_LOAD()
			set_primitive(primitive);
        }
};


// REGISTER_TYPE(occluder);