module;
// MaterialX's headers are mutually dependent (Element.h / Interface.h /
// Definition.h include each other) and don't survive being imported as
// header units, so they're included textually here instead of through a
// wrapper module.
#include <MaterialXCore/Document.h>
#include <MaterialXCore/Definition.h>
#include <MaterialXFormat/File.h>
#include <MaterialXFormat/Util.h>
#include <MaterialXFormat/XmlIo.h>

module Graphics:Materials.MaterialX;

import Core;
import GUI;
import :TextureAsset;
import :Materials.UniversalMaterial;

REGISTER_TYPE(MaterialXNode);
CEREAL_REGISTER_POLYMORPHIC_RELATION(MaterialNode, MaterialXNode);
CEREAL_FORCE_REGISTER(MaterialXNode);
CEREAL_FORCE_REGISTER_RELATION(MaterialNode, MaterialXNode);

namespace
{
    bool map_type(const std::string& mx_type, ShaderParamType& out)
    {
        auto& p = ShaderParams::get();

        if (mx_type == "float")                         out = p.FLOAT1;
        else if (mx_type == "integer")                  out = p.INT;
        else if (mx_type == "boolean")                  out = p.BOOL;
        else if (mx_type == "vector2")                  out = p.FLOAT2;
        else if (mx_type == "vector3" || mx_type == "color3") out = p.FLOAT3;
        else if (mx_type == "vector4" || mx_type == "color4") out = p.FLOAT4;
        else return false;

        return true;
    }

    // "0.5, 0.5, 0.5" -> {0.5, 0.5, 0.5}; anything unparsable is skipped.
    std::vector<float> parse_floats(const std::string& s)
    {
        std::vector<float> res;
        std::stringstream ss(s);
        std::string item;

        while (std::getline(ss, item, ','))
        {
            try { res.push_back(std::stof(item)); }
            catch (const std::exception&) {}
        }

        return res;
    }

    // MaterialX geometry properties the graph can supply: its own texcoord and
    // position inputs. Normals/tangents/bitangents have no graph input yet, so
    // those stay at the zero default.
    bool is_uv_prop(const std::string& p) { return p == "UV0"; }
    bool is_position_prop(const std::string& p) { return p == "Pobject" || p == "Pworld"; }

    bool is_geometry_category(const std::string& c) { return c == "normal" || c == "tangent" || c == "bitangent"; }

    // Texture sampling isn't a plain function call (the texture is a bindless
    // index and the shader supplies the sampler and lod), so these two nodes
    // emit their own code in MaterialXNode::operator(). Their `file` input is
    // a TEXTURE port fed by a TextureAssetNode, like SamplingNode's.
    bool is_image_category(const std::string& c) { return c == "image" || c == "tiledimage"; }

    // Shading-frame global (mx_stdlib.hlsl) behind a defaultgeomprop: all of
    // N/T/B in any space resolve to the world-space frame. Anything else
    // (Pview, ...) has no source yet and stays zero.
    std::string geometry_global(const std::string& prop)
    {
        if (prop == "Nworld" || prop == "Nobject")
            return "mx_geom_normal";

        if (prop == "Tworld" || prop == "Tobject")
            return "mx_geom_tangent";

        if (prop == "Bworld" || prop == "Bobject")
            return "mx_geom_bitangent";

        return "float3(0.0, 0.0, 0.0)";
    }

    std::string trim(const std::string& s)
    {
        auto b = s.find_first_not_of(" \t");
        auto e = s.find_last_not_of(" \t");
        return b == std::string::npos ? std::string() : s.substr(b, e - b + 1);
    }

    // "1" stays valid inside float3(1, 0, 0), but a bare scalar default of
    // "1" would be an int literal -- spell floats as floats.
    std::string float_literal(const std::string& s)
    {
        auto t = trim(s);

        if (t.empty())
            return "0.0";

        if (t.find_first_of(".eE") == std::string::npos && t.find_first_of("nN") == std::string::npos)
            t += ".0";

        return t;
    }

    // HLSL literal for a MaterialX default value string ("0.5, 0.5, 0.5").
    // An absent value becomes zero of that type.
    std::string default_literal(const ShaderParamType& type, const std::string& value)
    {
        if (type.M == 3)  // INT
            return trim(value).empty() ? "0" : trim(value);

        if (type.M == 4)  // BOOL
            return trim(value) == "true" ? "true" : "false";

        if (type.N == 1)
            return float_literal(value);

        std::vector<std::string> comps;
        std::stringstream ss(value);
        std::string item;

        while (std::getline(ss, item, ','))
            comps.push_back(float_literal(item));

        // Missing or short values pad with zero rather than emit a call
        // with the wrong component count.
        while ((int)comps.size() < type.N)
            comps.push_back("0.0");

        std::string res = type.name + "(";

        for (int i = 0; i < type.N; i++)
            res += (i ? ", " : "") + comps[i];

        return res + ")";
    }

