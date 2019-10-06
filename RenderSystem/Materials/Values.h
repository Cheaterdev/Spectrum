
namespace ShaderParams
{
    static const FlowGraph::data_types VECTOR("vector");
    static const FlowGraph::data_types FLOAT1("float");
    static const FlowGraph::data_types FLOAT2("float2");
    static const FlowGraph::data_types FLOAT3("float3");
    static const FlowGraph::data_types FLOAT4("float4");
};


struct shader_parameter
{
        std::string name;
        int M;
        int N;
        FlowGraph::data_types type;

        std::string to_string();

        shader_parameter() = default;
        shader_parameter(std::string str, FlowGraph::data_types type);


    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(name);
            ar&  NVP(type);
        }
};


class MaterialGraph;
class MaterialContext;
template<class T = FlowGraph::Node>
class MaterialTNode : public T, public  GUI::Elements::FlowGraph::VisualGraph
{
        bool is_vector(FlowGraph::data_types v)
        {
            return true;
        }

        int get_N(FlowGraph::data_types v)
        {
            if (v == ShaderParams::FLOAT1)
                return 1;

            if (v == ShaderParams::FLOAT2)
                return 2;

            if (v == ShaderParams::FLOAT3)
                return 3;

            if (v == ShaderParams::FLOAT4)
                return 4;

            if (v == ShaderParams::VECTOR)
                return 4;

            return 0;
        }
    protected:
        void operator()(::FlowGraph::GraphContext* c);;

        virtual  void operator()(MaterialContext*) = 0;

    public:

        virtual bool can_link(FlowGraph::data_types a, FlowGraph::data_types b)
        {
            if (is_vector(a) && is_vector(b))
            {
                if (a == ShaderParams::VECTOR || b == ShaderParams::VECTOR)
                    return true;

                return get_N(a) <= get_N(b);
            }

            return a == b;
        }

        virtual ~MaterialTNode()
        {}

        MaterialTNode()
        {
            start_if_output = true;
        }


   
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<T>(*this));
        }

};


typedef MaterialTNode<::FlowGraph::Node> MaterialNode;
typedef MaterialTNode<::FlowGraph::graph> MaterialGraphNode;

class MaterialFunction;



struct Uniform
{
	std::string name;
	//  std::string shader_name;

	FlowGraph::data_types type;

	struct _value
	{
		union
		{
			int i_value;
			float f_value;
			Vector<vec2_t<float, false>> f2_value;
			Vector<vec3_t<float,false>> f3_value;
			Vector<vec4_t<float, false>> f4_value;

			int raw_data[4];
		};
		_value()
		{
			raw_data[0] = 0;
			raw_data[1] = 0;
			raw_data[2] = 0;
			raw_data[3] = 0;

		}
	
	} value;

	Uniform()
	{
		name = "unknown";
	}


    Events::Event<Uniform*> on_change;
    using ptr = std::shared_ptr<Uniform>;

    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        ar& NVP(name);
        //   ar& NVP(shader_name);
        ar& NVP(type);
        ar& NVP(value.raw_data);
    }
};


BOOST_IS_BITWISE_SERIALIZABLE(Uniform::_value);

struct TextureSRVParams
{
	using ptr = std::shared_ptr<TextureSRVParams>;
	
	Asset::ref asset;
	Events::prop<bool> to_linear;

	
	TextureSRVParams():asset(nullptr),to_linear(false)
	{
	}

	TextureSRVParams(Asset::ref asset, bool to_linear) :asset(asset), to_linear(to_linear)
	{}
private:


	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int file_version)
	{
		ar& NVP(asset);
		ar& NVP(to_linear);
	}
};
class MaterialContext : public FlowGraph::GraphContext
{
        int params = 0;
        std::string voxel_shader;
        std::string pixel_shader;
        std::string tess_shader;
        std::string text;

        std::map < MaterialFunction*, std::string > functions;
        std::vector<TextureSRVParams::ptr> textures;

