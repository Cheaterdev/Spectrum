module Graphics:VoxelGI;
import <RenderSystem.h>;

import Graphics;
import HAL;
import Core;



using namespace FrameGraph;
using namespace HAL;


class GBufferDownsampler :public Events::prop_handler
{

public:
	using ptr = std::shared_ptr<GBufferDownsampler>;


	void generate(Graph& graph)
	{
		auto& frame = graph.get_context<ViewportInfo>();

		auto size = frame.frame_size;

		graph.add_library_pass<Passes::GBufferDownsampler>([this, size](auto& data, TaskBuilder& builder) {

			GBufferViewDesc::need(builder, data.gbuffer, true);
			builder.create(data.gbuffer.GBuffer_TempColor, { ivec3(size,0), HAL::Format::R8G8_UNORM,1,1 }, ResourceFlags::RenderTarget);

			return true;
			}, [this, &graph](auto& data, FrameContext& _context) {

				auto& command_list = _context.get_list();
				auto tempColor = *data.gbuffer.GBuffer_TempColor;
				GBuffer gbuffer = GBufferViewDesc::actualize(data.gbuffer);
				auto& graphics = command_list->get_graphics();

				graphics.set_signature(Layouts::DefaultLayout);

				graph.set_slot(SlotID::FrameInfo, graphics);

				graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);


				graphics.set_pipeline<PSOS::GBufferDownsample>();

				for (uint i = 1; i < gbuffer.depth_mips.resource->get_desc().as_texture().MipLevels; i++)
				{

					{

						HAL::TextureViewDesc subres;

						subres.ArraySize = 1;
						subres.FirstArraySlice = 0;
						subres.MipLevels = 1;
						subres.MipSlice = i;
						auto depth_view = gbuffer.depth_mips.resource->create_view<HAL::Texture2DView>(graphics.get_base(), subres);
						auto normal_view = gbuffer.normals.resource->create_view<HAL::Texture2DView>(graphics.get_base(), subres);

						RT::GBufferDownsampleRT rt;

						rt.GetColor() = normal_view.renderTarget;

						rt.GetDepth() = depth_view.renderTarget;

						graphics.set_rtv(rt);

					}


					Slots::GBufferDownsample downsample;



					{
						HAL::TextureViewDesc subres;

						subres.ArraySize = 1;
						subres.FirstArraySlice = 0;
						subres.MipLevels = 1;
						subres.MipSlice = i - 1;

						downsample.GetDepth() = gbuffer.depth_mips.resource->create_view<HAL::Texture2DView>(graphics.get_base(), subres).texture2D;
						downsample.GetNormals() = gbuffer.normals.resource->create_view<HAL::Texture2DView>(graphics.get_base(), subres).texture2D;
					}
					graphics.set(downsample);
					graphics.draw(4);
				}

				MipMapGenerator::get().generate_quality(graphics, nullptr, gbuffer, tempColor);



			});

	}


};



