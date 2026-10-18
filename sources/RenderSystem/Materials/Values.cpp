module Graphics:Materials.Values;


import GUI;

import :Asset;
import :TextureAsset;
import :AssetExplorer;

/*
int get_size(FlowGraph::data_types v)
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
}

*/

std::string shader_parameter::to_string()
{
	return type.name;
}
shader_parameter::shader_parameter(std::string str, ShaderParamType type)
{
	this->type = type;
	name = str;
	/*
				if (type == ShaderParams::get().FLOAT1)
					name = other.name + ".x";

				if (type == ShaderParams::get().FLOAT2)
					name = other.name + ".xy";

				if (type == ShaderParams::get().FLOAT3)
					name = other.name + ".xyz";

				if (type == ShaderParams::get().FLOAT4)
					name = other.name + ".xyzw";
					*/
}

void MaterialContext::clear_parameters()
{
	shader_parameter_uniform.clear();
	shader_parameter_srv.clear();
	textures.clear();
	//tiled_textures.clear();
	texture_counter = 0;
	unordered_counter = 0;

	uniforms.clear();
}

shader_parameter MaterialContext::create_value(Uniform::ptr f)
{
	std::string shader_name;

	auto it = shader_parameter_uniform.find(f);
	if (it == shader_parameter_uniform.end())
	{
		shader_name = get_new_param();
		shader_parameter_uniform[f] = shader_name;

		uniforms.emplace_back(f);
		uniform_struct += f->type.name;
		uniform_struct += " ";
		uniform_struct += shader_name;

		//uniform_struct += ":packoffset(c" + std::to_string(uniform_offset / 4) + ")";

		uniform_struct += ";\n";
		//uniform_offset += 4;// get_size(f->type);
	}
	else
		shader_name = it->second;




	/*uniform_struct += ":";
	uniform_struct += "packoffset(c";


	uniform_struct += std::to_string(uniform_offset/4);
	int fr=(uniform_offset)%4;
	if (fr == 1)    uniform_struct += ".y";
	if(fr==2)    uniform_struct += ".z";
	if (fr == 3)    uniform_struct += ".w";
	//if (fr == 4)    uniform_struct += ".w";


	uniform_struct += ")";*/

	// The preview build binds MaterialPreviewInfo (its own SIG table, see
	// material_preview.prism), not the production MaterialInfo -- same CB
	// layout/shader_name, different accessor.
	auto accessor = capture_preview ? "GetMaterialPreviewInfo()" : "GetMaterialInfo()";
	auto result = graph->add_value(f->type, std::string(accessor) + ".GetData()." + shader_name);
	// add_value() below always targets the root graph's text regardless of
	// which nested MaterialFunction is executing (pre-existing behavior,
	// not something this feature changes) -- so graph is the right owner
	// to append the capture write to as well.
	capture_value(graph, result);
	return result;
}

std::string MaterialContext::get_texture(TextureSRVParams::ptr& p)
{
	auto it = shader_parameter_srv.find(p);
	if (it != shader_parameter_srv.end())
	{

		return it->second;
	}

	textures.push_back(p);
	auto res = std::to_string(texture_counter++);
	shader_parameter_srv[p] = res;

	return res;
}

shader_parameter MaterialContext::get_texture(TextureSRVParams::ptr& p, shader_parameter tc)
{
	textures.push_back(p);
	std::string str = "tile_sample(";
	str += std::string("get_texture(") + std::to_string(texture_counter++) + std::string(")");
	str += ",Sampler,";
	str += tc.name + ".xy,";
	str += std::string("get_texture(") + std::to_string(texture_counter++) + std::string("),");
	str += std::string("visibility[") + std::to_string(unordered_counter++) + std::string("]");
	str += ")";
	auto val = graph->add_value(ShaderParams::get().FLOAT4, str);
	return val;
	//textures.push_back(asset);
	//return std::string("textures[") + std::to_string(textures.size() - 1) + std::string("]");
}


std::string MaterialContext::generate_uniform_struct()
{


	std::string result = "struct MaterialCB\n{\n";
	result += uniform_struct;

	if (uniform_struct.empty())
		result += "int dummy;\n";

	result += "};\n";

	uniform_offset = 0;
	return result;
}

void MaterialContext::start(std::string orig_file, MaterialGraph* graph)
{
	pixel_shader = ShaderSource();
	voxel_shader = ShaderSource();
	tess_shader = ShaderSource();
	hit_shader = ShaderSource();
	preview_shader = ShaderSource();

	this->graph = graph;
	uniform_struct = "";
	clear_parameters();
	text = "";//file->get_data();
	// std::string universal = file->get_data();
	functions.clear();

	params = 0;
	graph->get_normals()->set_enabled(true);
	graph->get_glow()->set_enabled(true);
	graph->get_texcoord()->set_enabled(true);
	graph->get_mettalic()->set_enabled(true);
	graph->get_base_color()->set_enabled(true);
	graph->get_roughness()->set_enabled(true);
	graph->get_opacity()->set_enabled(true);
	graph->get_ior()->set_enabled(true);
	graph->set_translucent_outputs_enabled(true);

	graph->get_tess_displacement()->set_enabled(false);
	graph->start(this);

	transparent = graph->get_opacity()->has_input();



	{


		if (functions.find(graph) != functions.end())
		{
			pixel_shader.macros.emplace_back("COMPILED_FUNC", graph->func_name);
			voxel_shader.macros.emplace_back("COMPILED_FUNC", graph->func_name);
			hit_shader.macros.emplace_back("COMPILED_FUNC", graph->func_name);
		}




		pixel_shader.text = text;
		voxel_shader.text = text;
		hit_shader.text = text;


		if (textures.empty())
		{
			//	hit_shader.macros.emplace_back("REFRACTION", "");
		}
	}




	if (graph->get_tess_displacement()->has_input())
	{

		text = "";
		functions.clear();

		graph->get_normals()->set_enabled(false);
		graph->get_glow()->set_enabled(false);
		graph->get_texcoord()->set_enabled(false);
		graph->get_mettalic()->set_enabled(false);
		graph->get_roughness()->set_enabled(false);
		graph->get_opacity()->set_enabled(false);
		graph->get_ior()->set_enabled(false);
		graph->set_translucent_outputs_enabled(false);

		graph->get_base_color()->set_enabled(false);
		graph->get_tess_displacement()->set_enabled(true);
		graph->start(this);

		if (functions.find(graph) != functions.end())
		{
			tess_shader.macros.emplace_back("COMPILED_FUNC", graph->func_name);
		}

		tess_shader.text = text;

	}

	// Third pass: same reachable node set as the pixel path (so every node
	// visible in the final material gets a slice), but with capture_preview
	// on so add_value()/create_value() inject a capture write per node (see
	// MaterialContext::capture_value). Reuses shader_parameter_uniform/_srv's
	// dedup, so this doesn't duplicate uniforms/textures registered above.
	{
		text = "";
		functions.clear();
		preview_slice_counter = 0;
		node_preview_slot.clear();

		graph->get_normals()->set_enabled(true);
		graph->get_glow()->set_enabled(true);
		graph->get_texcoord()->set_enabled(true);
		graph->get_mettalic()->set_enabled(true);
		graph->get_base_color()->set_enabled(true);
		graph->get_roughness()->set_enabled(true);
		graph->get_opacity()->set_enabled(true);
		graph->get_ior()->set_enabled(true);
		graph->set_translucent_outputs_enabled(true);
		graph->get_tess_displacement()->set_enabled(false);

		// Unlike the pixel/tess passes above, the preview pass wants a value
		// for every node reachable from its inputs, even ones whose output
		// isn't wired to anything downstream yet (still being authored, or a
		// deliberate dead end) -- see graph::set_start_if_output_recursive.
		// Nodes with missing/unconnected *inputs* still can't run (nothing to
		// compute from), so this only surfaces previews for otherwise-valid,
		// just-unused nodes -- it doesn't change what the compiled shader
		// itself does, since that already finished compiling above.
		graph->set_start_if_output_recursive(false);
		capture_preview = true;
		graph->start(this);
		capture_preview = false;
		graph->set_start_if_output_recursive(true);

		if (functions.find(graph) != functions.end())
			preview_shader.macros.emplace_back("COMPILED_FUNC", graph->func_name);

		preview_shader.text = text;
	}

	std::string uniforms_string = generate_uniform_struct();

	pixel_shader.uniforms = uniforms_string;
	voxel_shader.uniforms = uniforms_string;
	hit_shader.uniforms = uniforms_string;
	tess_shader.uniforms = uniforms_string;
	preview_shader.uniforms = uniforms_string;

	uniforms_ps = uniforms;
	text = pixel_shader.text;
}