        std::vector<Uniform::ptr> uniforms;
		
		std::map<Uniform::ptr, std::string> shader_parameter_uniform;
		std::map<TextureSRVParams::ptr, std::string> shader_parameter_srv;

		// TODO: tiled
		// std::map < { TextureSRVParams::ptr, shader_parameter},  shader_parameter  > shader_parameter_srv_tiled;


		
	
        std::string uniform_struct;
		int uniform_offset = 0;
        int texture_counter;
        int unordered_counter;
        MaterialGraph* graph = nullptr;

        std::string generate_uniform_struct();
    public:

        std::vector<Uniform::ptr> uniforms_ps;
        std::vector<Uniform::ptr> uniforms_tess;

        virtual ~MaterialContext();

        using ptr = s_ptr<MaterialContext>;
        std::string get_new_param();


        virtual void add_task(FlowGraph::Node* node) override;

        void register_function(MaterialFunction* func, std::string text);

        std::vector<Uniform::ptr> get_uniforms();
        std::vector<TextureSRVParams::ptr> get_textures();

        std::string get_texture(TextureSRVParams::ptr &p);
        shader_parameter get_texture(TextureSRVParams::ptr& p, shader_parameter tc);
        shader_parameter create_value(Uniform::ptr f);

        std::string get_result();
        std::string get_voxel_result();

        std::string get_tess_result();
        void start(std::string orig_file,MaterialGraph* graph);

		void clear_parameters();

        /*	virtual FlowGraph::graph* create_graph(){
        return new MaterialGraph();
        }*/
};

class MaterialFunction : public MaterialGraphNode
{
        std::string func_call;
        std::vector<shader_parameter> outputs;
        std::vector<std::string > orig_names;
        std::vector<std::string > f_output_names;

    protected:

        virtual std::list<Node::ptr>  on_drop(MyVariant value) override;


        virtual void on_finish() override;

        std::string generate_struct(std::vector<FlowGraph::output::ptr>& parameters);


        std::string generate_input(std::vector<FlowGraph::input::ptr>& parameters);

        std::string generate_output(std::vector<FlowGraph::output::ptr>& parameters);
        std::string text;

        void operator()(MaterialContext* mat_context) override;



    public:
        std::string func_name;
        MaterialFunction();
        virtual ~MaterialFunction();
        shader_parameter add_value(const FlowGraph::data_types type, std::string s);

        shader_parameter add_value(const FlowGraph::data_types type);

        void add_function(std::string s);

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<::FlowGraph::graph>(*this));
        }

};

class MaterialGraph : public MaterialFunction
{
        LEAK_TEST(MaterialGraph)

        FlowGraph::input::ptr position;
        FlowGraph::input::ptr texcoord;



        FlowGraph::output::ptr i_base_color;
        FlowGraph::output::ptr i_metallic;
     //   FlowGraph::output::ptr i_specular;
        FlowGraph::output::ptr i_roughness;
        FlowGraph::output::ptr i_emissive;
        FlowGraph::output::ptr i_normal;
        FlowGraph::output::ptr i_tess_displacement;

    public:
        using ptr = s_ptr<MaterialGraph>;

        MaterialGraph();
        virtual ~MaterialGraph();
        FlowGraph::input::ptr get_texcoord();

        FlowGraph::output::ptr get_base_color();

