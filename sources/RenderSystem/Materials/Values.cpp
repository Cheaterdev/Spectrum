#include "pch.h"

/*
int get_size(FlowGraph::data_types v)
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
				if (type == ShaderParams::FLOAT1)
					name = other.name + ".x";

				if (type == ShaderParams::FLOAT2)
					name = other.name + ".xy";

				if (type == ShaderParams::FLOAT3)
					name = other.name + ".xyz";

				if (type == ShaderParams::FLOAT4)
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

	return graph->add_value(f->type, std::string("GetMaterialInfo().GetData().") + shader_name);
}

std::string MaterialContext::get_texture(TextureSRVParams::ptr& p)
{
	auto it = shader_parameter_srv.find(p);
	if (it != shader_parameter_srv.end())
	{

		return it->second;
	}

	textures.push_back(p);
	auto res = std::string("get_texture(") + std::to_string(texture_counter++) + std::string(")");
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
	auto val = graph->add_value(ShaderParams::FLOAT4, str);
	return val;
	//textures.push_back(asset);
	//return std::string("textures[") + std::to_string(textures.size() - 1) + std::string("]");
}


std::string MaterialContext::generate_uniform_struct()
{
	if (uniform_struct.empty())
		return "\n";
	std::string result = "struct MaterialCB\n{\n";
	result += uniform_struct;

	for (auto u : uniforms)
	{
		/*  result += u->type.to_string();
		  result += " ";
		  result += u->shader_name;
		  result += ";\n";*/
	}

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

	this->graph = graph;
	uniform_struct = "";
	clear_parameters();
	text = "";//file->get_data();
	// std::string universal = file->get_data();
	functions.clear();

	params = 0;
	graph->get_normals()->set_enabled(true);
	graph->get_texcoord()->set_enabled(true);
	graph->get_mettalic()->set_enabled(true);
	graph->get_base_color()->set_enabled(true);
	graph->get_roughness()->set_enabled(true);

	graph->get_tess_displacement()->set_enabled(false);
	graph->start(this);



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
		graph->get_texcoord()->set_enabled(false);
		graph->get_mettalic()->set_enabled(false);
		graph->get_roughness()->set_enabled(false);

		graph->get_base_color()->set_enabled(false);
		graph->get_tess_displacement()->set_enabled(true);
		graph->start(this);

		if (functions.find(graph) != functions.end())
		{
			tess_shader.macros.emplace_back("COMPILED_FUNC", graph->func_name);
		}
	
		tess_shader.text = text;

	}
	std::string uniforms_string = generate_uniform_struct();

	pixel_shader.uniforms = uniforms_string;
	voxel_shader.uniforms = uniforms_string;
	hit_shader.uniforms = uniforms_string;
	tess_shader.uniforms = uniforms_string;

	uniforms_ps = uniforms;
	text = pixel_shader.text;
}

ShaderSource MaterialContext::get_tess_result()
{
	return tess_shader;
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
		TextureNode::ptr node(new TextureNode(item->asset->get_asset()->get_ptr<TextureAsset>()));
		register_node(node);
		result.push_back(node);
	}

	if (item->asset->get_type() == Asset_Type::TILED_TEXTURE)
	{
		TiledTextureNode::ptr node(new TiledTextureNode(item->asset->get_asset()->get_ptr<TiledTexture>()));
		register_node(node);
		result.push_back(node);
	}


	return result;

}