    // Names of every mx_* identifier that appears followed by '(' in the
    // stdlib source: the set of HLSL functions a nodedef can resolve to.
    std::set<std::string> scan_functions(const std::string& source)
    {
        std::set<std::string> res;

        for (size_t pos = source.find("mx_"); pos != std::string::npos; pos = source.find("mx_", pos + 1))
        {
            if (pos > 0 && (std::isalnum((unsigned char)source[pos - 1]) || source[pos - 1] == '_'))
                continue;

            size_t end = pos;

            while (end < source.size() && (std::isalnum((unsigned char)source[end]) || source[end] == '_'))
                end++;

            size_t next = end;

            while (next < source.size() && (source[next] == ' ' || source[next] == '\t'))
                next++;

            if (next < source.size() && source[next] == '(')
                res.insert(source.substr(pos, end - pos));
        }

        return res;
    }
}

size_t MaterialXLibrary::register_nodes(const std::filesystem::path& root, const std::filesystem::path& hlsl_stdlib)
{
    defs.clear();
    index.clear();

    std::string hlsl;
    {
        std::ifstream file(hlsl_stdlib, std::ios::binary);

        if (!file)
        {
            Log::get() << Log::LEVEL_ERROR << "MaterialX: cannot read " << hlsl_stdlib.string() << Log::endl;
            return 0;
        }

        std::stringstream ss;
        ss << file.rdbuf();
        hlsl = ss.str();

        // Implementations spill into generated files pulled in with #include
        // "name" (relative to the stdlib file), which count as implemented too.
        for (size_t pos = hlsl.find("#include \""); pos != std::string::npos; pos = hlsl.find("#include \"", pos + 1))
        {
            size_t begin = pos + 10;
            size_t end = hlsl.find('"', begin);

            if (end == std::string::npos)
                break;

            std::ifstream included(hlsl_stdlib.parent_path() / hlsl.substr(begin, end - begin), std::ios::binary);

            if (!included)
                continue;

            std::stringstream inc;
            inc << included.rdbuf();
            hlsl += "\n" + inc.str();
        }
    }

    auto implemented = scan_functions(hlsl);
    library_root = root;

    try
    {
        auto doc = MaterialX::createDocument();
        MaterialX::loadLibraries({ MaterialX::FilePath("libraries/stdlib") }, MaterialX::FileSearchPath(root.string()), doc);

        for (auto& nd : doc->getNodeDefs())
        {
            std::string function = "mx_" + nd->getNodeString();
            const std::string name = nd->getName();

            if (name.size() > 2 && name.compare(name.size() - 2, 2, "FA") == 0)
                function += "_fa";

            // A nodedef family whose members differ only in output type
            // (worleynoise2d float/vector2/vector3) can't share one HLSL
            // overload set, so it may name its variants mx_<category>_<type>.
            const bool is_image = is_image_category(nd->getNodeString());

            if (is_image)
                function = "mx_image";
            else if (!implemented.count(function))
            {
                auto outputs = nd->getActiveOutputs();

                if (outputs.empty())
                    continue;

                function += "_" + outputs[0]->getType();
            }

            if (!is_image && !implemented.count(function))
                continue;

            MxNodeDef def;
            def.name = name;
            def.category = nd->getNodeString();
            def.group = nd->getNodeGroup();
            def.function = function;

            bool supported = true;

            for (auto& in : nd->getActiveInputs())
            {
                MxPortDef port;
                port.name = in->getName();
                port.mx_type = in->getType();

                // The geometry nodes' `space` (model/object/world) is a string,
                // which the graph can't carry. They always answer in world
                // space, so the input is simply not exposed.
                if (port.mx_type == "string" && is_geometry_category(def.category))
                    continue;

                if (is_image)
                {
                    // Address modes, layers and frame ranges are strings the
                    // graph can't carry; the engine's sampler decides wrapping.
                    if (port.mx_type == "string" || port.name == "frameoffset")
                        continue;

                    if (port.mx_type == "filename")
                    {
                        port.type = ShaderParams::get().TEXTURE;
                        def.inputs.push_back(port);
                        continue;
                    }
                }

                supported &= map_type(port.mx_type, port.type);
                port.default_hlsl = supported ? default_literal(port.type, in->getValueString()) : "";
                port.geomprop = in->getAttribute("defaultgeomprop");

                // Frame vectors come from the shader's shading-frame globals
                // (mx_set_geometry); an unconnected input reads them directly.
                if (!port.geomprop.empty() && !is_uv_prop(port.geomprop) && !is_position_prop(port.geomprop))
                    port.default_hlsl = geometry_global(port.geomprop);

                if (supported && port.type.M == 1)
                {
                    port.default_values = parse_floats(in->getValueString());
                    port.default_values.resize(port.type.N, 0.0f);

                    // Slider range: MaterialX's own soft limits when given,
                    // otherwise [0,1] widened to cover the default.
                    auto lo = parse_floats(in->getAttribute("uisoftmin"));
                    auto hi = parse_floats(in->getAttribute("uisoftmax"));
                    float d = port.default_values[0];

                    port.soft_min = lo.empty() ? std::min(0.0f, d * 2) : lo[0];
                    port.soft_max = hi.empty() ? std::max(1.0f, d * 2) : hi[0];
                }

                def.inputs.push_back(port);
            }

            // A nodedef declares either explicit outputs (multi-output
            // nodes such as separate3) or just a `type`, meaning one
            // output named "out".
            auto outputs = nd->getActiveOutputs();

            if (outputs.empty())
            {
                MxPortDef port;
                port.name = "out";
                port.mx_type = nd->getType();
                supported &= map_type(port.mx_type, port.type);
                def.outputs.push_back(port);
            }
            else
                for (auto& out : outputs)
                {
                    MxPortDef port;
                    port.name = out->getName();
                    port.mx_type = out->getType();
                    supported &= map_type(port.mx_type, port.type);
                    def.outputs.push_back(port);
                }

            if (!supported || def.outputs.empty())
                continue;

            index[def.name] = defs.size();
            defs.push_back(std::move(def));
        }
    }
    catch (const std::exception& e)
    {
        Log::get() << Log::LEVEL_ERROR << "MaterialX: failed to load standard library from "
            << root.string() << ": " << e.what() << Log::endl;
        defs.clear();
        index.clear();
        return 0;
    }

    // Registered in (group, name) order so each submenu reads alphabetically
    // regardless of the order nodedefs appear in the library file.
    std::vector<const MxNodeDef*> ordered;

    for (auto& def : defs)
        ordered.push_back(&def);

    std::sort(ordered.begin(), ordered.end(), [](const MxNodeDef* a, const MxNodeDef* b)
        {
            return std::tie(a->group, a->name) < std::tie(b->group, b->name);
        });

    for (auto* def_ptr : ordered)
    {
        auto& def = *def_ptr;
        std::string short_name = def.name.rfind("ND_", 0) == 0 ? def.name.substr(3) : def.name;
        auto copy = def;

        FlowGraph::FlowSystem::get().register_node("MaterialX/" + def.group + "/" + short_name, [copy]() -> FlowGraph::window::ptr
            {
                return std::make_shared<MaterialXNode>(copy);
            });
    }

    Log::get() << Log::LEVEL_INFO << "MaterialX: registered " << defs.size() << " nodes" << Log::endl;
    return defs.size();
}