        FlowGraph::output::ptr get_mettalic();
    //    FlowGraph::output::ptr get_specular();
        FlowGraph::output::ptr get_normals();
           FlowGraph::output::ptr get_roughness();
    FlowGraph::output::ptr get_tess_displacement();
        virtual void start(MaterialContext* context);
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<::FlowGraph::graph>(*this));
            ar& NVP(position);
            ar& NVP(texcoord);
            ar& NVP(i_base_color);
            ar& NVP(i_metallic);
         //   ar& NVP(i_specular);
            ar& NVP(i_roughness);
            ar& NVP(i_emissive);
            ar& NVP(i_normal);
            ar& NVP(i_tess_displacement);
            i_base_color->default_value = shader_parameter("float4(0,0,0,1)", ShaderParams::FLOAT4);
            i_metallic->default_value = shader_parameter("0.0", ShaderParams::FLOAT1);
			i_roughness->default_value = shader_parameter("0.0", ShaderParams::FLOAT1);
       //     i_specular->default_value = shader_parameter("float4(0,0,0,0)", ShaderParams::FLOAT4);
            i_normal->default_value = shader_parameter("float4(0.5,0.5,1,0)", ShaderParams::FLOAT4);
            i_tess_displacement->default_value = shader_parameter("0.0", ShaderParams::FLOAT1);
        }

};

class TextureNode : public MaterialNode, public AssetHolder
{
        LEAK_TEST(TextureNode)

        FlowGraph::input::ptr i_tc;
        FlowGraph::output::ptr o_vec4, o_r, o_g, o_b, o_a;
     
        TextureNode();
		TextureSRVParams::ptr texture_info;
    public:
        using ptr = s_ptr<TextureNode>;

        TextureNode(TextureAsset::ptr _Asset, bool to_linear = false);
        virtual ~TextureNode();
        void operator()(MaterialContext* context) override;
        /* virtual bool test_start()
         {
             if (!i_tc->has_input() && !i_tc->has_value())
             {
                 shader_parameter input;
                 input.name = "input.tc";
                 input.type = ShaderParams::FLOAT2;
                 i_tc->put(input);
                 return false;
             }

             else
                 return Node::test_start();
         }*/

        virtual GUI::base::ptr create_editor_window();
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<::FlowGraph::Node>(*this));
            ar& NVP(boost::serialization::base_object<AssetHolder>(*this));
            ar& NVP(texture_info);
            ar& NVP(i_tc);
            ar& NVP(o_vec4);
            ar& NVP(o_r);
            ar& NVP(o_g);
            ar& NVP(o_b);
			ar& NVP(o_a);
        }

};



class PowerNode : public MaterialNode
{

        LEAK_TEST(PowerNode)

        FlowGraph::input::ptr i_vec, i_power;
        FlowGraph::output::ptr o_value;
    public:
        using ptr = s_ptr<PowerNode>;

        PowerNode();

        void operator()(MaterialContext*) override;
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<::FlowGraph::Node>(*this));
            ar& NVP(i_vec);
            ar& NVP(i_power);
            ar& NVP(o_value);
        }

};


class VectorNode : public MaterialNode
{
        FlowGraph::output::ptr o_value;
        Uniform::ptr uniform;

        VectorNode();
    public:
        using ptr = s_ptr<VectorNode>;

        VectorNode(vec4 value);

        void operator()(MaterialContext* c) override;

        virtual GUI::base::ptr create_editor_window()override;
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<MaterialNode>(*this));
            ar& NVP(o_value);
            ar& NVP(uniform);
        }

};


class ScalarNode : public MaterialNode
{
        FlowGraph::output::ptr o_value;
        Uniform::ptr uniform;

        ScalarNode() = default;
    public:
        using ptr = s_ptr<ScalarNode>;

        ScalarNode(float value);

        void operator()(MaterialContext* c) override;

        virtual GUI::base::ptr create_editor_window()override;
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<MaterialNode>(*this));
            ar& NVP(o_value);
            ar& NVP(uniform);
        }

};

class SumNode : public MaterialNode
{
        FlowGraph::input::ptr i_vec, i_power;
        FlowGraph::output::ptr o_value;
    public:
        using ptr = s_ptr<SumNode>;

        SumNode();

        void operator()(MaterialContext*);
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<::FlowGraph::Node>(*this));
            ar& NVP(i_vec);
            ar& NVP(i_power);
            ar& NVP(o_value);
        }

};


class MulNode : public MaterialNode
{
        FlowGraph::input::ptr i_vec, i_power;
        FlowGraph::output::ptr o_value;
    public:
        using ptr = s_ptr<MulNode>;