ShaderSource MaterialContext::get_tess_result()
{
	return tess_shader;
}

ShaderSource MaterialContext::get_preview_result()
{
	return preview_shader;
}

int MaterialContext::get_preview_slot(FlowGraph::Node* node)
{
	auto it = node_preview_slot.find(node);
	return it != node_preview_slot.end() ? it->second : -1;
}

int MaterialContext::get_preview_slot_count()
{
	return preview_slice_counter;
}

namespace
{
	// Preview slices are always float4 (see material_preview.prism); node values
	// can be narrower (scalars, float2/3), so pad/broadcast into one. Alpha is
	// always forced to 1 (even for genuine float4 values) -- the GUI draws
	// this alpha-blended, and a node's own .w (opacity/pack/whatever) isn't
	// meant to make its *preview thumbnail* translucent or invisible.
	//
	// preview_shade is a global set once per pixel in CS() (see
	// UniversalMaterialPreview.hlsl) -- 1 in flat 2D mode (no-op, same value
	// as before this existed), or a lit N.L term when PREVIEW_3D evaluates
	// this over an analytic sphere instead of a flat quad.
	std::string coerce_preview_value(const shader_parameter& val)
	{
		switch (val.type.N)
		{
		case 1:  return "float4((" + val.name + ").xxx * preview_shade, 1)";
		case 2:  return "float4(" + val.name + " * preview_shade, 0, 1)";
		case 3:  return "float4(" + val.name + " * preview_shade, 1)";
		default: return "float4((" + val.name + ").xyz * preview_shade, 1)";
		}
	}
}

void MaterialContext::capture_value(MaterialFunction* owner_func, const shader_parameter& val)
{
	if (!capture_preview || !capturing_node || !owner_func)
		return;

	// First captured value for a node wins the slot -- e.g. SpecToMetNode
	// generates two (albedo, metallic); the preview thumbnail just shows
	// whichever was computed first. Nodes that never reach here (their
	// operator() doesn't call add_value()/create_value() at all, e.g.
	// TiledTextureNode, currently unimplemented) simply get no slot --
	// get_preview_slot() returns -1, no thumbnail shown, not a crash.
	if (node_preview_slot.find(capturing_node) != node_preview_slot.end())
		return;

	int slice = preview_slice_counter++;
	node_preview_slot[capturing_node] = slice;

	// preview_tid, not id: this write lands inside COMPILED_FUNC's body, a
	// separate function from CS() where SV_DispatchThreadID is a parameter
	// -- see UniversalMaterialPreview.hlsl.
	owner_func->add_function(
		std::string("GetMaterialPreviewInfo().GetResults()[uint3(preview_tid.xy, ") +
		std::to_string(slice) + ")] = " + coerce_preview_value(val));
}

ShaderSource MaterialContext::get_pixel_result()
{
	return pixel_shader;
}

ShaderSource MaterialContext::get_voxel_result()
{
	return voxel_shader;
}

std::vector<TextureSRVParams::ptr> MaterialContext::get_textures()
{
	return textures;
}

std::vector<Uniform::ptr> MaterialContext::get_uniforms()
{
	return uniforms;
}

void MaterialContext::register_function(MaterialFunction* func, std::string text)
{
	functions[func] = text;
	this->text += text;
}

void MaterialContext::add_task(FlowGraph::Node* node)
{
	run(node);
}

std::string MaterialContext::get_new_param()
{
	return std::string("param_") + std::to_string(params++);
}

MaterialContext::~MaterialContext()
{
}

std::list<FlowGraph::Node::ptr>  MaterialFunction::on_drop(MyVariant value)
{
	auto item = value.get<GUI::Elements::asset_item::ptr>();

	std::list<FlowGraph::Node::ptr> result;
	if (!item)
		return result;

	if (item->asset->get_type() == Asset_Type::TEXTURE)
	{
		// Just the asset reference -- the user wires up a SamplingNode (with
		// a tc input) separately to actually sample it.
		TextureAssetNode::ptr node(new TextureAssetNode(item->asset->get_asset()->get_ptr<TextureAsset>()));
		register_node(node);
		result.push_back(node);
	}

	if (item->asset->get_type() == Asset_Type::TILED_TEXTURE)
	{
		//	TiledTextureNode::ptr node(new TiledTextureNode(item->asset->get_asset()->get_ptr<TiledTexture>()));
		//	register_node(node);
		//	result.push_back(node);
	}


	return result;

}



void MaterialFunction::on_finish()
{
	int o = 0;

	for (unsigned int i = 0; i < output_parametres.size(); i++)
	{
		if (!output_parametres[i]->enabled) continue;

		// A linked value can be a different numeric width/kind than the port
		// (links between numeric types are allowed, see ShaderParamType::
		// can_cast) -- convert to the port's own type, which is what the
		// `out` parameter was declared with in generate_output().
		auto value = output_parametres[i]->get<shader_parameter>();

		if (auto* dst = dynamic_cast<ShaderParamType*>(output_parametres[i]->type.get()))
			value.name = coerce(value, *dst);

		add_function(f_output_names[o++] + " = " + value.name);
	};

	for (unsigned int i = 0; i < outputs.size(); i++)
		output_parametres[i]->reset(outputs[i]);

	text += "}\n";
	auto mat_context = static_cast<MaterialContext*>(context);
	mat_context->register_function(this, text);

	if (owner)
	{
		auto mat_graph = static_cast<MaterialFunction*>(owner);
		//			mat_graph->add_value();
		mat_graph->add_function(func_call);
		/*
				o_vec4->put(val);
				o_r->put(shader_parameter(val.name + ".r", ShaderParams::get().FLOAT1));
				o_g->put(shader_parameter(val.name + ".g", ShaderParams::get().FLOAT1));
				o_b->put(shader_parameter(val.name + ".b", ShaderParams::get().FLOAT1));
				o_a->put(shader_parameter(val.name + ".a", ShaderParams::get().FLOAT1));
				*/
	}

	for (auto o : output_parametres)
		o->send_next();
}