const MxNodeDef* MaterialXLibrary::find(const std::string& nodedef_name) const
{
    auto it = index.find(nodedef_name);
    return it == index.end() ? nullptr : &defs[it->second];
}

MaterialXNode::MaterialXNode(const MxNodeDef& def) : nodedef(def.name)
{
    // Title shown on the node: the nodedef name without MaterialX's ND_ prefix.
    name = def.name.rfind("ND_", 0) == 0 ? def.name.substr(3) : def.name;

    for (auto& in : def.inputs)
        register_input(in.name, in.type);

    for (auto& out : def.outputs)
        register_output(out.name, out.type);

    for (auto& in : def.inputs)
    {
        Uniform::ptr u;

        if (in.type.M == 1 && in.geomprop.empty())
        {
            u = std::make_shared<Uniform>();
            u->name = in.name;
            u->type = in.type;

            for (int c = 0; c < in.type.N && c < (int)in.default_values.size(); c++)
                std::memcpy(&u->value.raw_data[c], &in.default_values[c], sizeof(float));
        }

        uniforms.push_back(u);
    }
}

void MaterialXNode::on_add(::FlowGraph::graph* g)
{
    MaterialNode::on_add(g);

    auto* mat_graph = dynamic_cast<MaterialGraph*>(g);
    auto* def = MaterialXLibrary::get().find(nodedef);

    if (!mat_graph || !def)
        return;

    // A real, visible wire rather than a hidden default: the graph's
    // texcoord/position values only exist as that function's own parameters,
    // so there is nothing to name from inside this node.
    for (size_t i = 0; i < def->inputs.size() && i < input_parametres.size(); i++)
    {
        if (input_parametres[i]->has_input())
            continue;

        if (is_uv_prop(def->inputs[i].geomprop))
            mat_graph->get_texcoord()->link(input_parametres[i]);
        else if (is_position_prop(def->inputs[i].geomprop))
            mat_graph->get_position()->link(input_parametres[i]);
    }
}

