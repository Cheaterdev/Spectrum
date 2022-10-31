﻿#include "pch_render.h"
#include "Assets/EngineAssets.h"
#include "universal_material.h"
#include "Effects/RTX/RTX.h"

static IdGenerator ids;

import HAL;

import crc32;

import Utils;
import Data;
import stl.core;
import stl.memory;

import Serializer;

import serialization;
import Log;
import Utils;
import crc32;

import Data;
import windows;
REGISTER_TYPE(materials::universal_material);
REGISTER_TYPE(materials::PipelinePasses);
REGISTER_TYPE(materials::PipelineSimple);


DynamicData generate_data(std::vector<Uniform::ptr>& un)
{
	DynamicData data;

	int offset = 0;

	for (auto& u : un)
	{
		int need_size = u->type.get_size();

		while (offset > 0 && (offset + need_size) > 16)
		{
			data.emplace_back(std::byte(0));
			offset = (offset + 1) % 16;
		}

		std::byte* ptr = nullptr;


		if (u->type == ShaderParams::FLOAT1)
		{
			ptr = reinterpret_cast<std::byte*>(&u->value.f_value);
		}

		if (u->type == ShaderParams::FLOAT2)
		{
			ptr = reinterpret_cast<std::byte*>(&u->value.f2_value);
		}

		if (u->type == ShaderParams::FLOAT3)
		{
			ptr = reinterpret_cast<std::byte*>(&u->value.f3_value);
		}

		if (u->type == ShaderParams::FLOAT4)
		{
			ptr = reinterpret_cast<std::byte*>(&u->value.f4_value);
		}

		data.insert(data.end(), ptr, ptr + need_size);


		offset = (offset + need_size) % 16;
	}

	return data;
}




void materials::universal_material::update()
{
	//std::lock_guard<std::mutex> g(m);
	PROFILE(L"universal_material");
	if (need_regenerate_material)
	{
		generate_material();
		need_regenerate_material = false;
	}

	if (need_update_uniforms)
	{
		auto generate = [this](std::vector<Uniform::ptr>& un)
		{
			pixel_data = generate_data(un);
			if (pixel_data.empty())
				return;
		};

		generate(ps_uniforms);

		need_update_compiled = true;
	}

	if (need_update_compiled)
	{
		material_info.GetTextures() = texture_srvs;// textures_handle ? (UINT)textures_handle.get_offset() : 0;
		material_info.GetData() = pixel_data;
		compiled_material_info = material_info.compile(StaticCompiledGPUData::get());
		local_addr = compiled_material_info.compiled();

		//local_addr_ids = to_native(compiled_material_info.offsets_cb);
		{
			auto elem = info_handle.map();// universal_material_info_part_manager::get().map_elements(info_handle.get_offset(), 1);
			elem[0].pipeline_id = pipeline->get_id();
			elem[0].material_cb = to_native(compiled_material_info.compiled());

			info_handle.write(0, elem);

		}

		update_rtx();
	}

	if (need_update_uniforms || need_update_compiled)
		mark_contents_changed();

	need_update_uniforms = false;
	need_update_compiled = false;


}

size_t  materials::universal_material::get_id()
{

	return  pipeline->get_id(); ///TODO: change for graph id
}


void materials::universal_material::compile()
{
	start_changing_contents();

	handlers.clear();

	texture_srvs.resize(textures.size());


	for (int i = 0; i < textures.size(); i++)
	{
		auto& t = textures[i];
		TextureAsset::ptr tex = t->asset->get_ptr<TextureAsset>();
		if (tex && tex->get_texture()->texture_2d())
			texture_srvs[i] = tex->get_texture()->texture_2d().texture2D;
		else
			texture_srvs[i] = EngineAssets::missing_texture.get_asset()->get_texture()->texture_2d().texture2D;
	}


	auto generate = [this](std::vector<Uniform::ptr>& un)
	{
		pixel_data = generate_data(un);

		for (auto u : un)
		{
			handlers.emplace_back();
			u->on_change.register_handler(&handlers.back(), [this](Uniform* u)
				{
					need_update_uniforms = true;
				});
		}
	};


	generate(ps_uniforms);
	material_info.GetTextures() = texture_srvs;// textures_handle ? (UINT)textures_handle.get_offset() : 0;
	material_info.GetData() = pixel_data;
	compiled_material_info = material_info.compile(StaticCompiledGPUData::get());

	local_addr = compiled_material_info.compiled();
	//local_addr_ids = to_native(compiled_material_info.offsets_cb);

	if (!info_handle)
	{
		info_handle = universal_material_info_part_manager::get().allocate(1);
	}

	auto elem = info_handle.map();
	elem[0].pipeline_id = pipeline->get_id();
	elem[0].material_cb = to_native(compiled_material_info.compiled());
	info_handle.write(0, elem);

	need_update_compiled = false;
	need_update_uniforms = false;

	RTX::get().rtx.init_material(this);
	end_changing_contents();
}