void MaterialFunction::on_finish()
{
	int o = 0;

	for (unsigned int i = 0; i < output_parametres.size(); i++)
	{
		if (!output_parametres[i]->enabled) continue;

		add_function(f_output_names[o++] + " = " + output_parametres[i]->get<shader_parameter>().name);
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
				o_r->put(shader_parameter(val.name + ".r", ShaderParams::FLOAT1));
				o_g->put(shader_parameter(val.name + ".g", ShaderParams::FLOAT1));
				o_b->put(shader_parameter(val.name + ".b", ShaderParams::FLOAT1));
				o_a->put(shader_parameter(val.name + ".a", ShaderParams::FLOAT1));
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
	text += "void " + func_name + "( " + input_struct + delim + output_struct + " )\n{\n";
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
	return result;
}

void MaterialFunction::add_function(std::string s)
{
	text += std::string("\t") + s + ";\n";
}
BOOST_CLASS_EXPORT_IMPLEMENT(ScalarNode);

BOOST_CLASS_EXPORT_IMPLEMENT(MulNode);
BOOST_CLASS_EXPORT_IMPLEMENT(SumNode);
BOOST_CLASS_EXPORT_IMPLEMENT(TextureNode);
BOOST_CLASS_EXPORT_IMPLEMENT(VectorNode);
BOOST_CLASS_EXPORT_IMPLEMENT(TiledTextureNode);
BOOST_CLASS_EXPORT_IMPLEMENT(MaterialGraph);
BOOST_CLASS_EXPORT_IMPLEMENT(SpecToMetNode);
BOOST_CLASS_EXPORT_IMPLEMENT(TextureSRVParams);


BOOST_CLASS_EXPORT(ShaderParamType);


MaterialGraph::MaterialGraph()
{
	position = register_input( "position",ShaderParams::FLOAT3);
	texcoord = register_input(/*ShaderParams::FLOAT2,*/ "texcoord", ShaderParams::FLOAT2);
	i_base_color = register_output(/*ShaderParams::FLOAT4,*/ "base color", ShaderParams::FLOAT4);
	i_metallic = register_output(/*ShaderParams::FLOAT1,*/ "metallic", ShaderParams::FLOAT1);
	i_roughness = register_output(/*ShaderParams::FLOAT1, */"roughness", ShaderParams::FLOAT1);
	i_normal = register_output(/*ShaderParams::FLOAT4, */"normal", ShaderParams::FLOAT4);
	i_tess_displacement = register_output(/*ShaderParams::FLOAT1,*/ "displacement", ShaderParams::FLOAT1);
	i_base_color->default_value = shader_parameter("float4(0,0,0,1)", ShaderParams::FLOAT4);
	i_metallic->default_value = shader_parameter("0.0", ShaderParams::FLOAT1);
	i_roughness->default_value = shader_parameter("0.0", ShaderParams::FLOAT1);

	i_normal->default_value = shader_parameter("float4(0.5,0.5,1,0)", ShaderParams::FLOAT4);
	i_tess_displacement->default_value = shader_parameter("0.0", ShaderParams::FLOAT1);
}

MaterialGraph::~MaterialGraph()
{
}

FlowGraph::input::ptr MaterialGraph::get_texcoord()
{
	return texcoord;
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

FlowGraph::output::ptr MaterialGraph::get_tess_displacement()
{
	return i_tess_displacement;
}

void MaterialGraph::start(MaterialContext* context)
{
	this->context = context;
	reset_for();
	position->put(shader_parameter("pos", ShaderParams::FLOAT3));
	texcoord->put(shader_parameter("tc", ShaderParams::FLOAT2));

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


TextureNode::TextureNode(TextureAsset::ptr _Asset, bool to_linear)
{
	texture_info = std::make_shared<TextureSRVParams>();
	texture_info->asset = register_asset<Asset>(_Asset);
	texture_info->to_linear = to_linear;

	i_tc = register_input(/*ShaderParams::FLOAT2,*/ "tc", ShaderParams::FLOAT2);
	o_vec4 = register_output(/*ShaderParams::FLOAT4,*/ "", ShaderParams::FLOAT4);
	o_r = register_output(/*ShaderParams::FLOAT1,*/ "", ShaderParams::FLOAT1);
	o_g = register_output(/*ShaderParams::FLOAT1,*/ "", ShaderParams::FLOAT1);
	o_b = register_output(/*ShaderParams::FLOAT1,*/ "", ShaderParams::FLOAT1);
	o_a = register_output(/*ShaderParams::FLOAT1,*/ "", ShaderParams::FLOAT1);
	++::FlowGraph::counter;
}

TextureNode::TextureNode()
{
}

TextureNode::~TextureNode()
{
	--::FlowGraph::counter;

	if (::FlowGraph::counter <= 0)
		::FlowGraph::counter = 0;
}

void TextureNode::operator()(MaterialContext* context)
{
	auto mat_graph = static_cast<MaterialFunction*>(owner);
	//auto val = mat_graph->add_value(ShaderParams::FLOAT4, "float4(1,0,0,1)");
	auto val = mat_graph->add_value(ShaderParams::FLOAT4, std::string("sample(") + context->get_texture(texture_info) + ",Sampler, " + i_tc->get<shader_parameter>().name + ".xy)");
	o_vec4->put(shader_parameter("float4(" + val.name + ".xyz,1)", ShaderParams::FLOAT4));
	o_r->put(shader_parameter(val.name + ".r", ShaderParams::FLOAT1));
	o_g->put(shader_parameter(val.name + ".g", ShaderParams::FLOAT1));
	o_b->put(shader_parameter(val.name + ".b", ShaderParams::FLOAT1));
	o_a->put(shader_parameter(val.name + ".a", ShaderParams::FLOAT1));
	//	mat_graph
}

PowerNode::PowerNode()
{
	i_vec = register_input(/*ShaderParams::VECTOR, */"value", ShaderParams::VECTOR);
	i_power = register_input(/*ShaderParams::VECTOR,*/ "power", ShaderParams::VECTOR);
	o_value = register_output(/*ShaderParams::VECTOR,*/ "result", ShaderParams::VECTOR);
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
	uniform.reset(new Uniform);
	uniform->value.f4_value = value;
	uniform->name = "some_val";
	uniform->type = ShaderParams::FLOAT4;
	o_value = register_output(/*ShaderParams::VECTOR,*/ "", ShaderParams::VECTOR);
}

VectorNode::VectorNode()
{
}

void VectorNode::operator()(MaterialContext* c)
{
	auto res = c->create_value(uniform);
	o_value->put(res);
}

GUI::base::ptr VectorNode::create_editor_window()
{
	GUI::Elements::colored_rect::ptr img(new GUI::Elements::colored_rect);
	img->color = uniform->value.f4_value;
	img->size = { 64, 64 };


	return img;
}

ScalarNode::ScalarNode(float value)
{
	uniform.reset(new Uniform);
	uniform->value.f_value = value;
	uniform->name = "some_val";
	uniform->type = ShaderParams::FLOAT1;
	o_value = register_output(/*ShaderParams::FLOAT1,*/ "", ShaderParams::FLOAT1);
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
	// img->text = std::to_string(value);
	// img->size = { 64, 64 };
	box->on_change = [this](int value, GUI::Elements::value_box* b)
	{
		uniform->value.f_value = float(value) / 100;
		b->info->text = std::to_string(uniform->value.f_value).substr(0, 4);
		uniform->on_change(uniform.get());
	};
	return box;
}

SumNode::SumNode()
{
	i_vec = register_input(/*ShaderParams::VECTOR,*/ "a", ShaderParams::VECTOR);
	i_power = register_input(/*ShaderParams::VECTOR,*/ "b", ShaderParams::VECTOR);
	o_value = register_output(/*ShaderParams::VECTOR,*/ "result", ShaderParams::VECTOR);
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
	i_vec = register_input(/*ShaderParams::VECTOR,*/ "a", ShaderParams::VECTOR);
	i_power = register_input(/*ShaderParams::VECTOR,*/ "b", ShaderParams::VECTOR);
	o_value = register_output(/*ShaderParams::VECTOR, */"result", ShaderParams::VECTOR);
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
	 v.type = ShaderParams::FLOAT1;
	 auto res = c->create_value(v);*/
	o_value->put(res);
}

TiledTextureNode::TiledTextureNode(TiledTexture::ptr _Asset) : asset(this)
{
	asset = register_asset(_Asset);
	i_tc = register_input(/*ShaderParams::FLOAT2,*/ "tc", ShaderParams::FLOAT2);
	o_vec4 = register_output(/*ShaderParams::FLOAT4, */"vec4", ShaderParams::FLOAT4);
	o_r = register_output(/*ShaderParams::FLOAT1,*/ "r", ShaderParams::FLOAT1);
	o_g = register_output(/*ShaderParams::FLOAT1, */"g", ShaderParams::FLOAT1);
	o_b = register_output(/*ShaderParams::FLOAT1,*/ "b"), ShaderParams::FLOAT1;
	o_a = register_output(/*ShaderParams::FLOAT1, */"a", ShaderParams::FLOAT1);
	++::FlowGraph::counter;
}

TiledTextureNode::TiledTextureNode() : asset(this)
{
}

TiledTextureNode::~TiledTextureNode()
{
	--::FlowGraph::counter;

	if (::FlowGraph::counter <= 0)
		::FlowGraph::counter = 0;
}

void TiledTextureNode::operator()(MaterialContext* context)
{
	auto mat_graph = static_cast<MaterialFunction*>(owner);
	//  auto val = mat_graph->add_value(ShaderParams::FLOAT4, "float4(1,0,0,1)");


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*	auto val = context->get_texture(*asset, i_tc->get<shader_parameter>());
	o_vec4->put(val);
	o_r->put(shader_parameter(val.name + ".r", ShaderParams::FLOAT1));
	o_g->put(shader_parameter(val.name + ".g", ShaderParams::FLOAT1));
	o_b->put(shader_parameter(val.name + ".b", ShaderParams::FLOAT1));
	o_a->put(shader_parameter(val.name + ".a", ShaderParams::FLOAT1));*/
	//	mat_graph
}


// gui part
using namespace GUI;
using namespace Elements;


GUI::base::ptr TextureNode::create_editor_window()
{
	GUI::Elements::image::ptr img(new GUI::Elements::image);
	img->texture.texture = Render::Texture::get_resource({ "textures/gui/shadow.png", false, false });
	img->texture.padding = { 9, 9, 9, 9 };
	img->padding = { 9, 9, 9, 9 };
	img->width_size = GUI::size_type::MATCH_CHILDREN;
	img->height_size = GUI::size_type::MATCH_CHILDREN;
	img->docking = dock::TOP;
	//   img->size = { 64, 64 };
	GUI::Elements::image::ptr img_inner(new GUI::Elements::image);
	//img_inner->texture.texture = asset->get_texture();

	img_inner->texture.srv = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1);


	auto asset = (texture_info->asset)->get_ptr<TextureAsset>();
	asset->get_texture()->texture_2d()->srv(texture_info->to_linear ? PixelSpace::MAKE_LINERAR : PixelSpace::MAKE_SRGB)(img_inner->texture.srv[0]);

	//img_inner->docking = GUI::dock::FILL;
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
	chk_srgb->on_check = [this, img_inner, asset](bool v) {

		texture_info->to_linear = v;
		asset->get_texture()->texture_2d()->srv(texture_info->to_linear ? PixelSpace::MAKE_LINERAR : PixelSpace::MAKE_SRGB)(img_inner->texture.srv[0]);
		//	owner->co
	};
	back->add_child(chk_srgb);

	return back;
}

bool ShaderParamType::can_cast(parameter_type* other)
{
	VectorType* vtype = dynamic_cast<VectorType*>(other);

	if (vtype) return true;
	ShaderParamType* type = dynamic_cast<ShaderParamType*>(other);

	if (!type) return false;

	if (N == -1 || type->N == -1)
		return M == type->M;

	bool res = M == type->M && N == type->N;
	return res;
}