        MulNode();

        void operator()(MaterialContext* c) override;
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<::FlowGraph::Node>(*this));
            ar& NVP(i_vec);
            ar& NVP(i_power);
            ar& NVP(o_value);
        }

};
template <class T>
void MaterialTNode<T>::operator()(::FlowGraph::GraphContext* c)
{
    (*this)(static_cast<MaterialContext*>(c));
}



class SpecToMetNode : public MaterialNode
{
	struct {
		FlowGraph::input::ptr albedo;
		FlowGraph::input::ptr specular;
	} inputs;

	struct {
		FlowGraph::output::ptr albedo;
		FlowGraph::output::ptr metallic;
	} outputs;



//	Uniform::ptr uniform;

	//SpecToMetNode() = default;
public:
	using ptr = s_ptr<SpecToMetNode>;

	SpecToMetNode()
	{
		inputs.albedo = register_input(ShaderParams::FLOAT4, "albedo");
		inputs.specular = register_input(ShaderParams::FLOAT4, "specular");

		outputs.albedo = register_output(ShaderParams::FLOAT4, "albedo");
		outputs.metallic = register_output(ShaderParams::FLOAT1, "metallic");

	}

	void operator()(MaterialContext* c) override
	{
		auto mat_graph = static_cast<MaterialFunction*>(owner);
	
	auto res1 = 	mat_graph->add_value(ShaderParams::FLOAT4);

	auto res2 = mat_graph->add_value(ShaderParams::FLOAT1);


	auto val1 = inputs.albedo->get<shader_parameter>();
	auto val2 = inputs.specular->get<shader_parameter>();


	 mat_graph->add_function(std::string("spec_to_metallic(") + val1.name + "," + val2.name +","+ res1.name + "," + res2.name + ")");





		outputs.albedo->put(res1);

		outputs.metallic->put(res2);

		


	}

	//virtual GUI::base::ptr create_editor_window()override;
private:
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar& NVP(boost::serialization::base_object<MaterialNode>(*this));
		ar& NVP(inputs.albedo);
		ar& NVP(inputs.specular);

		ar& NVP(outputs.albedo);
		ar& NVP(outputs.metallic);
	}

};


class TiledTextureNode : public MaterialNode, public AssetHolder
{

        FlowGraph::input::ptr i_tc;
        FlowGraph::output::ptr o_vec4, o_r, o_g, o_b, o_a;
        TiledTexture::ref asset;
        TiledTextureNode();
    public:
        using ptr = s_ptr<TiledTextureNode>;

        TiledTextureNode(TiledTexture::ptr _Asset);
        virtual ~TiledTextureNode();
        void operator()(MaterialContext* context) override;
        /* virtual bool test_start()
        {
        if (!i_tc->has_input() && !i_tc->has_value())
        {
        shader_parameter input;
        input.name = "input.tc";
        input.type = ShaderParams::FLOAT2;
        i_tc->put(input);
        return false;
        }

        else
        return Node::test_start();
        }*/
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<MaterialNode>(*this));
            ar& NVP(boost::serialization::base_object<AssetHolder>(*this));
            ar& NVP(asset);
            ar& NVP(i_tc);
            ar& NVP(o_vec4);
            ar& NVP(o_r);
            ar& NVP(o_g);
            ar& NVP(o_b);
            ar& NVP(o_a);
        }

};


BOOST_CLASS_EXPORT_KEY(ScalarNode);
BOOST_CLASS_EXPORT_KEY(MulNode);
BOOST_CLASS_EXPORT_KEY(SumNode);
BOOST_CLASS_EXPORT_KEY(TextureNode);
BOOST_CLASS_EXPORT_KEY(VectorNode);
BOOST_CLASS_EXPORT_KEY(TiledTextureNode);
BOOST_CLASS_EXPORT_KEY(MaterialGraph);
BOOST_CLASS_EXPORT_KEY(SpecToMetNode);