void materials::universal_material::on_graph_changed()
{
	need_regenerate_material = true;
}


void materials::universal_material::generate_texture_handles()
{

}

void materials::universal_material::generate_material()
{
	//   std::lock_guard<std::mutex> g(m);
	if (!context)
		context.reset(new MaterialContext);

	if (!info_handle)
	{
		info_handle = universal_material_info_part_manager::get().allocate(1);
	}



	context->start(include_file->get_data(), graph.get().get());


	auto ps_str = context->get_pixel_result().uniforms + include_file->get_data() + context->get_pixel_result().text;
	auto tess_orig_shader = context->get_tess_result().text;
	auto tess_str = tess_orig_shader.empty() ? std::string() : (context->get_tess_result().uniforms + include_file->get_data() + tess_orig_shader);
	auto voxel_str = context->get_voxel_result().uniforms + include_file->get_data() + context->get_voxel_result().text;



	auto raytracing_str = context->hit_shader.uniforms + include_file_raytacing->get_data() + context->hit_shader.text;


	raytracing_lib = HAL::library_shader::get_resource({ raytracing_str, "" , 0, context->hit_shader.macros, true });
	pipeline = PipelineManager::get().get_pipeline(ps_str, tess_str, voxel_str, raytracing_str, context);
	ps_uniforms = context->uniforms_ps;


	//	tess_uniforms = context->uniforms_tess;


		//generate_texture_handles();

		//   if (textures_changed)
	{
		for (auto& t : textures)
		{
			t->asset.destroy();
		}
		textures = context->get_textures();

		compile();
	}

	//  if ((textures_changed || shaders_changed))
	mark_contents_changed();

	need_regenerate_material = false;

	on_change();
}

MaterialGraph::ptr materials::universal_material::get_graph()
{
	return  graph.get();
}


materials::universal_material::universal_material(MaterialGraph::ptr graph) : include_file(this), include_file_raytacing(this)
{


	include_file = EngineAssets::material_header.get_asset();
	include_file_raytacing = EngineAssets::material_raytracing_header.get_asset();
	this->graph = BinaryData<MaterialGraph>(graph);
	graph->add_listener(this, false);
	need_regenerate_material = true;
	need_update_uniforms = true;
	generate_material();
}

void materials::universal_material::update_rtx()
{
	if (!HAL::Device::get().is_rtx_supported()) return;
	RTX::get().rtx.update_material(this);



}

void materials::universal_material::test()
{
	graph.test();
}


materials::universal_material::universal_material() : include_file(this), include_file_raytacing(this)
{

	wshader_name = std::wstring(L"material_") + std::to_wstring(ids.get());
	graph.on_create = [this](MaterialGraph::ptr g)
	{
		g->add_listener(this, false);
		on_graph_changed();
	};
}

void materials::universal_material::on_asset_change(std::shared_ptr<Asset> asset)
{
	if (asset == *include_file || asset == *include_file_raytacing)
		on_graph_changed();

	if (asset->get_type() == Asset_Type::TEXTURE)
		on_graph_changed();
}

void materials::universal_material::on_unlink(::FlowGraph::parameter*, ::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_link(::FlowGraph::parameter*, ::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_remove_output(::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_add_output(::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_remove_input(::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_add_input(::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_remove(::FlowGraph::window*)
{
	on_graph_changed();
}

void materials::universal_material::on_register(::FlowGraph::window*)
{
	on_graph_changed();
}

D3D_PRIMITIVE_TOPOLOGY materials::render_pass::get_topology()
{
	return  ds_shader ? D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST : D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

size_t materials::render_pass::get_pipeline_id()
{
	return pipeline_id;
}


//CEREAL_REGISTER_DYNAMIC_INIT(myclasses)