std::string MaterialFunction::generate_struct(std::vector<FlowGraph::output::ptr>& parameters)
{
	std::string res;
	auto mat_context = static_cast<MaterialContext*>(context);
	std::string struct_name = mat_context->get_new_param();
	res += "struct " + mat_context->get_new_param() + "\n{\n";

	for (unsigned int i = 0; i < parameters.size(); i++)
	{
		std::string param_name = mat_context->get_new_param();
		res += parameters[i]->get<shader_parameter>().type.name + " " + param_name + ";\n";
	}

	res += "\n};";
	text += res;
	return struct_name;
}

std::string MaterialFunction::generate_input(std::vector<FlowGraph::input::ptr>& parameters)
{
	std::string res;
	auto mat_context = static_cast<MaterialContext*>(context);

	for (unsigned int i = 0; i < parameters.size(); i++)
	{
		std::string param_name = mat_context->get_new_param();
		res += "in " + parameters[i]->get<shader_parameter>().type.name + " " + param_name;

		if (owner)
			func_call += parameters[i]->get<shader_parameter>().name;

		if (i < parameters.size() - 1)
		{
			res += ", ";

			if (owner)
				func_call += ", ";
		}

		parameters[i]->reset(shader_parameter(param_name, parameters[i]->get<shader_parameter>().type));
	}

	return res;
}

std::string MaterialFunction::generate_output(std::vector<FlowGraph::output::ptr>& parameters)
{
	std::string res;
	auto mat_context = static_cast<MaterialContext*>(context);
	outputs.clear();
	f_output_names.clear();
	orig_names.clear();
	bool added = false;

	for (unsigned int i = 0; i < parameters.size(); i++)
	{
		if (!parameters[i]->enabled) continue;

		if (added)
		{
			res += ", ";
			func_call += ", ";
		}

		std::string param_name = mat_context->get_new_param();
		res += "out " + parameters[i]->get<shader_parameter>().type.name + " " + param_name;
		f_output_names.push_back(param_name);

		if (owner)
		{
			auto mat_graph = static_cast<MaterialFunction*>(owner);
			outputs.push_back(mat_graph->add_value(parameters[i]->get<shader_parameter>().type));
			func_call += outputs.back().name;
		}

		added = true;
	}

	return res;
}

void MaterialFunction::operator()(MaterialContext* mat_context)
{
	this->context = context;
	text = "";
	func_name = mat_context->get_new_param();
	auto delim = (input_parametres.size() && output_parametres.size()) ? " , " : "";
	func_call = func_name + "(";
	auto input_struct = generate_input(input_parametres);
	func_call += delim;
	auto output_struct = generate_output(output_parametres);
	func_call += ")";
	text += "void " + func_name + "( " + input_struct + delim + output_struct + ", float lod )\n{\n";
	FlowGraph::graph::operator()(context);
}

MaterialFunction::MaterialFunction()
{
	start_child_nodes = true;
}

MaterialFunction::~MaterialFunction()
{
}

shader_parameter MaterialFunction::add_value(const ShaderParamType& type)
{
	auto mat_context = static_cast<MaterialContext*>(context);
	auto param_name = mat_context->get_new_param();
	text += std::string("\t") + type.name + " " + param_name + ";\n";
	shader_parameter result;
	result.name = param_name;
	result.type = type;
	return result;
}

shader_parameter MaterialFunction::add_value(const ShaderParamType& type, std::string s)
{
	auto mat_context = static_cast<MaterialContext*>(context);
	auto param_name = mat_context->get_new_param();
	text += std::string("\t") + type.name + " " + param_name + " = " + s + ";\n";
	shader_parameter result;
	result.name = param_name;
	result.type = type;
	mat_context->capture_value(this, result);
	return result;
}

void MaterialFunction::add_function(std::string s)
{
	text += std::string("\t") + s + ";\n";
}
REGISTER_MATERIAL_NODE(ScalarNode);
REGISTER_MATERIAL_NODE(MulNode);
REGISTER_MATERIAL_NODE(SumNode);
REGISTER_MATERIAL_NODE(TextureAssetNode);
REGISTER_MATERIAL_NODE(SamplingNode);
REGISTER_MATERIAL_NODE(VectorNode);
REGISTER_MATERIAL_NODE(TiledTextureNode);
REGISTER_MATERIAL_NODE(MaterialGraph);
REGISTER_MATERIAL_NODE(SpecToMetNode);
REGISTER_MATERIAL_NODE(ReconstructNormalZNode);





REGISTER_TYPE(ScalarNode);
REGISTER_TYPE(MulNode);
REGISTER_TYPE(SumNode);
REGISTER_TYPE(TextureAssetNode);
REGISTER_TYPE(SamplingNode);
REGISTER_TYPE(VectorNode);
REGISTER_TYPE(TiledTextureNode);
REGISTER_TYPE(MaterialGraph);
REGISTER_TYPE(SpecToMetNode);
REGISTER_TYPE(ReconstructNormalZNode);
//REGISTER_TYPE(TextureSRVParams);
REGISTER_TYPE(ShaderParamType);
REGISTER_TYPE(VectorType);

CEREAL_FORCE_REGISTER(ScalarNode);
CEREAL_FORCE_REGISTER(MulNode);
CEREAL_FORCE_REGISTER(SumNode);
CEREAL_FORCE_REGISTER(TextureAssetNode);
CEREAL_FORCE_REGISTER(SamplingNode);
CEREAL_FORCE_REGISTER(VectorNode);
CEREAL_FORCE_REGISTER(TiledTextureNode);
CEREAL_FORCE_REGISTER(MaterialGraph);
CEREAL_FORCE_REGISTER(SpecToMetNode);
CEREAL_FORCE_REGISTER(ReconstructNormalZNode);
CEREAL_FORCE_REGISTER(ShaderParamType);
CEREAL_FORCE_REGISTER(VectorType);

