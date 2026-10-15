export module Graphics:Materials.MaterialX;

import Core;
import GUI;
import :Materials.Values;

export
{

struct MxPortDef
{
    std::string name;
    std::string mx_type;      // MaterialX type name: "float", "color3", ...
    ShaderParamType type;
    std::string default_hlsl; // HLSL literal of `type`, used while the input is unconnected

    // Float components of the default (empty for int/bool), and the soft
    // range MaterialX suggests for a UI slider.
    std::vector<float> default_values;
    float soft_min = 0;
    float soft_max = 1;

    // MaterialX "defaultgeomprop": what an unconnected input reads from the
    // mesh instead of a constant (UV0, Pobject, ...). Empty for plain values.
    std::string geomprop;

    // int inputs with a MaterialX `enum` (names and the values they stand for);
    // shown as a drop-down instead of a slider.
    std::vector<std::string> enum_names;
    std::vector<float> enum_values;
};

// One implemented MaterialX nodedef, in graph-ready form. MaterialX types
// that have no HLSL numeric equivalent (matrix, string, filename, closures)
// make the whole nodedef unavailable rather than partially representable.
struct MxNodeDef
{
    std::string name;      // ND_add_float
    std::string category;  // add
    std::string group;     // math
    std::string function;  // HLSL function in mx_stdlib.hlsl
    std::vector<MxPortDef> inputs;
    std::vector<MxPortDef> outputs;
};

// MaterialX standard library, loaded once from <root>/libraries. The
// MaterialX library is only used to read nodedefs; HLSL for each node comes
// from workdir/shaders/materialx/mx_stdlib.hlsl (see its header for how a
// nodedef is matched to a function).
class MaterialXLibrary : public Singleton<MaterialXLibrary>
{
    friend class Singleton<MaterialXLibrary>;

    std::vector<MxNodeDef> defs;
    std::map<std::string, size_t> index;
    std::filesystem::path library_root;

    MaterialXLibrary() = default;

public:
    // Reads a .mtlx file and registers one material asset per surface material
    // in it (or per bare surface shader node when the file has no material
    // nodes). Textures are registered as assets, resolved against the file's
    // folder. Returns the number of materials created; problems are logged.
    size_t import_file(const std::filesystem::path& file);

    // The graph half of import_file: one named material graph per surface in
    // the file, without creating assets for them.
    std::vector<std::pair<std::string, MaterialGraph::ptr>> build_graphs(const std::filesystem::path& file);

    // Loads <root>/libraries/stdlib and scans <hlsl_stdlib> for implemented
    // functions, then adds a node-picker entry for every implemented nodedef.
    // Returns the number of registered nodes (0 on failure, which is logged).
    size_t register_nodes(const std::filesystem::path& root, const std::filesystem::path& hlsl_stdlib);

    const MxNodeDef* find(const std::string& nodedef_name) const;
    const std::vector<MxNodeDef>& get_all() const { return defs; }
};

// One generic node for every MaterialX nodedef: ports and HLSL call come from
// the registry entry named by `nodedef`. Unconnected inputs emit the
// nodedef's default literal.
class MaterialXNode : public MaterialNode
{
    std::string nodedef;

    // One per input, null where the input has no editable constant (inputs that
    // read geometry, and textures). int and bool inputs are float uniforms too,
    // cast where they are used. A uniform lives in MaterialCB, so editing it
    // never recompiles the shader and materials with the same graph keep sharing
    // a PSO.
    std::vector<Uniform::ptr> uniforms;

    MaterialXNode() = default;

    // Creates the uniform of every editable input that lacks one: all of them for
    // a fresh node, and the int/bool ones for a graph saved before those were
    // uniforms (their slots load as null).
    void ensure_uniforms();

protected:
    void reset_for() override;

public:
    using ptr = s_ptr<MaterialXNode>;

    explicit MaterialXNode(const MxNodeDef& def);

    void operator()(MaterialContext* c) override;

    // Wires UV0 / position inputs to the graph's own texcoord / position
    // inputs when the node is dropped into a MaterialGraph.
    void on_add(::FlowGraph::graph* g) override;

    GUI::base::ptr create_editor_window() override;

    // Sets the editable constant of a float-typed input (used by the .mtlx
    // importer). False when the input doesn't exist or has no constant.
    bool set_constant(const std::string& input, const std::vector<float>& values);

    // Current constant of a float-typed input (empty if it has none).
    std::vector<float> get_constant(const std::string& input) const;

    const std::string& get_nodedef() const { return nodedef; }

    // Null when the nodedef has no input of that name.
    ::FlowGraph::input::ptr get_input_by_name(const std::string& input);

private:
    SERIALIZE()
    {
        SAVE_PARENT(MaterialNode);
        ar& NVP(nodedef);
        ar& NVP(uniforms);
        IF_LOAD() ensure_uniforms();
    }
};

}
