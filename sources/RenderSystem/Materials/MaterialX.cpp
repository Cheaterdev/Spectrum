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
    }

    auto implemented = scan_functions(hlsl);

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

            if (!implemented.count(function))
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
                supported &= map_type(port.mx_type, port.type);
                port.default_hlsl = supported ? default_literal(port.type, in->getValueString()) : "";
                port.geomprop = in->getAttribute("defaultgeomprop");

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

    for (size_t i = 0; i < def->inputs.size(); i++)
    {
        auto& port = input_parametres[i];

        // Unconnected float inputs read their editable uniform; connected ones
        // (and the int/bool baked defaults) use the value that arrived.
        auto value = (!port->has_input() && i < uniforms.size() && uniforms[i])
            ? c->create_value(uniforms[i])
            : port->get<shader_parameter>();

        args += (i ? ", " : "") + coerce(value, def->inputs[i].type);
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