GUI::base::ptr MaterialXNode::create_editor_window()
{
    auto* def = MaterialXLibrary::get().find(nodedef);
    auto panel = std::make_shared<GUI::Elements::layouts::vertical>();
    panel->docking     = GUI::dock::TOP;
    // The node body sizes itself from its children, so rows docked TOP have
    // no parent width to stretch to unless the panel fixes one.
    panel->width_size  = GUI::size_type::FIXED;
    panel->height_size = GUI::size_type::MATCH_CHILDREN;
    bool any_slider = false;

    // The color picker is wider than a slider row needs to be.
    bool any_color = false;

    for (size_t i = 0; def && i < def->inputs.size() && i < uniforms.size(); i++)
        any_color |= uniforms[i] && (def->inputs[i].mx_type == "color3" || def->inputs[i].mx_type == "color4");

    panel->size = { any_color ? 300.0f : 200.0f, 0 };

    for (size_t i = 0; def && i < def->inputs.size() && i < uniforms.size(); i++)
    {
        auto uniform = uniforms[i];

        if (!uniform)
            continue;

        auto& port = def->inputs[i];

        // Colors get the same picker the Vector node uses; alpha (color4) is
        // the picker's own alpha slider, and is ignored for color3.
        if (port.mx_type == "color3" || port.mx_type == "color4")
        {
            auto make_gap = []
            {
                auto gap = std::make_shared<GUI::base>();
                gap->docking     = GUI::dock::TOP;
                gap->height_size = GUI::size_type::FIXED;
                gap->size        = { 0, 8 };
                return gap;
            };

            panel->add_child(make_gap());

            auto title = std::make_shared<GUI::Elements::label>();
            title->text        = port.name;
            title->docking     = GUI::dock::TOP;
            title->height_size = GUI::size_type::FIXED;
            title->size        = { 0, 18 };
            title->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
            panel->add_child(title);

            float rgba[4] = { 0, 0, 0, 1 };

            for (int c = 0; c < port.type.N; c++)
                std::memcpy(&rgba[c], &uniform->value.raw_data[c], sizeof(float));

            const int components = port.type.N;
            auto picker = std::make_shared<GUI::Elements::color_picker>();
            picker->docking = GUI::dock::TOP;
            picker->set_channels(components == 3 ? GUI::Elements::color_channels::RGB : GUI::Elements::color_channels::RGBA);
            picker->set_color(float4(rgba[0], rgba[1], rgba[2], rgba[3]));
            picker->on_change.register_handler(nullptr, [uniform, components](float4 color)
            {
                float v[4] = { color.x, color.y, color.z, color.w };

                for (int c = 0; c < components; c++)
                    std::memcpy(&uniform->value.raw_data[c], &v[c], sizeof(float));

                uniform->on_change(uniform.get());
            });
            panel->add_child(picker);
            panel->add_child(make_gap());

            any_slider = true;
            continue;
        }

        for (int c = 0; c < port.type.N; c++)
        {
            float current;
            std::memcpy(&current, &uniform->value.raw_data[c], sizeof(float));

            auto row = std::make_shared<GUI::base>();
            row->docking     = GUI::dock::TOP;
            row->height_size = GUI::size_type::FIXED;
            row->size        = { 0, 22 };

            auto label = std::make_shared<GUI::Elements::label>();
            label->text        = port.type.N == 1 ? port.name : port.name + "." + "xyzw"[c];
            label->docking     = GUI::dock::LEFT;
            label->width_size  = GUI::size_type::FIXED;
            label->size        = { 70, 0 };
            label->magnet_text = FW1_LEFT | FW1_VCENTER | FW1_NOWORDWRAP;
            row->add_child(label);

            auto slider = std::make_shared<GUI::Elements::float_slider>();
            slider->min         = port.soft_min;
            slider->max         = port.soft_max;
            slider->value       = current;
            slider->docking     = GUI::dock::FILL;
            slider->height_size = GUI::size_type::FIXED;
            slider->size        = { 0, 14 };
            slider->y_type      = GUI::pos_y_type::CENTER;
            slider->on_change   = [uniform, c](float v)
            {
                std::memcpy(&uniform->value.raw_data[c], &v, sizeof(float));
                uniform->on_change(uniform.get());
            };
            row->add_child(slider);

            panel->add_child(row);
            any_slider = true;
        }
    }

    // No editable constants (only int/bool or geometry inputs): keep the
    // node's default live preview.
    if (!any_slider)
        return MaterialNode::create_editor_window();

    return panel;
}