VoxelGI::VoxelGI(Scene::ptr& scene) :scene(scene), VariableContext(L"VoxelGI")
{
	scene->on_element_add.register_handler(this, [this](scene_object* object) {
		auto render_object = dynamic_cast<MeshAssetInstance*>(object);

		if (!render_object) return;

		if (render_object->type == MESH_TYPE::STATIC)
			need_start_new = true;
		});



	{
		auto desc = HAL::ResourceDesc::Tex3D(Format::R8G8B8A8_UNORM, { 512, 512, 512 }, 1, HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess/*, D3D12_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE*/);

		albedo.set(desc);
		normal.set(desc);

		desc.as_texture().MipLevels = 7;
		desc.as_texture().Format = Format::R16G16B16A16_FLOAT;
		tex_lighting.set(desc);

		lighed_to_albedo_coeff = tex_lighting.tex_result->resource->get_tiled_manager().get_tiles_count() / albedo.tex_result->resource->get_tiled_manager().get_tiles_count();
		dynamic_generator_lighted = TileDynamicGenerator(tex_lighting.tex_result->resource->get_tiled_manager().get_tiles_count());
		dynamic_generator_voxelizing = TileDynamicGenerator(albedo.tex_result->resource->get_tiled_manager().get_tiles_count());
		visibility = std::make_shared<VisibilityBufferUniversal>(tex_lighting.tex_result->resource->get_tiled_manager().get_tiles_count());

		tex_lighting.tex_result->resource->get_tiled_manager().on_load = [this](ivec4 pos)
		{
			if (gpu_tiles_buffer[pos.w])
			{
				gpu_tiles_buffer[pos.w]->insert(pos.xyz);
			}

		};

		tex_lighting.tex_result->resource->get_tiled_manager().on_zero = [this](ivec4 pos)
		{
			if (gpu_tiles_buffer[pos.w])
			{
				gpu_tiles_buffer[pos.w]->erase(pos.xyz);
			}
		};


		albedo.tex_dynamic->resource->get_tiled_manager().on_load = [this](ivec4 pos)
		{
			albedo_tiles->insert(pos.xyz);
		};

		albedo.tex_dynamic->resource->get_tiled_manager().on_zero = [this](ivec4 pos)
		{
			albedo_tiles->erase(pos.xyz);
		};
		albedo_tiles.reset(new GPUTilesBuffer);
		albedo_tiles->set_size(albedo.tex_result->resource->get_tiled_manager().get_tiles_count(), albedo.tex_result->resource->get_tiled_manager().get_tile_shape());


		gpu_tiles_buffer.resize(tex_lighting.tex_result->resource->get_tiled_manager().unpacked_mip_count);

		for (int i = 0; i < gpu_tiles_buffer.size(); i++)
		{
			gpu_tiles_buffer[i].reset(new GPUTilesBuffer);
			gpu_tiles_buffer[i]->set_size(tex_lighting.tex_result->resource->get_tiled_manager().get_tiles_count(i), tex_lighting.tex_result->resource->get_tiled_manager().get_tile_shape());
		}


	}

	init_states();

}




void VoxelGI::init_states()
{



}

void VoxelGI::start_new(HAL::CommandList& list)
{


	tex_lighting.zero_tiles(list);

	albedo.zero_tiles(list);
	normal.zero_tiles(list);


	dynamic_generator_lighted.remove_all();
	dynamic_generator_voxelizing.remove_all();

}