// ---- Polymorphic relations ----
// parameter_type subtypes (stored as unique_ptr<parameter_type> in parameter::type)
CEREAL_REGISTER_POLYMORPHIC_RELATION(::FlowGraph::parameter_type, ShaderParamType);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::FlowGraph::parameter_type, VectorType);
// Node subtypes (stored as shared_ptr<FlowGraph::Node> in graph::nodes)
CEREAL_REGISTER_POLYMORPHIC_RELATION(::FlowGraph::Node, MaterialNode);   // abstract; needed for transitive paths
CEREAL_REGISTER_POLYMORPHIC_RELATION(::FlowGraph::Node, TextureAssetNode); // SAVE_PARENT(Node) in serialize
CEREAL_REGISTER_POLYMORPHIC_RELATION(::FlowGraph::Node, SamplingNode);    // SAVE_PARENT(Node) in serialize
CEREAL_REGISTER_POLYMORPHIC_RELATION(::FlowGraph::Node, SumNode);
CEREAL_REGISTER_POLYMORPHIC_RELATION(::FlowGraph::Node, MulNode);
CEREAL_REGISTER_POLYMORPHIC_RELATION(MaterialNode,      ScalarNode);
CEREAL_REGISTER_POLYMORPHIC_RELATION(MaterialNode,      VectorNode);
CEREAL_REGISTER_POLYMORPHIC_RELATION(MaterialNode,      SpecToMetNode);
CEREAL_REGISTER_POLYMORPHIC_RELATION(MaterialNode,      ReconstructNormalZNode);
CEREAL_REGISTER_POLYMORPHIC_RELATION(MaterialNode,      TiledTextureNode);
// MaterialGraph chain (FlowGraph::graph→MaterialGraph, transitive Node→graph already in FlowGraph.cpp)
CEREAL_REGISTER_POLYMORPHIC_RELATION(::FlowGraph::graph, MaterialGraph);

// Force-init casters at static-init time — shallower pairs first so BFS
// builds transitive paths (e.g. Node→ScalarNode) in a single pass.
CEREAL_FORCE_REGISTER_RELATION(::FlowGraph::parameter_type, ShaderParamType);
CEREAL_FORCE_REGISTER_RELATION(::FlowGraph::parameter_type, VectorType);
CEREAL_FORCE_REGISTER_RELATION(::FlowGraph::Node,  MaterialNode);
CEREAL_FORCE_REGISTER_RELATION(::FlowGraph::Node,  TextureAssetNode);
CEREAL_FORCE_REGISTER_RELATION(::FlowGraph::Node,  SamplingNode);
CEREAL_FORCE_REGISTER_RELATION(::FlowGraph::Node,  SumNode);
CEREAL_FORCE_REGISTER_RELATION(::FlowGraph::Node,  MulNode);
CEREAL_FORCE_REGISTER_RELATION(MaterialNode,       ScalarNode);
CEREAL_FORCE_REGISTER_RELATION(MaterialNode,       VectorNode);
CEREAL_FORCE_REGISTER_RELATION(MaterialNode,       SpecToMetNode);
CEREAL_FORCE_REGISTER_RELATION(MaterialNode,       ReconstructNormalZNode);
CEREAL_FORCE_REGISTER_RELATION(MaterialNode,       TiledTextureNode);
CEREAL_FORCE_REGISTER_RELATION(::FlowGraph::graph, MaterialGraph);


MaterialGraph::MaterialGraph()
{
	position = register_input("position", ShaderParams::get().FLOAT3);
	texcoord = register_input(/*ShaderParams::get().FLOAT2,*/ "texcoord", ShaderParams::get().FLOAT2);
	i_base_color = register_output(/*ShaderParams::get().FLOAT4,*/ "base color", ShaderParams::get().FLOAT4);
	i_metallic = register_output(/*ShaderParams::get().FLOAT1,*/ "metallic", ShaderParams::get().FLOAT1);
	i_roughness = register_output(/*ShaderParams::get().FLOAT1, */"roughness", ShaderParams::get().FLOAT1);
	i_normal = register_output(/*ShaderParams::get().FLOAT4, */"normal", ShaderParams::get().FLOAT4);
	i_tess_displacement = register_output(/*ShaderParams::get().FLOAT1,*/ "displacement", ShaderParams::get().FLOAT1);
	i_glow = register_output(/*ShaderParams::get().FLOAT4, */"glow", ShaderParams::get().FLOAT4);
	// Registered after glow so they append to the end of the COMPILED_FUNC output list.
	i_opacity = register_output("opacity", ShaderParams::get().FLOAT1);
	i_refraction = register_output("ior", ShaderParams::get().FLOAT1);
	// Translucent-only (refraction wired): see TranslucentRTX, raytracing.prism.
	i_thickness = register_output("thickness", ShaderParams::get().FLOAT1);
	i_transmission = register_output("transmission weight", ShaderParams::get().FLOAT1);
	i_absorption_distance = register_output("transmission depth", ShaderParams::get().FLOAT1);
	i_coat = register_output("coat weight", ShaderParams::get().FLOAT1);
	i_coat_roughness = register_output("coat roughness", ShaderParams::get().FLOAT1);
	i_coat->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_coat_roughness->default_value = shader_parameter("0.1", ShaderParams::get().FLOAT1);
	i_sheen = register_output("sheen weight", ShaderParams::get().FLOAT1);
	i_sheen_roughness = register_output("sheen roughness", ShaderParams::get().FLOAT1);
	i_sheen->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_sheen_roughness->default_value = shader_parameter("0.3", ShaderParams::get().FLOAT1);
	i_anisotropy = register_output("anisotropy", ShaderParams::get().FLOAT1);
	i_anisotropy_rotation = register_output("anisotropy rotation", ShaderParams::get().FLOAT1);
	i_anisotropy->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_anisotropy_rotation->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_sss_weight = register_output("subsurface weight", ShaderParams::get().FLOAT1);
	i_sss_radius = register_output("subsurface radius", ShaderParams::get().FLOAT3);
	i_sss_scale = register_output("subsurface scale", ShaderParams::get().FLOAT1);
	i_sss_weight->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_sss_radius->default_value = shader_parameter("float3(1,1,1)", ShaderParams::get().FLOAT3);
	i_sss_scale->default_value = shader_parameter("1.0", ShaderParams::get().FLOAT1);
	i_sheen_tint = register_output("sheen tint", ShaderParams::get().FLOAT1);
	i_sheen_tint->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_transmission_color = register_output("transmission color", ShaderParams::get().FLOAT3);
	i_transmission_color->default_value = shader_parameter("float3(-1,-1,-1)", ShaderParams::get().FLOAT3);
	i_opacity_threshold = register_output("opacity threshold", ShaderParams::get().FLOAT1);
	i_opacity_threshold->default_value = shader_parameter("0.5", ShaderParams::get().FLOAT1);
	i_thin_film_thickness = register_output("thin film thickness", ShaderParams::get().FLOAT1);
	i_thin_film_ior = register_output("thin film ior", ShaderParams::get().FLOAT1);
	i_thin_film_thickness->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_thin_film_ior->default_value = shader_parameter("1.5", ShaderParams::get().FLOAT1);

	i_base_color->default_value = shader_parameter("float4(0,0,0,1)", ShaderParams::get().FLOAT4);
	i_metallic->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_roughness->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);

	i_normal->default_value = shader_parameter("float4(0.5,0.5,1,0)", ShaderParams::get().FLOAT4);
	i_tess_displacement->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);

	i_glow->default_value = shader_parameter("float4(0.0,0.0,0,0)", ShaderParams::get().FLOAT4);

	// 1.0 opacity = fully opaque; 1.0 IOR = no refraction (air).
	i_opacity->default_value = shader_parameter("1.0", ShaderParams::get().FLOAT1);
	i_refraction->default_value = shader_parameter("1.0", ShaderParams::get().FLOAT1);
	set_translucent_defaults();
}