void MaterialXNode::reset_for()
{
    MaterialNode::reset_for();

    auto* def = MaterialXLibrary::get().find(nodedef);

    if (!def)
        return;

    // A node only runs once every input has been set, and an unconnected
    // input is never set by anyone -- seed those with the nodedef default,
    // exactly as if a constant node were wired in.
    for (size_t i = 0; i < input_parametres.size() && i < def->inputs.size(); i++)
        if (!input_parametres[i]->has_input())
            input_parametres[i]->put(shader_parameter(def->inputs[i].default_hlsl, def->inputs[i].type));
}

void MaterialXNode::operator()(MaterialContext* c)
{
    auto* def = MaterialXLibrary::get().find(nodedef);

    // The registry is rebuilt from the stdlib files at startup; a graph that
    // references a nodedef this build doesn't implement just emits nothing.
    if (!def)
    {
        Log::get() << Log::LEVEL_WARNING << "MaterialX: unknown nodedef " << nodedef << Log::endl;
        return;
    }

    auto mat_graph = static_cast<MaterialFunction*>(owner);

    std::string args;
    std::vector<shader_parameter> values;

    for (size_t i = 0; i < def->inputs.size(); i++)
    {
        auto& port = input_parametres[i];

        // Unconnected float inputs read their editable uniform; connected ones
        // (and the int/bool baked defaults) use the value that arrived.
        auto value = (!port->has_input() && i < uniforms.size() && uniforms[i])
            ? c->create_value(uniforms[i])
            : port->get<shader_parameter>();

        values.push_back(value);

        if (def->inputs[i].type != ShaderParams::get().TEXTURE)
            args += (i ? ", " : "") + coerce(value, def->inputs[i].type);
    }

    if (is_image_category(def->category))
    {
        auto find = [&](const char* input_name)
        {
            for (size_t i = 0; i < def->inputs.size(); i++)
                if (def->inputs[i].name == input_name)
                    return (int)i;

            return -1;
        };

        auto get = [&](const char* input_name, const ShaderParamType& type)
        {
            int i = find(input_name);
            return i < 0 ? std::string() : coerce(values[i], type);
        };

        auto& out_type = def->outputs[0].type;
        const int file = find("file");

        // Nothing linked to `file` (the seeded value of an unconnected input
        // is not an input): the MaterialX `default`, as if the image were missing.
        if (file < 0 || !input_parametres[file]->has_input())
        {
            output_parametres[0]->put(mat_graph->add_value(out_type, get("default", out_type)));
            return;
        }

        std::string uv = get("texcoord", ShaderParams::get().FLOAT2);

        // tiledimage's nodegraph: ((uv * tiling - offset) / imagesize) * tilesize.
        if (def->category == "tiledimage")
            uv = "(((" + uv + " * " + get("uvtiling", ShaderParams::get().FLOAT2) + " - " + get("uvoffset", ShaderParams::get().FLOAT2)
                + ") / " + get("realworldimagesize", ShaderParams::get().FLOAT2) + ") * " + get("realworldtilesize", ShaderParams::get().FLOAT2) + ")";

        // Same call SamplingNode emits; `Sampler` and `lod` are the shader's own.
        auto sampled = mat_graph->add_value(ShaderParams::get().FLOAT4,
            "sample(" + values[file].name + ", Sampler, " + uv + ", lod)");

        static const char* swizzle[] = { "", ".x", ".xy", ".xyz", "" };
        output_parametres[0]->put(mat_graph->add_value(out_type, sampled.name + swizzle[out_type.N]));
        return;
    }

    if (def->outputs.size() == 1)
    {
        auto res = mat_graph->add_value(def->outputs[0].type, def->function + "(" + args + ")");
        output_parametres[0]->put(res);
        return;
    }

    // Several outputs: declare each, then pass them as trailing out-parameters.
    std::vector<shader_parameter> outs;
    std::string call = def->function + "(" + args;

    for (auto& out : def->outputs)
    {
        outs.push_back(mat_graph->add_value(out.type));
        call += (call.back() == '(' ? "" : ", ") + outs.back().name;
    }

    mat_graph->add_function(call + ")");

    for (size_t i = 0; i < outs.size(); i++)
        output_parametres[i]->put(outs[i]);
}

