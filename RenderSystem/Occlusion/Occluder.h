class occluder;
class camera;
class debug_drawer
{
        std::map<std::shared_ptr<Primitive>, mat4x4> aabbs;
        Render::PipelineState::ptr state;


#pragma pack(push, 4)
        struct node
        {
            mat4x4 mat;
            vec4 min;
            vec4 max;
            vec4 color;
            //vec4 color;
        };
#pragma pack(pop)

        //  DX11::Buffer<node>::ptr const_buffer;
    public:
        int visible_count;
        debug_drawer();
        void add(std::shared_ptr<Primitive>, mat4x4);
        void draw(Render::CommandList& context, camera* cam);
};

class occluder
{
	vec3 min;
	vec3 max;
    public:
     
		Primitive::ptr primitive_global;
		Primitive::ptr primitive_childs;

        virtual void debug_draw(debug_drawer& drawer);
        virtual bool is_inside(const Frustum& f);

		void set_primitive(Primitive::ptr primitive)
		{
			this->primitive = primitive;
			if(primitive)
			primitive_global = primitive->clone();
		}
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
		void apply_transform(mat4x4 & t)
		{
			if (primitive_global&&primitive)
			{
				primitive_global->apply_transform(primitive, t);


				min = primitive_global->get_min();
				max = primitive_global->get_max();
				if (primitive_childs)
				{
					min = vec3::min(primitive_childs->get_min(), min);
					max = vec3::max(primitive_childs->get_max(), max);
				}

			}
		}

		vec3 get_min()
		{
			return min;
		}

		vec3 get_max()
		{
			return max;
		}

	
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
		bool is_childs_inside(const Frustum& f)
		{
			/*  if (!childs_occluder.primitive) return false;

			return intersect(f, childs_occluder.primitive.get(), global_transform);*/

			return false;
		}


    private:
		Primitive::ptr primitive;
        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(primitive);
			if(Archive::is_loading::value)
			set_primitive(primitive);
        }
};


BOOST_CLASS_EXPORT_KEY(occluder);