void MaterialGraph::set_translucent_defaults()
{
	// Thin-walled, fully transmissive, absorbing to albedo over 1 unit.
	i_thickness->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_transmission->default_value = shader_parameter("1.0", ShaderParams::get().FLOAT1);
	i_absorption_distance->default_value = shader_parameter("1.0", ShaderParams::get().FLOAT1);
}

MaterialGraph::~MaterialGraph()
{
}

FlowGraph::input::ptr MaterialGraph::get_texcoord()
{
	return texcoord;
}

FlowGraph::input::ptr MaterialGraph::get_position()
{
	return position;
}

FlowGraph::output::ptr MaterialGraph::get_base_color()
{
	return i_base_color;
}

FlowGraph::output::ptr MaterialGraph::get_mettalic()
{
	return i_metallic;
}

FlowGraph::output::ptr MaterialGraph::get_roughness()
{
	return i_roughness;
}


FlowGraph::output::ptr MaterialGraph::get_normals()
{
	return i_normal;
}


FlowGraph::output::ptr MaterialGraph::get_glow()
{
	return i_glow;
}



FlowGraph::output::ptr MaterialGraph::get_tess_displacement()
{
	return i_tess_displacement;
}

FlowGraph::output::ptr MaterialGraph::get_opacity()
{
	return i_opacity;
}

FlowGraph::output::ptr MaterialGraph::get_ior()
{
	return i_refraction;
}

FlowGraph::output::ptr MaterialGraph::get_thickness()
{
	return i_thickness;
}

FlowGraph::output::ptr MaterialGraph::get_transmission_weight()
{
	return i_transmission;
}

FlowGraph::output::ptr MaterialGraph::get_transmission_color()
{
	return i_transmission_color;
}

FlowGraph::output::ptr MaterialGraph::get_transmission_depth()
{
	return i_absorption_distance;
}

FlowGraph::output::ptr MaterialGraph::get_opacity_threshold()
{
	return i_opacity_threshold;
}

FlowGraph::output::ptr MaterialGraph::get_thin_film_thickness()
{
	return i_thin_film_thickness;
}

FlowGraph::output::ptr MaterialGraph::get_thin_film_ior()
{
	return i_thin_film_ior;
}

FlowGraph::output::ptr MaterialGraph::get_coat()
{
	return i_coat;
}

FlowGraph::output::ptr MaterialGraph::get_coat_roughness()
{
	return i_coat_roughness;
}

FlowGraph::output::ptr MaterialGraph::get_sheen()
{
	return i_sheen;
}

FlowGraph::output::ptr MaterialGraph::get_sheen_roughness()
{
	return i_sheen_roughness;
}

FlowGraph::output::ptr MaterialGraph::get_anisotropy()
{
	return i_anisotropy;
}

FlowGraph::output::ptr MaterialGraph::get_anisotropy_rotation()
{
	return i_anisotropy_rotation;
}

FlowGraph::output::ptr MaterialGraph::get_subsurface_weight()
{
	return i_sss_weight;
}

FlowGraph::output::ptr MaterialGraph::get_subsurface_radius()
{
	return i_sss_radius;
}

FlowGraph::output::ptr MaterialGraph::get_subsurface_scale()
{
	return i_sss_scale;
}

FlowGraph::output::ptr MaterialGraph::get_sheen_tint()
{
	return i_sheen_tint;
}

void MaterialGraph::set_coat_outputs_enabled(bool enabled)
{
	i_coat->set_enabled(enabled);
	i_coat_roughness->set_enabled(enabled);
	i_sheen->set_enabled(enabled);
	i_sheen_roughness->set_enabled(enabled);
	i_anisotropy->set_enabled(enabled);
	i_anisotropy_rotation->set_enabled(enabled);
	i_sss_weight->set_enabled(enabled);
	i_sss_radius->set_enabled(enabled);
	i_sss_scale->set_enabled(enabled);
	i_sheen_tint->set_enabled(enabled);
}

// Ports are serialized with the graph, so a graph saved before the coat
// outputs existed comes back without them. They go last in the COMPILED_FUNC
// output list, so appending reproduces the order a new graph has.
void MaterialGraph::restore_coat_outputs()
{
	// The constructor's own ports were dropped when the saved port list was
	// loaded over them, so the members are stale until looked up again.
	i_coat.reset();
	i_coat_roughness.reset();
	i_sheen.reset();
	i_sheen_roughness.reset();
	i_anisotropy.reset();
	i_anisotropy_rotation.reset();
	i_sss_weight.reset();
	i_sss_radius.reset();
	i_sss_scale.reset();
	i_sheen_tint.reset();
	i_transmission_color.reset();
	i_opacity_threshold.reset();
	i_thin_film_thickness.reset();
	i_thin_film_ior.reset();

	for (auto& out : output_parametres)
	{
		// Transparency ports were renamed to MaterialX's names.
		if (out->name == "refraction")
			out->name = "ior";
		else if (out->name == "transmission")
			out->name = "transmission weight";
		else if (out->name == "absorption distance")
			out->name = "transmission depth";

		if (out->name == "coat weight")
			i_coat = out;
		else if (out->name == "coat roughness")
			i_coat_roughness = out;
		else if (out->name == "sheen weight")
			i_sheen = out;
		else if (out->name == "sheen roughness")
			i_sheen_roughness = out;
		else if (out->name == "anisotropy")
			i_anisotropy = out;
		else if (out->name == "anisotropy rotation")
			i_anisotropy_rotation = out;
		else if (out->name == "subsurface weight")
			i_sss_weight = out;
		else if (out->name == "subsurface radius")
			i_sss_radius = out;
		else if (out->name == "subsurface scale")
			i_sss_scale = out;
		else if (out->name == "sheen tint")
			i_sheen_tint = out;
		else if (out->name == "transmission color")
			i_transmission_color = out;
		else if (out->name == "opacity threshold")
			i_opacity_threshold = out;
		else if (out->name == "thin film thickness")
			i_thin_film_thickness = out;
		else if (out->name == "thin film ior")
			i_thin_film_ior = out;
	}

	if (!i_coat)
		i_coat = register_output("coat weight", ShaderParams::get().FLOAT1);

	if (!i_coat_roughness)
		i_coat_roughness = register_output("coat roughness", ShaderParams::get().FLOAT1);

	if (!i_sheen)
		i_sheen = register_output("sheen weight", ShaderParams::get().FLOAT1);

	if (!i_sheen_roughness)
		i_sheen_roughness = register_output("sheen roughness", ShaderParams::get().FLOAT1);

	if (!i_anisotropy)
		i_anisotropy = register_output("anisotropy", ShaderParams::get().FLOAT1);

	if (!i_anisotropy_rotation)
		i_anisotropy_rotation = register_output("anisotropy rotation", ShaderParams::get().FLOAT1);

	if (!i_sss_weight)
		i_sss_weight = register_output("subsurface weight", ShaderParams::get().FLOAT1);

	if (!i_sss_radius)
		i_sss_radius = register_output("subsurface radius", ShaderParams::get().FLOAT3);

	if (!i_sss_scale)
		i_sss_scale = register_output("subsurface scale", ShaderParams::get().FLOAT1);

	if (!i_sheen_tint)
		i_sheen_tint = register_output("sheen tint", ShaderParams::get().FLOAT1);

	if (!i_transmission_color)
		i_transmission_color = register_output("transmission color", ShaderParams::get().FLOAT3);

	if (!i_opacity_threshold)
		i_opacity_threshold = register_output("opacity threshold", ShaderParams::get().FLOAT1);

	if (!i_thin_film_thickness)
		i_thin_film_thickness = register_output("thin film thickness", ShaderParams::get().FLOAT1);

	if (!i_thin_film_ior)
		i_thin_film_ior = register_output("thin film ior", ShaderParams::get().FLOAT1);

	i_coat->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_coat_roughness->default_value = shader_parameter("0.1", ShaderParams::get().FLOAT1);
	i_sheen->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_sheen_roughness->default_value = shader_parameter("0.3", ShaderParams::get().FLOAT1);
	i_anisotropy->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_anisotropy_rotation->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_sss_weight->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_sss_radius->default_value = shader_parameter("float3(1,1,1)", ShaderParams::get().FLOAT3);
	i_sss_scale->default_value = shader_parameter("1.0", ShaderParams::get().FLOAT1);
	i_sheen_tint->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_transmission_color->default_value = shader_parameter("float3(-1,-1,-1)", ShaderParams::get().FLOAT3);
	i_opacity_threshold->default_value = shader_parameter("0.5", ShaderParams::get().FLOAT1);
	i_thin_film_thickness->default_value = shader_parameter("0.0", ShaderParams::get().FLOAT1);
	i_thin_film_ior->default_value = shader_parameter("1.5", ShaderParams::get().FLOAT1);
}