bool MaterialXNode::set_constant(const std::string& input, const std::vector<float>& values)
{
    auto* def = MaterialXLibrary::get().find(nodedef);

    if (!def)
        return false;

    for (size_t i = 0; i < def->inputs.size() && i < uniforms.size(); i++)
    {
        if (def->inputs[i].name != input || !uniforms[i])
            continue;

        for (int c = 0; c < def->inputs[i].type.N && c < (int)values.size(); c++)
            std::memcpy(&uniforms[i]->value.raw_data[c], &values[c], sizeof(float));

        return true;
    }

    return false;
}

// ---- .mtlx import -----------------------------------------------------------

namespace
{
    using OutPtr = ::FlowGraph::output::ptr;

    // A surface-shader parameter: wired to another node's output, or a constant.
    struct Channel
    {
        OutPtr src;
        std::vector<float> value;
    };

    // Which parameter of each surface shader feeds which output of the
    // material graph. The graph has one base color, metallic, roughness,
    // glow, opacity and normal; everything else the shader models (sheen,
    // coat, subsurface, transmission, ...) is dropped.
    struct SurfaceMapping
    {
        const char* category;
        const char* base_color;
        const char* base_weight;      // multiplies base_color; "" = none
        const char* metalness;
        const char* roughness;
        const char* emission_color;
        const char* emission_weight;  // "" = none
        const char* opacity;
        const char* normal;
    };

    const SurfaceMapping surface_mappings[] =
    {
        { "standard_surface",  "base_color",   "base",        "metalness",      "specular_roughness", "emission_color", "emission",             "opacity",          "normal" },
        { "open_pbr_surface",  "base_color",   "base_weight", "base_metalness", "specular_roughness", "emission_color", "emission_luminance",   "geometry_opacity", "geometry_normal" },
        { "gltf_pbr",          "base_color",   "",            "metallic",       "roughness",          "emissive",       "emissive_strength",    "alpha",            "normal" },
        { "UsdPreviewSurface", "diffuseColor", "",            "metallic",       "roughness",          "emissiveColor",  "",                     "opacity",          "normal" },
    };

    class MtlxImporter
    {
        MaterialGraph::ptr graph;
        std::filesystem::path dir;
        std::map<std::string, std::shared_ptr<MaterialXNode>> built;
        std::map<std::string, TextureAsset::ptr> textures;
        std::vector<std::pair<::FlowGraph::window::ptr, int>> placed;

        void place(const ::FlowGraph::window::ptr& node, int depth)
        {
            graph->register_node(node);
            placed.push_back({ node, depth });
        }

        // The graph auto-wires UV/position inputs when a node is added; an
        // explicit connection from the file replaces that.
        void unlink_auto(const ::FlowGraph::input::ptr& port)
        {
            graph->get_texcoord()->unlink(port);
            graph->get_position()->unlink(port);
        }

        static std::string value_string(const MaterialX::InputPtr& in)
        {
            if (in->hasInterfaceName())
                if (auto iface = in->getInterfaceInput())
                    return iface->getResolvedValueString();

            return in->getResolvedValueString();
        }

        OutPtr texture_source(const std::string& file, bool to_linear)
        {
            std::filesystem::path full = std::filesystem::path(file).is_absolute() ? std::filesystem::path(file) : dir / file;
            auto& asset = textures[full.string()];

            if (!asset)
            {
                auto* raw = new TextureAsset(full);
                raw->try_register();
                asset = raw->get_ptr<TextureAsset>();
            }

            auto node = std::make_shared<TextureAssetNode>(asset, to_linear);
            place(node, 1);
            return node->get_output(0);
        }

        OutPtr output_of(const std::shared_ptr<MaterialXNode>& node, const MxNodeDef& def, const std::string& name)
        {
            for (size_t i = 0; i < def.outputs.size(); i++)
                if (name.empty() || def.outputs[i].name == name)
                    return node->get_output((int)i);

            return node->get_output(0);
        }

        OutPtr build(const MaterialX::NodePtr& node, const std::string& out_name, int depth)
        {
            auto nodedef = node->getNodeDef();
            auto* def = nodedef ? MaterialXLibrary::get().find(nodedef->getName()) : nullptr;

            if (!def)
            {
                Log::get() << Log::LEVEL_WARNING << "MaterialX import: node '" << node->getName() << "' ("
                    << node->getCategory() << ") has no implementation here; its connections are left at defaults" << Log::endl;
                return nullptr;
            }

            auto& mx = built[node->getNamePath()];

            if (!mx)
            {
                mx = std::make_shared<MaterialXNode>(*def);
                place(mx, depth);

                for (size_t i = 0; i < def->inputs.size(); i++)
                    if (auto in = node->getInput(def->inputs[i].name))
                        connect(*mx, *def, i, in, depth + 1);
            }

            return output_of(mx, *def, out_name);
        }