void VoxelGI::voxelize(MeshRenderContext::ptr& context, main_renderer* r, Graph& graph)
{
	auto& graphics = context->list->get_graphics();
	auto& compute = context->list->get_compute();
	auto& list = *context->list;



	if (all_scene_regen_counter > 0)
	{
		if (vis_update.valid())
		{
			auto updates = vis_update.get();


			auto albedo_tiles = updates.tiles_to_load | std::views::transform([this](ivec3 pos) {return pos / lighed_to_albedo_coeff; });

			albedo.tex_static->resource->get_tiled_manager().load_tiles2(&list, albedo_tiles);
			normal.tex_static->resource->get_tiled_manager().load_tiles2(&list, albedo_tiles);

			tex_lighting.load_static(updates.tiles_to_load);
		}
	}

	if (!all_scene_regen_counter) {
		dynamic_generator_lighted.begin(scene->voxel_info.GetMin().xyz, scene->voxel_info.GetMin().xyz + scene->voxel_info.GetSize().xyz);
		dynamic_generator_voxelizing.begin(scene->voxel_info.GetMin().xyz, scene->voxel_info.GetMin().xyz + scene->voxel_info.GetSize().xyz);

		scene->iterate_meshes(MESH_TYPE::DYNAMIC, [this](scene_object::ptr obj) {

			MeshAssetInstance* mesh = dynamic_cast<MeshAssetInstance*>(obj.get());

			if (mesh)
			{
				for (auto elem : mesh->rendering)
				{
					auto min = elem.primitive_global->get_min();
					auto max = elem.primitive_global->get_max();

					dynamic_generator_lighted.add(min, max);
					dynamic_generator_voxelizing.add(min, max);
				}
			}

			}
		);
		dynamic_generator_lighted.end();
		dynamic_generator_voxelizing.end();
	}

	albedo.tex_dynamic->resource->get_tiled_manager().zero_tiles(&list, dynamic_generator_voxelizing.tiles_to_remove);
	normal.tex_dynamic->resource->get_tiled_manager().zero_tiles(&list, dynamic_generator_voxelizing.tiles_to_remove);
	tex_lighting.zero_dynamic(dynamic_generator_lighted.tiles_to_remove);


	albedo.tex_dynamic->resource->get_tiled_manager().load_tiles(&list, dynamic_generator_voxelizing.tiles_to_load);
	normal.tex_dynamic->resource->get_tiled_manager().load_tiles(&list, dynamic_generator_voxelizing.tiles_to_load);


	tex_lighting.load_dynamic(dynamic_generator_lighted.tiles_to_load);


	PROFILE_GPU(L"voxelizing");

	if (clear_scene && all_scene_regen_counter)
	{
		PROFILE_GPU(L"clear");
		list.clear_uav(albedo.tex_static->texture_3d().mips[0].rwTexture3D);
	}
	else
	{
		PROFILE_GPU(L"copy");

		albedo_tiles->update(context->list);
		compute.set_pipeline<PSOS::VoxelCopy>();
		graph.set_slot(SlotID::VoxelInfo, compute);
		{

			{
				Slots::VoxelCopy utils;
				utils.GetTarget()[0] = albedo.tex_dynamic->texture_3d().mips[0].rwTexture3D;
				utils.GetSource()[0] = albedo.tex_static->texture_3d().texture3D;

				utils.GetTarget()[1] = normal.tex_dynamic->texture_3d().mips[0].rwTexture3D;
				utils.GetSource()[1] = normal.tex_static->texture_3d().texture3D;

				auto& params = utils.GetParams();
				params.GetTiles() = albedo_tiles->buffer;
				params.GetVoxels_per_tile() = ivec4(normal.tex_result->resource->get_tiled_manager().get_tile_shape(), 0);
				compute.set(utils);
			}

			compute.exec_indirect(albedo_tiles->dispatch_buffer, 1);
		}
	}


	context->render_type = RENDER_TYPE::VOXEL;

	Slots::Voxelization voxelization;
	voxelization.GetInfo().GetMin() = scene->voxel_info.GetMin();
	voxelization.GetInfo().GetSize() = scene->voxel_info.GetSize();
	voxelization.GetInfo().GetVoxel_tiles_count() = scene->voxel_info.GetVoxel_tiles_count();
	voxelization.GetInfo().GetVoxels_per_tile() = scene->voxel_info.GetVoxels_per_tile();

	voxelization.GetVisibility() = visibility->buffer->resource->create_view<HAL::Texture3DView>(list).mips[0].rwTexture3D;


	if (all_scene_regen_counter)
	{
		context->render_mesh = MESH_TYPE::STATIC;
		voxelization.GetAlbedo() = albedo.tex_static->texture_3d().mips[0].rwTexture3D;
		voxelization.GetNormals() = normal.tex_static->texture_3d().mips[0].rwTexture3D;
	}
	else
	{
		context->render_mesh = MESH_TYPE::DYNAMIC;

		voxelization.GetAlbedo() = albedo.tex_dynamic->texture_3d().mips[0].rwTexture3D;
		voxelization.GetNormals() = normal.tex_dynamic->texture_3d().mips[0].rwTexture3D;
	}

	albedo.flush(list);
	normal.flush(list);
	tex_lighting.flush(list);

	context->voxelization_compiled = voxelization.compile(list);

	graphics.set_viewport(float4{ 0, 0,  albedo.tex_dynamic->get_size().xy });
	graphics.set_scissor({ 0, 0,  albedo.tex_dynamic->get_size().xy });

				{
				RT::NoOutput rt;
				graphics.set_rtv(rt);
				}

	{
		PROFILE_GPU(L"render");
		r->render(context, scene);
	}


	if (all_scene_regen_counter)
	{
		PROFILE_GPU(L"visibility update");
		vis_update = visibility->update(context->list);
	}

	for (auto&& b : gpu_tiles_buffer)
		if (b)
			b->update(context->list);


}