// Toggled together with the coat outputs: both are "extra" outputs that every
// full (non-tessellation) variant of COMPILED_FUNC takes and the tessellation
// variant omits.
void MaterialGraph::set_translucent_outputs_enabled(bool enabled)
{
	set_coat_outputs_enabled(enabled);
	i_thickness->set_enabled(enabled);
	i_transmission->set_enabled(enabled);
	i_absorption_distance->set_enabled(enabled);
	i_transmission_color->set_enabled(enabled);
	i_opacity_threshold->set_enabled(enabled);
	i_thin_film_thickness->set_enabled(enabled);
	i_thin_film_ior->set_enabled(enabled);
}

void MaterialGraph::start(MaterialContext* context)
{
	this->context = context;
	reset_for();
	position->put(shader_parameter("pos", ShaderParams::get().FLOAT3));
	texcoord->put(shader_parameter("tc", ShaderParams::get().FLOAT2));

	for (auto n : nodes)
	{
		graph* g = dynamic_cast<graph*>(n.get());

		if (g)
			g->start(context);
	}

	(*this)(context);

	if (output_count != output_parametres.size())
		on_finish();
}


TextureAssetNode::TextureAssetNode(TextureAsset::ptr _Asset, bool to_linear)
{
	// Nodes created via the palette (FlowSystem::create) get their title
	// from the registered name automatically; nodes built directly in code
	// (mesh import, asset-explorer material building -- see
	// make_sampling_node) never go through that path, so set it here too.
	name = "TextureAssetNode";
	texture_info = std::make_shared<TextureSRVParams>(std::move(register_asset<Asset>(_Asset)),to_linear);

	o_texture = register_output("texture", ShaderParams::get().TEXTURE);
}

TextureAssetNode::TextureAssetNode()
{
	name = "TextureAssetNode";
}

TextureAssetNode::~TextureAssetNode()
{

}

void TextureAssetNode::operator()(MaterialContext* context)
{
	// Just the bindless texture index as a string -- SamplingNode is the one
	// that actually calls sample()/get_texture() with it. Not a real
	// computed shader value, so it never goes through
	// MaterialContext::capture_value (no live per-node preview to build from
	// this -- create_editor_window() always shows the raw asset instead).
	o_texture->put(shader_parameter(context->get_texture(texture_info), ShaderParams::get().TEXTURE));
}

SamplingNode::SamplingNode()
{
	name = "SamplingNode";
	i_tc = register_input("tc", ShaderParams::get().FLOAT2);
	i_texture = register_input("texture", ShaderParams::get().TEXTURE);
	o_vec4 = register_output("", ShaderParams::get().FLOAT4);
	o_r = register_output("", ShaderParams::get().FLOAT1);
	o_g = register_output("", ShaderParams::get().FLOAT1);
	o_b = register_output("", ShaderParams::get().FLOAT1);
	o_a = register_output("", ShaderParams::get().FLOAT1);
}

SamplingNode::~SamplingNode()
{
}

void SamplingNode::operator()(MaterialContext* context)
{
	auto mat_graph = static_cast<MaterialFunction*>(owner);

	// No upstream TextureAssetNode linked yet -- nothing to sample.
	if (!i_texture->has_input() && !i_texture->has_value())
		return;

	auto tex = i_texture->get<shader_parameter>();
	auto val = mat_graph->add_value(ShaderParams::get().FLOAT4, std::string("sample(") + tex.name + ",Sampler, " + i_tc->get<shader_parameter>().name + ".xy, lod)");
	o_vec4->put(shader_parameter("float4(" + val.name + ".xyz,1)", ShaderParams::get().FLOAT4));
	o_r->put(shader_parameter(val.name + ".r", ShaderParams::get().FLOAT1));
	o_g->put(shader_parameter(val.name + ".g", ShaderParams::get().FLOAT1));
	o_b->put(shader_parameter(val.name + ".b", ShaderParams::get().FLOAT1));
	o_a->put(shader_parameter(val.name + ".a", ShaderParams::get().FLOAT1));
}

SamplingNode::ptr make_sampling_node(MaterialGraph* graph, TextureAsset::ptr asset, bool to_linear)
{
	auto asset_node = std::make_shared<TextureAssetNode>(asset, to_linear);
	graph->register_node(asset_node);

	auto sample_node = std::make_shared<SamplingNode>();
	graph->register_node(sample_node);

	asset_node->get_output(0)->link(sample_node->get_input(1));
	graph->get_texcoord()->link(sample_node->get_input(0));

	return sample_node;
}

PowerNode::PowerNode()
{
	i_vec = register_input(/*ShaderParams::get().VECTOR, */"value", ShaderParams::get().VECTOR);
	i_power = register_input(/*ShaderParams::get().VECTOR,*/ "power", ShaderParams::get().VECTOR);
	o_value = register_output(/*ShaderParams::get().VECTOR,*/ "result", ShaderParams::get().VECTOR);
}

void PowerNode::operator()(MaterialContext*)
{
	auto mat_graph = static_cast<MaterialFunction*>(owner);
	auto val = i_vec->get<shader_parameter>();
	auto pow = i_power->get<shader_parameter>();
	auto res = mat_graph->add_value(val.type, "pow(" + val.name + "," + pow.name + ")");
	o_value->put(res);
	//	mat_graph
}