        OutPtr source_of(const MaterialX::InputPtr& in, int depth)
        {
            if (in->hasInterfaceName())
            {
                auto iface = in->getInterfaceInput();
                return iface ? source_of(iface, depth) : nullptr;
            }

            if (!in->hasNodeName() && !in->hasNodeGraphString())
                return nullptr;

            if (auto n = in->getConnectedNode())
                return build(n, in->getOutputString(), depth);

            if (auto o = in->getConnectedOutput())
                if (auto n = o->getConnectedNode())
                    return build(n, o->getOutputString(), depth);

            return nullptr;
        }

        void connect(MaterialXNode& mx, const MxNodeDef& def, size_t i, const MaterialX::InputPtr& in, int depth)
        {
            auto port = mx.get_input((int)i);
            auto& pd = def.inputs[i];

            if (pd.type == ShaderParams::get().TEXTURE)
            {
                std::string file = value_string(in);

                if (file.empty())
                    return;

                const bool color = def.outputs[0].mx_type == "color3" || def.outputs[0].mx_type == "color4";
                texture_source(file, color)->link(port);
                return;
            }

            if (auto src = source_of(in, depth))
            {
                unlink_auto(port);
                src->link(port);
                return;
            }

            std::string text = value_string(in);

            if (text.empty())
                return;

            if (pd.type.M == 1)
            {
                mx.set_constant(pd.name, parse_floats(text));
                return;
            }

            // int/bool inputs are baked into the shader text from the nodedef
            // default, so a different value arrives as a constant node (the
            // link casts float to int).
            float v = 0;

            try { v = text == "true" ? 1.0f : text == "false" ? 0.0f : std::stof(text); }
            catch (const std::exception&) { return; }

            auto constant = std::make_shared<ScalarNode>(v);
            place(constant, depth);
            constant->get_output(0)->link(port);
        }

        Channel channel(const MaterialX::NodePtr& shader, const char* name, int depth)
        {
            Channel res;

            if (!*name)
                return res;

            if (auto in = shader->getInput(name))
            {
                if ((res.src = source_of(in, depth)))
                    return res;

                res.value = parse_floats(value_string(in));
            }

            if (res.value.empty())
                if (auto nd = shader->getNodeDef())
                    if (auto di = nd->getInput(name))
                        res.value = parse_floats(di->getValueString());

            return res;
        }

        static bool is_one(const Channel& c)
        {
            return !c.src && c.value.size() == 1 && std::abs(c.value[0] - 1.0f) < 1e-6f;
        }

        // color * weight, folded when both are constants and skipped when the
        // weight is a constant 1.
        Channel scale(const Channel& color, const Channel& weight, int depth)
        {
            if (is_one(weight) || (!weight.src && weight.value.empty()))
                return color;

            if (!color.src && !weight.src)
            {
                Channel res = color;

                for (auto& v : res.value)
                    v *= weight.value[0];

                return res;
            }

            auto* def = MaterialXLibrary::get().find("ND_multiply_color3FA");

            if (!def)
                return color;

            auto node = std::make_shared<MaterialXNode>(*def);
            place(node, depth);
            feed(*node, 0, "in1", color);
            feed(*node, 1, "in2", weight);

            Channel res;
            res.src = node->get_output(0);
            return res;
        }

        void feed(MaterialXNode& node, int index, const char* name, const Channel& c)
        {
            if (c.src)
            {
                auto port = node.get_input(index);
                unlink_auto(port);
                c.src->link(port);
            }
            else
                node.set_constant(name, c.value);
        }

        void emit(const Channel& c, const OutPtr& target, int components)
        {
            if (c.src)
            {
                c.src->link(target);
                return;
            }

            if (c.value.empty())
                return;

            if (components == 1)
            {
                auto node = std::make_shared<ScalarNode>(c.value[0]);
                place(node, 0);
                node->get_output(0)->link(target);
                return;
            }

            float v[4] = { c.value[0], c.value[0], c.value[0], 1.0f };

            if (c.value.size() >= 3)
                for (int i = 0; i < 3; i++)
                    v[i] = c.value[i];

            if (c.value.size() >= 4)
                v[3] = c.value[3];

            auto node = std::make_shared<VectorNode>(vec4(v[0], v[1], v[2], v[3]));
            place(node, 0);
            node->get_output(0)->link(target);
        }

        static bool nonzero(const Channel& c)
        {
            if (c.src)
                return true;

            for (float v : c.value)
                if (v != 0.0f)
                    return true;

            return false;
        }

    public:
        MtlxImporter(MaterialGraph::ptr g, std::filesystem::path d) : graph(std::move(g)), dir(std::move(d)) {}

