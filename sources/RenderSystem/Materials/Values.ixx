export module Graphics:Materials.Values;


import Core;
import :Asset;
import :TextureAsset;

import GUI;
export
{

class VectorType;

constexpr struct ShaderParamType:public ::FlowGraph::parameter_type
{
	 int M;
     int N;
     std::string    name;

     bool can_cast(parameter_type* other) override;
     ShaderParamType() = default;
     constexpr  ShaderParamType(int M, int N, std::string name):M(M),N(N), name(name)
    {
        
    }
    virtual ~ShaderParamType() = default;
    bool operator==(const ShaderParamType& t) const;

    int get_size();

private:
    SERIALIZE()
	{
        SAVE_PARENT(::FlowGraph::parameter_type);
		ar& NVP(M);
		ar& NVP(N);
        ar& NVP(name);
	}
};

class VectorType:public ::FlowGraph::parameter_type
{
public:
   constexpr VectorType()
	{

	}

	bool can_cast(parameter_type* other) override;


private:
    SERIALIZE()
	{
        SAVE_PARENT(::FlowGraph::parameter_type);

	}
};

struct shader_parameter
{
	std::string name;
    ShaderParamType type;
   
    std::string to_string();

        shader_parameter() = default;
        shader_parameter(std::string str, ShaderParamType type);
	//	shader_parameter(ShaderParamType type);


    private:
        SERIALIZE()
        {
            ar& NVP(name);
            ar&  NVP(type);
        }
};


struct ShaderParams:public Singleton<ShaderParams>
{
    // TODO: make const
    const  VectorType VECTOR;
    const ShaderParamType FLOAT1= ShaderParamType(1,1,"float");
    const ShaderParamType FLOAT2 = ShaderParamType(1,2,"float2");
    const ShaderParamType FLOAT3 = ShaderParamType(1,3,"float3");
    const ShaderParamType FLOAT4 = ShaderParamType(1,4,"float4");
    // A texture reference (TextureAssetNode's output, SamplingNode's input)
    // -- not a numeric value, so M=2 deliberately doesn't match any FLOATn
    // (all M=1) or VectorType::can_cast's M==1 check, so the GUI won't let
    // it link to a plain float/vector port.
    const ShaderParamType TEXTURE = ShaderParamType(2,1,"uint");
};
template <class T, class N>
class TemplatedParameter: public T
{

};

class MaterialGraph;
class MaterialContext;
template<class T = FlowGraph::Node>
class MaterialTNode : public T, public  GUI::Elements::FlowGraph::VisualGraph
{
      /*  bool is_vector(FlowGraph::data_types v)
        {
            return true;
        }

        int get_N(FlowGraph::data_types v)
        {
            if (v == ShaderParams::get().FLOAT1)
                return 1;

            if (v == ShaderParams::get().FLOAT2)
                return 2;

            if (v == ShaderParams::get().FLOAT3)
                return 3;

            if (v == ShaderParams::get().FLOAT4)
                return 4;

            if (v == ShaderParams::get().VECTOR)
                return 4;

            return 0;
        }*/
    protected:
        void operator()(::FlowGraph::GraphContext* c);;

        virtual  void operator()(MaterialContext*) = 0;

    public:

    /*    virtual bool can_link(FlowGraph::data_types a, FlowGraph::data_types b)
        {
            if (is_vector(a) && is_vector(b))
            {
                if (a == ShaderParams::get().VECTOR || b == ShaderParams::get().VECTOR)
                    return true;

                return get_N(a) <= get_N(b);
            }

            return a == b;
        }*/
/*
        template<class ParamType>
        TemplatedParameter<input, ParamType>::ptr get_input(int i)
        {
            T::get_input();
            
        }
      */
        virtual ~MaterialTNode()
        {}

        MaterialTNode()
        {
            T::start_if_output = true;
        }

        // Default per-node preview thumbnail (see create_node_preview_hook).
        // Node types with their own custom editor window (VectorNode,
        // ScalarNode, MaterialGraph) override this themselves and don't get
        // this default -- but can still call build_live_preview_widget()
        // directly (see TextureAssetNode::create_editor_window) to embed the same
        // live thumbnail alongside their own content.
        virtual GUI::base::ptr create_editor_window() override;

        // Null if there's no hook registered yet, this node has no
        // preview_material reachable via its owning graph, or this is a
        // graph/function container node (those never get a preview slice).
        GUI::base::ptr build_live_preview_widget();

    private:
        SERIALIZE()
        {
            SAVE_PARENT(T);
        }

};


typedef MaterialTNode<::FlowGraph::Node> MaterialNode;
typedef MaterialTNode<::FlowGraph::graph> MaterialGraphNode;

    

	template<class T>
class MaterialNodeRegister
{
public:
    MaterialNodeRegister(std::string name)
    {
        FlowGraph::FlowSystem::get().register_node<T>(name);
	}

};


class MaterialFunction;



struct Uniform
{
	std::string name;
    ShaderParamType type;
	//  std::string shader_name;

	//FlowGraph::data_types type;

	struct _value
	{
		union
		{
			int i_value;
			float f_value;
			Vector<vector_data_t<float,2>> f2_value;
			Vector<vector_data_t<float,3>> f3_value;
			Vector<vector_data_t<float,4>> f4_value;

			int raw_data[4];
		};
		_value();
	
	} value;

	Uniform();


    Events::Event<Uniform*> on_change;
    using ptr = std::shared_ptr<Uniform>;

    SERIALIZE()
    {
        ar& NVP(name);
        //   ar& NVP(shader_name);
        ar& NVP(type);
        ar& NVP(value.raw_data);
    }
};


struct TextureSRVParams
{
	using ptr = std::shared_ptr<TextureSRVParams>;
	
	Asset::ref asset;
	Events::prop<bool> to_linear;

	
	TextureSRVParams();

	TextureSRVParams(Asset::ref&& asset, bool to_linear);
private:


    SERIALIZE()
	{
		ar& NVP(asset);
		ar& NVP(to_linear);
	}
};

struct ShaderSource
{
    std::string uniforms;
	std::string text;
	std::string function_name;
	std::vector<HAL::shader_macro> macros;
};
class MaterialContext : public FlowGraph::GraphContext
{
        int params = 0;

		ShaderSource voxel_shader;
		ShaderSource pixel_shader;
		ShaderSource tess_shader;
		ShaderSource preview_shader;


        std::string text;

        // Per-node preview capture (see material_preview.sig / Materials.cpp).
        // Only populated while generating preview_shader.
        //
        // Node outputs are transient here -- put() triggers send_next()
        // immediately, which pushes the value downstream then clears it
        // (parameter::send_next() in Core/FlowGraph/FlowGraph.cpp), all
        // synchronously inside put(). So a node's output.value is gone by
        // the time its operator() returns; capture_value() is called
        // directly from add_value()/create_value() instead, where the
        // value is generated, gated by capturing_node (set/restored around
        // each node's call in MaterialTNode<T>::operator()).
        bool capture_preview = false;
        int preview_slice_counter = 0;
        std::map<FlowGraph::Node*, int> node_preview_slot;
    public:
        FlowGraph::Node* capturing_node = nullptr;
        void capture_value(MaterialFunction* owner_func, const shader_parameter& val);
    private:

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
		ShaderSource hit_shader;

        // Set during start(): true when the graph's opacity output is driven,
        // i.e. the material is (partially) transparent. Consumed CPU-side.
        bool transparent = false;

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

		ShaderSource get_pixel_result();
		ShaderSource get_voxel_result();
		ShaderSource get_tess_result();
		ShaderSource get_preview_result();

        void start(std::string orig_file,MaterialGraph* graph);

		void clear_parameters();

		// Slice a node was assigned in the preview results array, or -1 if the
		// graph hasn't been (re)generated with previews yet / node has no output.
		int get_preview_slot(FlowGraph::Node* node);
		int get_preview_slot_count();

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

     
		static ptr create_default() {
			return std::shared_ptr<MaterialFunction>(new MaterialFunction());
		}
        virtual ~MaterialFunction();
        shader_parameter add_value(const ShaderParamType& type, std::string s);

        shader_parameter add_value(const ShaderParamType& type);

        void add_function(std::string s);

    private:
        SERIALIZE()
        {
            SAVE_PARENT(::FlowGraph::graph);

        }

};

class MaterialGraph : public MaterialFunction
{
        FlowGraph::input::ptr position;
        FlowGraph::input::ptr texcoord;



        FlowGraph::output::ptr i_base_color;
        FlowGraph::output::ptr i_metallic;
     //   FlowGraph::output::ptr i_specular;
        FlowGraph::output::ptr i_roughness;
        FlowGraph::output::ptr i_emissive;
        FlowGraph::output::ptr i_normal;
        FlowGraph::output::ptr i_tess_displacement;
        FlowGraph::output::ptr i_glow;
        // Registered after i_glow so they append to the end of the generated
        // COMPILED_FUNC output list (keeps existing output order stable).
        FlowGraph::output::ptr i_opacity;
        FlowGraph::output::ptr i_refraction;

    public:
        using ptr = s_ptr<MaterialGraph>;

        // The owning material (set by universal_material); used to build a live
        // preview on the graph-output node.
        Asset* preview_material = nullptr;
        // Filled by the app layer (which can build a preview widget above the
        // Materials module — Materials can't import the renderer without a cycle).
        static inline std::function<GUI::base::ptr(std::shared_ptr<Asset>)> create_preview_hook;
        // Same idea, per node: builds the small live-value thumbnail shown on
        // graph nodes that don't already have a custom editor window (see
        // MaterialTNode<T>::create_editor_window below).
        static inline std::function<GUI::base::ptr(std::shared_ptr<Asset>, ::FlowGraph::Node*)> create_node_preview_hook;

        MaterialGraph();
        virtual ~MaterialGraph();
        virtual GUI::base::ptr create_editor_window() override;
        FlowGraph::input::ptr get_texcoord();

        FlowGraph::output::ptr get_base_color();

        FlowGraph::output::ptr get_mettalic();
    //    FlowGraph::output::ptr get_specular();
        FlowGraph::output::ptr get_normals();

        FlowGraph::output::ptr get_glow();
           FlowGraph::output::ptr get_roughness();
    FlowGraph::output::ptr get_tess_displacement();
        FlowGraph::output::ptr get_opacity();
        FlowGraph::output::ptr get_refraction();
        virtual void start(MaterialContext* context);
    private:
        SERIALIZE()
        {
            SAVE_PARENT(::FlowGraph::graph);
           ar& NVP(position);
            ar& NVP(texcoord);
            ar& NVP(i_base_color);
            ar& NVP(i_metallic);
            ar& NVP(i_glow);
         //   ar& NVP(i_specular);
            ar& NVP(i_roughness);
            ar& NVP(i_emissive);
            ar& NVP(i_normal);
            ar& NVP(i_tess_displacement);
            ar& NVP(i_opacity);
            ar& NVP(i_refraction);
            i_base_color->default_value = shader_parameter("float4(0,0,0,1)", ShaderParams::get().FLOAT4);
            i_metallic->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
			i_roughness->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
       //     i_specular->default_value = shader_parameter("float4(0,0,0,0)", ShaderParams::get().FLOAT4);
            i_normal->default_value = shader_parameter("float4(0.5,0.5,1,0)", ShaderParams::get().FLOAT4);
            i_glow->default_value = shader_parameter("float4(0.0,0.0,0,0)", ShaderParams::get().FLOAT4);
            i_tess_displacement->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
            // 1.0 opacity = fully opaque; 1.0 IOR = no refraction (air).
            i_opacity->default_value = shader_parameter("1.0", ShaderParams::get().FLOAT1);
            i_refraction->default_value = shader_parameter("1.0", ShaderParams::get().FLOAT1);
        }

};

// Holds a texture asset reference and exposes it (as a ShaderParams::TEXTURE
// value) for SamplingNode to sample. Split out from what used to be a single
// TextureNode so a texture can be referenced once and sampled multiple ways
// (different tc, different sampling nodes) without re-picking the asset each
// time, and so the asset preview (the raw texture) is separate from a
// sampled node's live computed-value preview.
class TextureAssetNode : public MaterialNode, public AssetHolder
{
        FlowGraph::output::ptr o_texture;

        TextureAssetNode();
        TextureSRVParams::ptr texture_info;
    public:
        using ptr = s_ptr<TextureAssetNode>;

        TextureAssetNode(TextureAsset::ptr _Asset, bool to_linear = false);
        virtual ~TextureAssetNode();
        void operator()(MaterialContext* context) override;

        // Raw 2D texture preview -- unlike a sampled node's live
        // computed-value preview, this asset has no "value" of its own to
        // run through the shader/preview PSO.
        virtual GUI::base::ptr create_editor_window();
    private:
        SERIALIZE()
        {
            SAVE_PARENT(::FlowGraph::Node);
            SAVE_PARENT(AssetHolder);

            ar& NVP(texture_info);
            ar& NVP(o_texture);
        }

};

// Samples a texture (from an upstream TextureAssetNode) at a given tc.
// Used to be TextureNode, with the texture asset reference baked in
// directly instead of taken as an input -- see TextureAssetNode's comment.
class SamplingNode : public MaterialNode
{
        FlowGraph::input::ptr i_tc, i_texture;
        FlowGraph::output::ptr o_vec4, o_r, o_g, o_b, o_a;

    public:
        using ptr = s_ptr<SamplingNode>;

        SamplingNode();
        virtual ~SamplingNode();
        void operator()(MaterialContext* context) override;

             	static ptr create_default() {
			return std::make_shared<SamplingNode>();
		}
    private:
        SERIALIZE()
        {
            SAVE_PARENT(::FlowGraph::Node);

            ar& NVP(i_tc);
            ar& NVP(i_texture);
            ar& NVP(o_vec4);
            ar& NVP(o_r);
            ar& NVP(o_g);
            ar& NVP(o_b);
			ar& NVP(o_a);
        }

};

// Convenience for callers that just want to sample a texture asset at the
// graph's own texcoord input -- the common case for procedurally-built
// graphs (mesh import, asset-explorer "create material from textures").
// Builds and registers both nodes and wires TextureAssetNode -> SamplingNode
// -> graph texcoord; returns the SamplingNode so callers can still link
// individual output channels (o_r/o_g/... for packed maps).
SamplingNode::ptr make_sampling_node(MaterialGraph* graph, TextureAsset::ptr asset, bool to_linear = false);

class PowerNode : public MaterialNode
{
        FlowGraph::input::ptr i_vec, i_power;
        FlowGraph::output::ptr o_value;
     PowerNode();
    public:
        using ptr = s_ptr<PowerNode>;

       
		static ptr create_default() {
			return std::shared_ptr<PowerNode>(new PowerNode());
		}
        void operator()(MaterialContext*) override;
    private:
        SERIALIZE()
        {
            SAVE_PARENT(::FlowGraph::Node);
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
             	static ptr create_default() {
			return std::make_shared<VectorNode>(vec4(1,1,1,1));
		}
        void operator()(MaterialContext* c) override;

        virtual GUI::base::ptr create_editor_window()override;
    private:
        SERIALIZE()
        {
            SAVE_PARENT(MaterialNode);
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
               	static ptr create_default() {
			return std::make_shared<ScalarNode>(1.0f);
		}
        void operator()(MaterialContext* c) override;

        virtual GUI::base::ptr create_editor_window()override;
    private:
        SERIALIZE()
        {
            SAVE_PARENT(MaterialNode);
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
             	static ptr create_default() {
			return std::make_shared<SumNode>();
		}
        void operator()(MaterialContext*);
    private:
        SERIALIZE()
        {
            SAVE_PARENT(::FlowGraph::Node);
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
                              	static ptr create_default() {
			return std::make_shared<MulNode>();
		}
        void operator()(MaterialContext* c) override;
    private:
        SERIALIZE()
        {
            SAVE_PARENT(::FlowGraph::Node);
            ar& NVP(i_vec);
            ar& NVP(i_power);
            ar& NVP(o_value);
        }

};
                    
template <class T>
void MaterialTNode<T>::operator()(::FlowGraph::GraphContext* c)
{
    auto* mat_context = static_cast<MaterialContext*>(c);

    // Marks this as the "currently executing" leaf node so add_value()/
    // create_value() can attribute generated values to it (see
    // MaterialContext::capturing_node). Save/restore so nested
    // MaterialFunction traversal nests correctly; skip container nodes
    // themselves -- only leaf nodes represent a single computed value.
    FlowGraph::Node* prev_capturing = mat_context->capturing_node;
    if (!dynamic_cast<::FlowGraph::graph*>(this))
        mat_context->capturing_node = this;

    (*this)(mat_context);

    mat_context->capturing_node = prev_capturing;
}

template <class T>
GUI::base::ptr MaterialTNode<T>::create_editor_window()
{
    return build_live_preview_widget();
}

template <class T>
GUI::base::ptr MaterialTNode<T>::build_live_preview_widget()
{
    // Graph/function container nodes (MaterialGraph, MaterialFunction) never
    // get a preview slice -- capture_value() skips them (see
    // MaterialTNode<T>::operator()) since their "outputs" are boundary pins,
    // not a single computed value. Bail out before building an empty widget.
    if (dynamic_cast<::FlowGraph::graph*>(this))
        return nullptr;

    if (!MaterialGraph::create_node_preview_hook)
        return nullptr;

    // Walk up to the owning MaterialGraph (nodes inside a nested
    // MaterialFunction have that function as their immediate owner, not the
    // root graph) to reach preview_material.
    ::FlowGraph::graph* cur = this->get_graph();
    while (cur)
    {
        if (auto* g = dynamic_cast<MaterialGraph*>(cur))
        {
            if (!g->preview_material)
                return nullptr;
            return MaterialGraph::create_node_preview_hook(g->preview_material->get_ptr<Asset>(), this);
        }
        cur = cur->get_graph();
    }
    return nullptr;
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

	SpecToMetNode();

     	static ptr create_default() {
			return std::make_shared<SpecToMetNode>();
		}

	void operator()(MaterialContext* c) override;

	//virtual GUI::base::ptr create_editor_window()override;
private:
    SERIALIZE()
	{
        SAVE_PARENT(MaterialNode);
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
     //   TiledTexture::ref asset;
        TiledTextureNode();
    public:
        using ptr = s_ptr<TiledTextureNode>;

    //    TiledTextureNode(TiledTexture::ptr _Asset);
        virtual ~TiledTextureNode();
        void operator()(MaterialContext* context) override;
        /* virtual bool test_start()
        {
        if (!i_tc->has_input() && !i_tc->has_value())
        {
        shader_parameter input;
        input.name = "input.tc";
        input.type = ShaderParams::get().FLOAT2;
        i_tc->put(input);
        return false;
        }

        else
        return Node::test_start();
        }*/
    private:
        SERIALIZE()
        {
            SAVE_PARENT(MaterialNode);
            SAVE_PARENT(AssetHolder);


            ar& NVP(i_tc);
            ar& NVP(o_vec4);
            ar& NVP(o_r);
            ar& NVP(o_g);
            ar& NVP(o_b);
            ar& NVP(o_a);
        }

};
       
}