VectorNode::VectorNode(vec4 value)
{
	// See TextureAssetNode::TextureAssetNode -- callers like
	// AssimpLoader/StencilRenderer/main.cpp build these directly (default
	// material color/roughness/metallic), bypassing the palette factory
	// that would otherwise set the title.
	name = "VectorNode";
	uniform.reset(new Uniform);
	uniform->value.f4_value = value;
	uniform->name = "some_val";
	uniform->type = ShaderParams::get().FLOAT4;
	o_value = register_output(/*ShaderParams::get().VECTOR,*/ "", ShaderParams::get().VECTOR);
}

VectorNode::VectorNode()
{
	name = "VectorNode";
}

void VectorNode::operator()(MaterialContext* c)
{
	auto res = c->create_value(uniform);
	o_value->put(res);
}

GUI::base::ptr MaterialGraph::create_editor_window()
{
	// Live material preview on the graph-output node (widget built by the app).
	if (create_preview_hook && preview_material)
		return create_preview_hook(preview_material->get_ptr<Asset>());
	return nullptr;
}

GUI::base::ptr VectorNode::create_editor_window()
{
	auto picker = std::make_shared<GUI::Elements::color_picker>();
	picker->on_change.register_handler(nullptr, [this](float4 color) {
		// color.x/y/z/w are R/G/B/A in [0,1]
		uniform->value.f4_value = color;
		uniform->on_change(uniform.get());
		});
	picker->set_color(uniform->value.f4_value); // start red


	return picker;
}

ScalarNode::ScalarNode(float value)
{
	name = "ScalarNode";
	uniform.reset(new Uniform);
	uniform->value.f_value = value;
	uniform->name = "some_val";
	uniform->type = ShaderParams::get().FLOAT1;
	o_value = register_output(/*ShaderParams::get().FLOAT1,*/ "", ShaderParams::get().FLOAT1);
}

void ScalarNode::operator()(MaterialContext* c)
{
	auto mat_graph = static_cast<MaterialFunction*>(owner);
	auto res = c->create_value(uniform);
	o_value->put(res);
	//	mat_graph
}

GUI::base::ptr ScalarNode::create_editor_window()
{
	GUI::Elements::value_box::ptr box(new GUI::Elements::value_box);
	box->docking = GUI::dock::TOP;

	// Values are stored as current_value/100, so max_value=100 capped scalars at
	// 1.0. Widen the range so values > 1 (e.g. a refraction index) are editable,
	// and seed current_value from the uniform so the box starts at its real value.
	box->max_value     = 1000; // 0.00 .. 10.00 in 0.01 steps
	box->current_value = int(uniform->value.f_value * 100 + 0.5f);

	box->on_change = [this](int value, GUI::Elements::value_box* b)
	{
		uniform->value.f_value = float(value) / 100;
		b->info->text = std::to_string(uniform->value.f_value).substr(0, 4);
		uniform->on_change(uniform.get());
	};

box->info->text = std::to_string(uniform->value.f_value).substr(0, 4);
	return box;
}

SumNode::SumNode()
{
	i_vec = register_input(/*ShaderParams::get().VECTOR,*/ "a", ShaderParams::get().VECTOR);
	i_power = register_input(/*ShaderParams::get().VECTOR,*/ "b", ShaderParams::get().VECTOR);
	o_value = register_output(/*ShaderParams::get().VECTOR,*/ "result", ShaderParams::get().VECTOR);
}

void SumNode::operator()(MaterialContext*)
{
	auto mat_graph = static_cast<MaterialFunction*>(owner);
	auto val = i_vec->get<shader_parameter>();
	auto pow = i_power->get<shader_parameter>();
	auto res = mat_graph->add_value(val.type, "(" + val.name + "+" + pow.name + ")");
	o_value->put(res);
}

MulNode::MulNode()
{
	name = "MulNode";
	i_vec = register_input(/*ShaderParams::get().VECTOR,*/ "a", ShaderParams::get().VECTOR);
	i_power = register_input(/*ShaderParams::get().VECTOR,*/ "b", ShaderParams::get().VECTOR);
	o_value = register_output(/*ShaderParams::get().VECTOR, */"result", ShaderParams::get().VECTOR);
}

void MulNode::operator()(MaterialContext* c)
{
	auto mat_graph = static_cast<MaterialFunction*>(owner);
	auto val = i_vec->get<shader_parameter>();
	auto pow = i_power->get<shader_parameter>();
	auto res = mat_graph->add_value(val.type, "(" + val.name + "*" + pow.name + ")");
	/* Uniform v;
	 v.name = "some_val";
	 v.def_value = 5.0f;
	 v.type = ShaderParams::get().FLOAT1;
	 auto res = c->create_value(v);*/
	o_value->put(res);
}

ReconstructNormalZNode::ReconstructNormalZNode()
{
	name = "ReconstructNormalZNode";
	i_r = register_input("r", ShaderParams::get().FLOAT1);
	i_g = register_input("g", ShaderParams::get().FLOAT1);
	o_normal = register_output("normal", ShaderParams::get().FLOAT4);
}

void ReconstructNormalZNode::operator()(MaterialContext* c)
{
	auto mat_graph = static_cast<MaterialFunction*>(owner);
	auto r = i_r->get<shader_parameter>();
	auto g = i_g->get<shader_parameter>();

	auto x = mat_graph->add_value(ShaderParams::get().FLOAT1, r.name + " * 2 - 1");
	auto y = mat_graph->add_value(ShaderParams::get().FLOAT1, g.name + " * 2 - 1");
	auto z = mat_graph->add_value(ShaderParams::get().FLOAT1,
		"sqrt(saturate(1 - " + x.name + "*" + x.name + " - " + y.name + "*" + y.name + "))");
	auto packed = mat_graph->add_value(ShaderParams::get().FLOAT4,
		"float4(float3(" + x.name + "," + y.name + "," + z.name + ") * 0.5 + 0.5, 1)");

	o_normal->put(packed);
}
/*
TiledTextureNode::TiledTextureNode(TiledTexture::ptr _Asset) : asset(this)
{
	asset = register_asset(_Asset);
	i_tc = register_input("tc", ShaderParams::get().FLOAT2);
	o_vec4 = register_output("vec4", ShaderParams::get().FLOAT4);
	o_r = register_output( "r", ShaderParams::get().FLOAT1);
	o_g = register_output("g", ShaderParams::get().FLOAT1);
	o_b = register_output( "b"), ShaderParams::get().FLOAT1;
	o_a = register_output("a", ShaderParams::get().FLOAT1);
	++::FlowGraph::counter;
}
*/

TiledTextureNode::TiledTextureNode()/* : asset(this)*/
{
}

TiledTextureNode::~TiledTextureNode()
{
}