        bool build_surface(const MaterialX::NodePtr& shader)
        {
            const SurfaceMapping* map = nullptr;

            for (auto& m : surface_mappings)
                if (shader->getCategory() == m.category)
                    map = &m;

            if (!map)
            {
                Log::get() << Log::LEVEL_WARNING << "MaterialX import: surface shader '" << shader->getCategory()
                    << "' is not supported (standard_surface, open_pbr_surface, gltf_pbr, UsdPreviewSurface are)" << Log::endl;
                return false;
            }

            const int d = 1;

            emit(scale(channel(shader, map->base_color, d), channel(shader, map->base_weight, d), d), graph->get_base_color(), 4);
            emit(channel(shader, map->metalness, d), graph->get_mettalic(), 1);
            emit(channel(shader, map->roughness, d), graph->get_roughness(), 1);

            auto emission = scale(channel(shader, map->emission_color, d), channel(shader, map->emission_weight, d), d);

            if (nonzero(emission))
                emit(emission, graph->get_glow(), 4);

            auto opacity = channel(shader, map->opacity, d);

            // Only wire opacity when it does something; an all-ones constant
            // is the graph's default.
            bool opaque = !opacity.src;

            for (float v : opacity.value)
                opaque &= v >= 0.999f;

            if (!opaque)
                emit(opacity, graph->get_opacity(), 1);

            if (auto normal = shader->getInput(map->normal))
            {
                auto n = normal->getConnectedNode();

                // The graph's normal output takes the tangent-space texel as
                // stored; MaterialX's normalmap node decodes it, so the map
                // goes straight to the output instead.
                if (n && n->getCategory() == "normalmap")
                {
                    if (auto in = n->getInput("in"))
                        if (auto src = source_of(in, d))
                            src->link(graph->get_normals());
                }
                else if (n || normal->hasNodeGraphString())
                    Log::get() << Log::LEVEL_WARNING << "MaterialX import: only normalmap nodes are supported on the normal input" << Log::endl;
            }

            return true;
        }

        // Columns by distance from the graph output, rows in creation order.
        void layout()
        {
            int max_depth = 0;

            for (auto& p : placed)
                max_depth = std::max(max_depth, p.second);

            std::map<int, int> rows;

            for (auto& p : placed)
                p.first->pos = vec2(40.0f + (max_depth - p.second) * 330.0f, 40.0f + rows[p.second]++ * 300.0f);
        }
    };
}

size_t MaterialXLibrary::import_file(const std::filesystem::path& file)
{
    struct Shader { std::string name; MaterialX::NodePtr node; };
    std::vector<Shader> shaders;
    MaterialX::DocumentPtr doc;

    try
    {
        auto lib = MaterialX::createDocument();
        MaterialX::FilePathVec folders;

        for (const char* name : { "libraries/stdlib", "libraries/pbrlib", "libraries/bxdf" })
            if (std::filesystem::exists(library_root / name))
                folders.push_back(MaterialX::FilePath(name));

        MaterialX::loadLibraries(folders, MaterialX::FileSearchPath(library_root.string()), lib);

        doc = MaterialX::createDocument();
        MaterialX::readFromXmlFile(doc, MaterialX::FilePath(file.string()), MaterialX::FileSearchPath(file.parent_path().string()));
        doc->importLibrary(lib);

        for (auto& m : doc->getMaterialNodes())
            if (auto in = m->getInput("surfaceshader"))
                if (auto shader = in->getConnectedNode())
                    shaders.push_back({ m->getName(), shader });

        if (shaders.empty())
            for (auto& n : doc->getNodes())
                if (n->getType() == "surfaceshader")
                    shaders.push_back({ n->getName(), n });
    }
    catch (const std::exception& e)
    {
        Log::get() << Log::LEVEL_ERROR << "MaterialX import: cannot read " << file.string() << ": " << e.what() << Log::endl;
        return 0;
    }

    if (shaders.empty())
    {
        Log::get() << Log::LEVEL_WARNING << "MaterialX import: " << file.string() << " has no surface material" << Log::endl;
        return 0;
    }

    size_t count = 0;

    for (auto& shader : shaders)
    {
        auto graph = std::make_shared<MaterialGraph>();
        MtlxImporter importer(graph, file.parent_path());

        if (!importer.build_surface(shader.node))
            continue;

        importer.layout();

        auto* material = new materials::universal_material(graph);
        material->register_new(std::wstring(shader.name.begin(), shader.name.end()));
        count++;
    }

    Log::get() << Log::LEVEL_INFO << "MaterialX import: " << file.string() << " -> " << count << " material(s)" << Log::endl;
    return count;
}