void TiledTextureNode::operator()(MaterialContext* context)
{
	auto mat_graph = static_cast<MaterialFunction*>(owner);
	//  auto val = mat_graph->add_value(ShaderParams::get().FLOAT4, "float4(1,0,0,1)");


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*	auto val = context->get_texture(*asset, i_tc->get<shader_parameter>());
	o_vec4->put(val);
	o_r->put(shader_parameter(val.name + ".r", ShaderParams::get().FLOAT1));
	o_g->put(shader_parameter(val.name + ".g", ShaderParams::get().FLOAT1));
	o_b->put(shader_parameter(val.name + ".b", ShaderParams::get().FLOAT1));
	o_a->put(shader_parameter(val.name + ".a", ShaderParams::get().FLOAT1));*/
	//	mat_graph
}


// gui part
using namespace GUI;
using namespace Elements;


GUI::base::ptr TextureAssetNode::create_editor_window()
{
	GUI::Elements::image::ptr img(new GUI::Elements::image);
	img->texture = Skin::get().Shadow;

	img->padding = img->texture.padding;
	img->width_size = GUI::size_type::MATCH_CHILDREN;
	img->height_size = GUI::size_type::MATCH_CHILDREN;
	img->docking = dock::TOP;

	auto asset = (texture_info->asset)->get_ptr<TextureAsset>();

	// The raw asset texture -- this node has no computed shader "value" of
	// its own (no live per-node preview to show instead, see operator()).
	GUI::Elements::image::ptr img_inner(new GUI::Elements::image);
	img_inner->texture = asset->get_texture()->texture_2d();
	img_inner->size = { 64, 64 };
	img->add_child(img_inner);



	auto back = std::make_shared<base>();
	back->width_size = size_type::MATCH_CHILDREN;
	back->height_size = size_type::MATCH_CHILDREN;


	back->add_child(img);





	auto chk_srgb = std::make_shared<check_box_text>();

	chk_srgb->docking = dock::TOP;

	chk_srgb->get_label()->text = "linear";

	chk_srgb->get_check()->set_checked(texture_info->to_linear);
	chk_srgb->on_check = [this, asset](bool v) {

		texture_info->to_linear = v;
		//	asset->get_texture()->texture_2d().srv(texture_info->to_linear ? PixelSpace::MAKE_LINERAR : PixelSpace::MAKE_SRGB)(img_inner->texture.srv[0]);
			//	owner->co
	};
	back->add_child(chk_srgb);

	return back;
}

// M=1 is a float scalar/vector, M=3/4 are INT/BOOL; M=2 (TEXTURE) is not
// numeric and never converts.
static bool is_numeric_type(const ShaderParamType& t)
{
	return t.M == 1 || t.M == 3 || t.M == 4;
}

bool ShaderParamType::can_cast(parameter_type* other)
{
	VectorType* vtype = dynamic_cast<VectorType*>(other);

	if (vtype) return true;
	ShaderParamType* type = dynamic_cast<ShaderParamType*>(other);

	if (!type) return false;

	if (N == -1 || type->N == -1)
		return M == type->M;

	if (M == type->M && N == type->N)
		return true;

	// Any numeric type links to any other; the width/kind mismatch is
	// resolved by coerce() where the value is consumed (MaterialXNode inputs
	// and MaterialFunction's output assignment).
	return is_numeric_type(*this) && is_numeric_type(*type);
}

std::string coerce(const shader_parameter& src, const ShaderParamType& dst)
{
	const auto& s = src.type;

	if (s.M == dst.M && s.N == dst.N)
		return src.name;

	if (!is_numeric_type(s) || !is_numeric_type(dst))
		return src.name;

	const std::string& n = src.name;
	const bool src_scalar = s.M != 1 || s.N == 1;

	// Scalar destinations (float, int, bool): take .x of a vector, then cast.
	if (dst.M != 1 || dst.N == 1)
	{
		std::string x = (s.M == 1 && s.N > 1) ? "(" + n + ").x" : n;
		return "((" + dst.name + ")(" + x + "))";
	}

	// Float vector destinations.
	if (src_scalar)
		return "((" + dst.name + ")(" + (s.M == 1 ? n : "(float)(" + n + ")") + "))";

	if (s.N > dst.N)
	{
		static const char* swizzle[] = { "", ".x", ".xy", ".xyz", ".xyzw" };
		return "(" + n + ")" + swizzle[dst.N];
	}

	// s.N < dst.N, both vectors (2->3, 2->4, 3->4): pad with 0, w gets 1.
	std::string pad;
	for (int i = s.N; i < dst.N; i++)
		pad += (i == 3 ? ", 1" : ", 0");

	return dst.name + "(" + n + pad + ")";
}
REGISTER_TYPE(MaterialContext);

bool ShaderParamType::operator==(const ShaderParamType& t) const
{
    return M == t.M && N == t.N;
}

int ShaderParamType::get_size()
{
    return M * N * sizeof(float);
}

bool VectorType::can_cast(parameter_type* other)
{
    VectorType* vtype = dynamic_cast<VectorType*>(other);

    if (vtype) return true;

    ShaderParamType* type = dynamic_cast<ShaderParamType*>(other);

    if (type) return type->M == 1;

    return false;
}

Uniform::_value::_value()
{
    raw_data[0] = 0;
    raw_data[1] = 0;
    raw_data[2] = 0;
    raw_data[3] = 0;
}

Uniform::Uniform()
{
    name = "unknown";
}

TextureSRVParams::TextureSRVParams() : asset(nullptr), to_linear(false)
{
}

TextureSRVParams::TextureSRVParams(Asset::ref&& asset, bool to_linear)
    : asset(std::move(asset)), to_linear(to_linear)
{
}

SpecToMetNode::SpecToMetNode()
{
    // Explicit types are required here, not cosmetic: register_input/
    // register_output default to strict_parameter when omitted, and
    // strict_parameter::can_cast() is permissive as a *source* type (any
    // destination accepts it) but ShaderParamType::can_cast() -- which runs
    // when THIS node's outputs are the source being linked INTO a strictly
    // typed destination like MaterialGraph's base_color/metallic -- only
    // recognizes VectorType/ShaderParamType sources, so an untyped
    // strict_parameter output there silently fails can_link() (link()
    // returns false, no exception, no connection made).
    inputs.albedo   = register_input("albedo", ShaderParams::get().VECTOR);
    inputs.specular = register_input("specular", ShaderParams::get().VECTOR);

    outputs.albedo   = register_output("albedo", ShaderParams::get().FLOAT4);
    outputs.metallic = register_output("metallic", ShaderParams::get().FLOAT1);
}

void SpecToMetNode::operator()(MaterialContext* c)
{
    auto mat_graph = static_cast<MaterialFunction*>(owner);

    auto res1 = mat_graph->add_value(ShaderParams::get().FLOAT4);
    auto res2 = mat_graph->add_value(ShaderParams::get().FLOAT1);

    auto val1 = inputs.albedo->get<shader_parameter>();
    auto val2 = inputs.specular->get<shader_parameter>();

    mat_graph->add_function(std::string("spec_to_metallic(") + val1.name + "," + val2.name + "," + res1.name + "," + res2.name + ")");

    outputs.albedo->put(res1);
    outputs.metallic->put(res2);
}