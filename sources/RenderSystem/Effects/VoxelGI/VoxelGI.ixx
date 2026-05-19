export module Graphics:VoxelGI;

import :PSSM;
import :VisibilityBuffer;
import :TileDynamicGenerator;
import :GPUTilesBuffer;
import :Scene;
import :MeshRenderer;
import :ReflectionDenoiser;
import :MipMapGenerator;
import :FrameGraphContext;
		import :RTX;

import FrameGraph;
import HAL;
#include "profiling/macros.h"

//class GBufferDownsampler;

using namespace HAL;
using namespace FrameGraph;


export class Texture3DMultiTiles
{
	HAL::update_tiling_info tilings_info;

public:
	HAL::Texture::ptr tex_dynamic;
	HAL::Texture::ptr tex_static;


	HAL::Texture::ptr tex_result;


	void flush(HAL::CommandList& list)
	{
		tilings_info.resource = tex_result->resource.get();
		list.update_tilings(std::move(tilings_info));
	}


	void set(HAL::ResourceDesc desc)
	{
		desc.Flags |= ResFlags::Virtual | ResFlags::DisableStateTracking;
		tex_dynamic.reset(new HAL::Texture(desc, TextureLayout::SHADER_RESOURCE));
		tex_static.reset(new HAL::Texture(desc, TextureLayout::SHADER_RESOURCE));

		tex_result.reset(new HAL::Texture(desc, TextureLayout::SHADER_RESOURCE));

		tex_dynamic->resource->set_name("tex_dynamic");

		tex_dynamic->resource->set_name("tex_static");
		tex_dynamic->resource->set_name("tex_result");

		tex_dynamic->resource->get_tiled_manager().on_load = [this](ivec4 pos)
		{
			auto heap_pos = tex_dynamic->resource->get_tiled_manager().tiles[0][pos.xyz].heap_position;
			heap_pos.handle = ResourceHandle();
			tex_result->resource->get_tiled_manager().map_tile(tilings_info, pos.xyz, heap_pos);
		};


		tex_dynamic->resource->get_tiled_manager().on_zero = [this](ivec4 pos)
		{
			if (tex_static->resource->get_tiled_manager().is_mapped(pos.xyz, pos.w))
			{
				auto heap_pos = tex_static->resource->get_tiled_manager().tiles[0][pos.xyz].heap_position;
				heap_pos.handle = ResourceHandle();
				tex_result->resource->get_tiled_manager().map_tile(tilings_info, pos.xyz, heap_pos);
			}
			else
			{
				tex_result->resource->get_tiled_manager().zero_tile(tilings_info, pos.xyz, 0);
			}
		};


		tex_static->resource->get_tiled_manager().on_load = [this](ivec4 pos)
		{
			if (!tex_dynamic->resource->get_tiled_manager().is_mapped(pos.xyz, pos.w))
			{
				auto heap_pos = tex_static->resource->get_tiled_manager().tiles[0][pos.xyz].heap_position;
				heap_pos.handle = ResourceHandle();
				tex_result->resource->get_tiled_manager().map_tile(tilings_info, pos.xyz, heap_pos);
			}
		};

		tex_static->resource->get_tiled_manager().on_zero = [this](ivec4 pos)
		{
			if (!tex_dynamic->resource->get_tiled_manager().is_mapped(pos.xyz, pos.w))
			{
				tex_result->resource->get_tiled_manager().zero_tile(tilings_info, pos.xyz, 0);
			}
		};
	}


	void zero_tiles(HAL::CommandList& list)
	{
		tex_dynamic->resource->get_tiled_manager().zero_tiles((list));
		tex_static->resource->get_tiled_manager().zero_tiles((list));

		tilings_info.tiles.clear();
		//	flush(list);
	}
};


export class Texture3DRefTiles
{
	HAL::update_tiling_info tilings_info;

	grid<ivec3, bool> static_tiles;
	grid<ivec3, bool> dynamic_tiles;

public:
	HAL::Texture::ptr tex_result;


	void flush(HAL::CommandList& list)
	{
		tilings_info.resource = tex_result->resource.get();
		list.update_tilings(std::move(tilings_info));
	}


	void set(HAL::ResourceDesc desc)
	{
		desc.Flags |= ResFlags::Virtual | ResFlags::DisableStateTracking;
		tex_result.reset(new HAL::Texture(desc, TextureLayout::SHADER_RESOURCE));

		static_tiles.resize(tex_result->resource->get_tiled_manager().get_tiles_count(), 0);
		dynamic_tiles.resize(tex_result->resource->get_tiled_manager().get_tiles_count(), 0);
	}


	void load_static(std::list<uint3>& tiles)
	{
		for (auto& pos : tiles)
		{
			static_tiles[pos] = true;

			if (!dynamic_tiles[pos])
			{
				tex_result->resource->get_tiled_manager().load_tile(tilings_info, pos, 0, true);
			}
		}
	}

	void zero_static(std::list<uint3>& tiles)
	{
		for (auto& pos : tiles)
		{
			static_tiles[pos] = false;
			if (!dynamic_tiles[pos])
			{
				tex_result->resource->get_tiled_manager().zero_tile(tilings_info, pos, 0);
			}
		}
	}

	void load_dynamic(std::list<uint3>& tiles)
	{
		for (auto& pos : tiles)
		{
			dynamic_tiles[pos] = true;

			if (!static_tiles[pos])
			{
				tex_result->resource->get_tiled_manager().load_tile(tilings_info, pos, 0, true);
			}
		}
	}

	void zero_dynamic(std::list<uint3>& tiles)
	{
		for (auto& pos : tiles)
		{
			dynamic_tiles[pos] = false;
			if (!static_tiles[pos])
			{
				tex_result->resource->get_tiled_manager().zero_tile(tilings_info, pos, 0);
			}
		}
	}


	void zero_tiles(HAL::CommandList& list)
	{
		tilings_info.tiles.clear();

		tex_result->resource->get_tiled_manager().zero_tiles(list);


		static_tiles.fill(false);
		dynamic_tiles.fill(false);
	}
};


export class VoxelGI : public Events::prop_handler, VariableContext
{
public:
	enum class VISUALIZE_TYPE :int
	{
		FULL = 0,
		INDIRECT = 1,
		REFLECTION = 2,
		VOXEL = 3
	};

	float3 min;
	float3 size;

private:
//	std::shared_ptr<GBufferDownsampler> downsampler;

	VisibilityBufferUniversal::ptr visibility;
	ReflectionDenoiser reflection_denoiser;

	TileDynamicGenerator dynamic_generator_voxelizing;
	TileDynamicGenerator dynamic_generator_lighted;

	Scene::ptr& scene;

	int all_scene_regen_counter = 0;

	bool need_start_new = false;
	int gi_index = 0;
	int refl_index = 0;
	void init_states();
	int light_counter = 0;

	std::future<visibility_update> vis_update;

	struct EyeData : public prop_handler
	{
		HAL::Texture::ptr downsampled_reflection;
		HAL::Texture::ptr current_gi_texture;

		Events::prop<ivec2> size;
		EyeData();
	};

	bool recreate_static = false;

	// private render helpers (defined in VoxelGIGraph.cpp, use <RenderSystem.h> symbols)
	void voxelize(MeshRenderContext::ptr& context, main_renderer* r, Graph& graph);

public:
	using ptr = std::shared_ptr<VoxelGI>;

	ivec3 lighed_to_albedo_coeff;
	std::vector<GPUTilesBuffer::ptr> gpu_tiles_buffer;

	GPUTilesBuffer::ptr albedo_tiles;

	Texture3DMultiTiles albedo;
	Texture3DMultiTiles normal;
	Texture3DRefTiles tex_lighting;

	Variable<bool> voxelize_scene = {true, "voxelize_scene", this};
	Variable<bool> light_scene = {true, "light_scene", this};
	Variable<bool> clear_scene = {true, "clear_scene", this};

	Variable<bool> use_rtx = {true, "use_rtx", this};
	Variable<bool> multiple_bounces = {true, "multiple_bounces", this};


	Variable<bool> denoiser = {true, "denoiser", this};
	Variable<bool> reflecton = {true, "reflecton", this};


	void pass_data( FrameGraph::TaskBuilder& builder)
	{
			// Always register external textures with the frame graph
		builder.pass_texture("VoxelAlbedo",        albedo.tex_result);
		builder.pass_texture("VoxelNormal",        normal.tex_result);
		builder.pass_texture("VoxelLighted",       tex_lighting.tex_result);
		builder.pass_texture("VoxelAlbedoStatic",  albedo.tex_static);
		builder.pass_texture("VoxelNormalStatic",  normal.tex_static);
		builder.pass_texture("VoxelAlbedoDynamic", albedo.tex_dynamic);
		builder.pass_texture("VoxelNormalDynamic", normal.tex_dynamic);
	}

	void resize(ivec2 size);
	void start_new(HAL::CommandList& list);

	VoxelGI(Scene::ptr& scene);
	void generate(MeshRenderContext::ptr& context, main_renderer::ptr r, PSSM& pssm);

	template<typename TPipeline>
	explicit VoxelGI(TPipeline& pipeline, Scene::ptr& scene) : VoxelGI(scene)
	{
		// ---- Voxelize -------------------------------------------------------
		// setup also performs the generate_pre work (pass_texture, voxel_info)

		pipeline.voxelize.setup_func = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
		
			// Update voxel bounds and info
			Slots::VoxelInfo& voxel_info = this->scene->voxel_info;
			min  = this->scene->get_min() - float3(1, 1, 1);
			size = this->scene->get_max() + float3(1, 1, 1) - this->scene->get_min();

			if (all_scene_regen_counter > 0)
				all_scene_regen_counter--;

			static bool prev = false;
			bool cur = !!GetAsyncKeyState('P');
			if (!cur && prev)
				need_start_new = true;
			prev = cur;

			if (need_start_new)
				all_scene_regen_counter = 3;

			voxel_info.GetMin().xyz  = min;
			voxel_info.GetSize().xyz = size;
			voxel_info.GetSize().x = voxel_info.GetSize().y = voxel_info.GetSize().z =
				std::max(200.0f, voxel_info.GetSize().max_element());
			voxel_info.GetVoxel_tiles_count().xyz =
				tex_lighting.tex_result->resource->get_tiled_manager().get_tiles_count(0);
			voxel_info.GetVoxels_per_tile().xyz =
				tex_lighting.tex_result->resource->get_tiled_manager().get_tile_shape();

			this->scene->voxels_compiled =
				this->scene->voxel_info.compile(*builder.graph->builder.current_frame);
			builder.graph->register_slot_setter(this->scene->voxels_compiled);

			if (!voxelize_scene) return false;

			builder.need(data.VoxelAlbedo,        ResourceFlags::UnorderedAccess);
			builder.need(data.VoxelNormal,        ResourceFlags::UnorderedAccess);
			builder.need(data.VoxelAlbedoStatic,  ResourceFlags::UnorderedAccess);
			builder.need(data.VoxelNormalStatic,  ResourceFlags::UnorderedAccess);
			builder.need(data.VoxelAlbedoDynamic, ResourceFlags::UnorderedAccess);
			builder.need(data.VoxelNormalDynamic, ResourceFlags::UnorderedAccess);
			return true;
		};

		pipeline.voxelize.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& command_list = context.get_list();
			auto& cam          = context.graph->get_context<CameraInfo>();
			auto& sceneinfo    = context.graph->get_context<SceneInfo>();

			if (need_start_new)
			{
				start_new(*command_list);
				need_start_new = false;
			}

			MeshRenderContext::ptr mesh_ctx(new MeshRenderContext());
			mesh_ctx->current_time = 0;
			mesh_ctx->priority     = TaskPriority::HIGH;
			mesh_ctx->list         = command_list;
			mesh_ctx->cam          = cam.cam;

			auto scene    = sceneinfo.scene;
			auto renderer = sceneinfo.renderer;
			mesh_ctx->begin();

			voxelize(mesh_ctx, renderer.get(), *context.graph);
		};

		// ---- Lighting -------------------------------------------------------

		pipeline.lighting.flags       = PassFlags::Compute;
		pipeline.lighting.setup_func  = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			light_counter = (light_counter + 1) % 5;
			if (!light_scene) return false;

			builder.need(data.global_depth,          ResourceFlags::ComputeRead);
			builder.need(data.global_camera,         ResourceFlags::ComputeRead);
			builder.need(data.sky_cubemap_filtered,  ResourceFlags::PixelRead);
			builder.need(data.VoxelLighted,          ResourceFlags::UnorderedAccess);
			builder.need(data.VoxelAlbedo,           ResourceFlags::ComputeRead);
			builder.need(data.VoxelNormal,           ResourceFlags::ComputeRead);
			builder.need(data.VoxelAlbedoStatic,     ResourceFlags::ComputeRead);
			builder.need(data.VoxelNormalStatic,     ResourceFlags::ComputeRead);
			builder.need(data.VoxelAlbedoDynamic,    ResourceFlags::ComputeRead);
			builder.need(data.VoxelNormalDynamic,    ResourceFlags::ComputeRead);
			return true;
		};

		pipeline.lighting.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& command_list = context.get_list();
			auto  sky_cubemap_filtered = *data.sky_cubemap_filtered;
			auto& cam = context.graph->get_context<CameraInfo>();

			MeshRenderContext::ptr mesh_ctx(new MeshRenderContext());
			mesh_ctx->current_time = 0;
			mesh_ctx->priority     = TaskPriority::HIGH;
			mesh_ctx->list         = command_list;
			mesh_ctx->cam          = cam.cam;

			auto& list    = *mesh_ctx->list;
			auto& compute = mesh_ctx->list->get_compute();

			compute.set_pipeline<PSOS::Lighting>(
				PSOS::Lighting::SecondBounce.Use((all_scene_regen_counter == 0) && multiple_bounces));

			Slots::VoxelLighting ligthing;
			{
				ligthing.GetAlbedo()  = albedo.tex_result->texture_3d().texture3D;
				ligthing.GetNormals() = normal.tex_result->texture_3d().texture3D;
				ligthing.GetOutput()  = tex_lighting.tex_result->texture_3d().mips[0].rwTexture3D;
				ligthing.GetTex_cube() = sky_cubemap_filtered.textureCube;

				HAL::Texture3DViewDesc subres;
				subres.MipLevels = tex_lighting.tex_result->get_desc().as_texture().MipLevels - 1;
				subres.MipSlice  = 1;
				ligthing.GetLower() =
					tex_lighting.tex_result->resource->create_view<Texture3DView>(list, subres).texture3D;

				auto& params = ligthing.GetParams();
				params.GetTiles() = gpu_tiles_buffer[0]->buffer;
				params.GetVoxels_per_tile().xyz =
					tex_lighting.tex_result->resource->get_tiled_manager().get_tile_shape();

				auto& pssm = ligthing.GetPssmGlobal();
				pssm.GetLight_buffer() = data.global_depth->texture2D;
				auto buffer_view = data.global_camera->resource->create_view<
					HAL::StructuredBufferView<Table::Camera>>(list);
				pssm.GetLight_camera() = buffer_view;

				compute.set(ligthing);
			}

			context.graph->set_slot(SlotID::VoxelInfo, compute);
			compute.exec_indirect(gpu_tiles_buffer[0]->dispatch_buffer, 1);
		};

		// ---- Mipmapping -----------------------------------------------------

		pipeline.mipmapping.flags       = PassFlags::Compute;
		pipeline.mipmapping.setup_func  = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			if (!light_scene) return false;
			builder.need(data.VoxelLighted, ResourceFlags::UnorderedAccess);
			return true;
		};

		pipeline.mipmapping.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& command_list = context.get_list();
			auto  voxel_lighted = *data.VoxelLighted;
			auto& cam = context.graph->get_context<CameraInfo>();

			MeshRenderContext::ptr mesh_ctx(new MeshRenderContext());
			mesh_ctx->current_time = 0;
			mesh_ctx->priority     = TaskPriority::HIGH;
			mesh_ctx->list         = command_list;
			mesh_ctx->cam          = cam.cam;

			auto& list    = *mesh_ctx->list;
			auto& compute = mesh_ctx->list->get_compute();

			compute.set_signature(Layouts::DefaultLayout);
			context.graph->set_slot(SlotID::VoxelInfo, compute);

			compute.set_pipeline<PSOS::VoxelZero>();
			{
				PROFILE_GPU(L"ZERO");
				for (uint i = 1; i < tex_lighting.tex_result->get_desc().as_texture().MipLevels; i++)
				{
					if (i >= gpu_tiles_buffer.size() || !gpu_tiles_buffer[i])
					{
						list.clear_uav(tex_lighting.tex_result->texture_3d().mips[i].rwTexture3D,
							vec4(0, 0, 0, 0));
						continue;
					}

					{
						Slots::VoxelZero utils;
						utils.GetTarget() = tex_lighting.tex_result->texture_3d().mips[i].rwTexture3D;
						auto& params = utils.GetParams();
						params.GetTiles() = gpu_tiles_buffer[i]->buffer;
						params.GetVoxels_per_tile().xyz =
							tex_lighting.tex_result->resource->get_tiled_manager().get_tile_shape();
						compute.set(utils);
					}

					compute.exec_indirect(gpu_tiles_buffer[i]->dispatch_buffer, 1);
				}
			}

			{
				PROFILE_GPU(L"EXEC");
				unsigned int mip_count = 1;
				while (mip_count < tex_lighting.tex_result->get_desc().as_texture().MipLevels)
				{
					if (!gpu_tiles_buffer[mip_count]) break;
					unsigned int current_mips = std::min(
						3u, tex_lighting.tex_result->get_desc().as_texture().MipLevels - mip_count);
					compute.set_pipeline<PSOS::VoxelDownsample>(
						PSOS::VoxelDownsample::Count(current_mips));

					{
						Slots::VoxelMipMap mipmapping;
						mipmapping.GetSrcMip() =
							tex_lighting.tex_result->texture_3d().mips[mip_count - 1].texture3D;
						for (unsigned int i = 0; i < current_mips; i++)
							mipmapping.GetOutMips()[i] =
								tex_lighting.tex_result->texture_3d().mips[mip_count + i].rwTexture3D;
						auto& params = mipmapping.GetParams();
						params.GetTiles() = gpu_tiles_buffer[mip_count]->buffer;
						params.GetVoxels_per_tile().xyz =
							tex_lighting.tex_result->resource->get_tiled_manager().get_tile_shape();
						compute.set(mipmapping);
					}
					PROFILE_GPU((std::wstring(L"mip_") + std::to_wstring(mip_count)).c_str());
					compute.exec_indirect(gpu_tiles_buffer[mip_count]->dispatch_buffer, 1);
					mip_count += current_mips;
				}
			}
		};

		// ---- VoxelScreen ----------------------------------------------------

		pipeline.voxelScreen.flags       = PassFlags::Compute;
		pipeline.voxelScreen.setup_func  = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			auto& frame = builder.graph->get_context<ViewportInfo>();
			auto  sz    = frame.frame_size;
			UINT  count = 2 * Math::DivideByMultiple(sz.x, 32) * Math::DivideByMultiple(sz.y, 32);

			GBufferViewDesc::need(builder, data.gbuffer);
			builder.create(data.VoxelFramesCount,
				{ ivec3(sz.x, sz.y, 0), HAL::Format::R16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
			builder.create(data.VoxelIndirectNoise,
				{ ivec3(sz.x, sz.y, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 0 }, ResourceFlags::UnorderedAccess);
			builder.create(data.VoxelIndirectFiltered,
				{ ivec3(sz.x, sz.y, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 },
				ResourceFlags::UnorderedAccess | ResourceFlags::Static);
			builder.need(data.sky_cubemap_filtered, ResourceFlags::PixelRead);
			builder.need(data.VoxelLighted,         ResourceFlags::ComputeRead);
			builder.need(data.BlueNoise,            ResourceFlags::ComputeRead);

			builder.create(data.VoxelScreen_hi,  { 1u, false }, ResourceFlags::UnorderedAccess);
			builder.create(data.VoxelScreen_low, { 1u, false }, ResourceFlags::UnorderedAccess);

			builder.create(data.VoxelScreen_low_data, { count, true }, ResourceFlags::UnorderedAccess);
			builder.create(data.VoxelScreen_hi_data,  { count, true }, ResourceFlags::UnorderedAccess);
			return true;
		};

		pipeline.voxelScreen.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& command_list = context.get_list();

			bool use_rtx_flag = HAL::Device::get().is_rtx_supported() && (bool)use_rtx;
			GBuffer gbuffer                = GBufferViewDesc::actualize(data.gbuffer);
			auto    sky_cubemap_filtered   = *data.sky_cubemap_filtered;
			auto    noisy_output           = *data.VoxelIndirectNoise;
			auto    gi_filtered            = *data.VoxelIndirectFiltered;
			auto    frames_count           = *data.VoxelFramesCount;
			auto    sz                     = noisy_output.get_size();

			auto& sceneinfo = context.graph->get_context<SceneInfo>();

			if (data.VoxelIndirectFiltered.is_new())
			{
				command_list->clear_uav(gi_filtered.rwTexture2D,   vec4(0, 0, 0, 0));
				command_list->clear_uav(noisy_output.rwTexture2D,  vec4(0, 0, 0, 0));
				command_list->clear_uav(frames_count.rwTexture2D,  vec4(0, 0, 0, 0));
			}

			auto& compute = command_list->get_compute();

			if (use_rtx_flag)
				command_list->get_compute().set_signature(RTX::get().rtx.m_root_sig);
			else
				compute.set_signature(Layouts::DefaultLayout);

			context.graph->set_slot(SlotID::FrameInfo,  compute);
			context.graph->set_slot(SlotID::VoxelInfo,  compute);
			context.graph->set_slot(SlotID::SceneData,  compute);

			{
				Slots::VoxelScreen voxelScreen;
				gbuffer.SetTable(voxelScreen.GetGbuffer());
				voxelScreen.GetVoxels()    = tex_lighting.tex_result->texture_3d().texture3D;
				voxelScreen.GetTex_cube()  = sky_cubemap_filtered.textureCube;
				voxelScreen.GetPrev_depth() = gbuffer.depth_prev_mips.texture2D;
				voxelScreen.GetPrev_gi()   = gi_filtered.texture2D;
				compute.set(voxelScreen);
			}

			{
				Slots::VoxelOutput output;
				output.GetFrames()    = frames_count.rwTexture2D;
				output.GetNoise()     = noisy_output.rwTexture2D;
				output.GetBlueNoise() = data.BlueNoise->texture2D;
				compute.set(output);
			}

			if (use_rtx_flag)
				RTX::get().render<Indirect>(compute, sceneinfo.scene->raytrace_scene, noisy_output.get_size());
			else
			{
				PROFILE_GPU(L"noise");
				compute.set_pipeline<PSOS::VoxelIndirectLow>();
				compute.dispatch(sz);
			}

			compute.set_signature(Layouts::DefaultLayout);
			{
				compute.set_pipeline<PSOS::FrameClassification>();

				PROFILE_GPU(L"classification");
				compute.clear_counter(*data.VoxelScreen_hi_data);
				compute.clear_counter(*data.VoxelScreen_low_data);

				{
					Slots::FrameClassification frame_classification;
					frame_classification.GetFrames() = frames_count.texture2D;
					frame_classification.GetHi()     = data.VoxelScreen_hi_data->appendStructuredBuffer;
					frame_classification.GetLow()    = data.VoxelScreen_low_data->appendStructuredBuffer;
					compute.set(frame_classification);
				}

				uint2 tiles_count = uint2(Math::DivideByMultiple(sz.x, 32), Math::DivideByMultiple(sz.y, 32));
				compute.dispatch(tiles_count);
			}
			{
				PROFILE_GPU(L"init_dispatch");
				Slots::FrameClassificationInitDispatch init;
				init.GetHi_counter()       = data.VoxelScreen_hi_data->counter_view;
				init.GetLow_counter()      = data.VoxelScreen_low_data->counter_view;
				init.GetHi_dispatch_data() = data.VoxelScreen_hi->rwStructuredBuffer;
				init.GetLow_dispatch_data() = data.VoxelScreen_low->rwStructuredBuffer;
				compute.set(init);
				compute.set_pipeline<PSOS::FrameClassificationInitDispatch>();
				compute.dispatch(1, 1, 1);
			}

			{
				PROFILE_GPU(L"mipmaps");
				MipMapGenerator::get().generate(compute, noisy_output);
			}

			if (denoiser)
			{
				PROFILE_GPU(L"history");
				compute.set_pipeline<PSOS::DenoiserHistoryFix>();
				{
					Slots::DenoiserHistoryFix denoiser_history;
					HAL::TextureViewDesc subres;
					subres.ArraySize        = 1;
					subres.FirstArraySlice  = 0;
					subres.MipLevels        = noisy_output.resource->get_desc().as_texture().MipLevels - 1;
					subres.MipSlice         = 1;
					denoiser_history.GetColor() =
						noisy_output.resource->create_view<Texture2DView>(*command_list, subres).texture2D;
					subres.MipLevels = 1;
					denoiser_history.GetFrames()  = frames_count.texture2D;
					denoiser_history.GetTarget()  = noisy_output.rwTexture2D;
					compute.set(denoiser_history);
				}

				{
					Slots::TilingPostprocess tilingPostprocess;
					tilingPostprocess.GetTiling().GetTiles() = data.VoxelScreen_hi_data->structuredBuffer;
					compute.set(tilingPostprocess);
				}
				compute.exec_indirect(*data.VoxelScreen_hi, 1);
			}

			gi_index = 1 - gi_index;
		};

		// ---- VoxelCombine ---------------------------------------------------

		pipeline.voxelCombine.flags       = PassFlags::Compute;
		pipeline.voxelCombine.setup_func  = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			builder.need(data.ResultTexture,        ResourceFlags::UnorderedAccess);
			GBufferViewDesc::need(builder, data.gbuffer);
			builder.need(data.VoxelIndirectFiltered, ResourceFlags::UnorderedAccess);
			builder.need(data.sky_cubemap_filtered,  ResourceFlags::PixelRead);
			builder.need(data.VoxelFramesCount,      ResourceFlags::UnorderedAccess);
			builder.need(data.VoxelIndirectNoise,    ResourceFlags::ComputeRead);
			builder.need(data.VoxelScreen_hi,        ResourceFlags::ComputeRead);
			builder.need(data.VoxelScreen_low,       ResourceFlags::ComputeRead);
			builder.need(data.VoxelScreen_low_data,  ResourceFlags::ComputeRead);
			builder.need(data.VoxelScreen_hi_data,   ResourceFlags::ComputeRead);
			return true;
		};

		pipeline.voxelCombine.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& command_list = context.get_list();

			auto target_tex          = *data.ResultTexture;
			GBuffer gbuffer          = GBufferViewDesc::actualize(data.gbuffer);
			auto sky_cubemap_filtered = *data.sky_cubemap_filtered;
			auto noisy_output        = *data.VoxelIndirectNoise;
			auto frames_count        = *data.VoxelFramesCount;
			auto gi_filtered         = *data.VoxelIndirectFiltered;
			auto sz                  = target_tex.get_size();

			auto& compute = command_list->get_compute();
			compute.set_signature(Layouts::DefaultLayout);
			context.graph->set_slot(SlotID::FrameInfo, compute);

			{
				Slots::VoxelScreen voxelScreen;
				gbuffer.SetTable(voxelScreen.GetGbuffer());
				voxelScreen.GetVoxels()     = tex_lighting.tex_result->texture_3d().texture3D;
				voxelScreen.GetTex_cube()   = sky_cubemap_filtered.textureCube;
				voxelScreen.GetPrev_depth() = gbuffer.depth_prev_mips.texture2D;
				voxelScreen.GetPrev_gi()    = gi_filtered.texture2D;
				compute.set(voxelScreen);
			}
			context.graph->set_slot(SlotID::VoxelInfo, compute);

			{
				Slots::VoxelBlur voxelBlur;
				voxelBlur.GetNoisy_output()  = noisy_output.texture2D;
				voxelBlur.GetPrev_result()   = frames_count.texture2D;
				voxelBlur.GetScreen_result() = target_tex.rwTexture2D;
				voxelBlur.GetGi_result()     = gi_filtered.rwTexture2D;
				compute.set(voxelBlur);
			}

			{
				PROFILE_GPU(L"blur");
				compute.set_pipeline<PSOS::VoxelIndirectFilter>(PSOS::VoxelIndirectFilter::Blur());
				{
					Slots::TilingPostprocess tp;
					tp.GetTiling().GetTiles() = data.VoxelScreen_hi_data->structuredBuffer;
					compute.set(tp);
				}
				compute.exec_indirect(*data.VoxelScreen_hi, 1);
			}

			{
				PROFILE_GPU(L"blur2");
				compute.set_pipeline<PSOS::VoxelIndirectFilter>();
				{
					Slots::TilingPostprocess tp;
					tp.GetTiling().GetTiles() = data.VoxelScreen_low_data->structuredBuffer;
					compute.set(tp);
				}
				compute.exec_indirect(*data.VoxelScreen_low, 1);
			}
		};

		// ---- ScreenReflection -----------------------------------------------

		pipeline.screenReflection.flags       = PassFlags::Compute;
		pipeline.screenReflection.setup_func  = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			if (!reflecton) return false;

			auto& frame = builder.graph->get_context<ViewportInfo>();
			auto  sz    = frame.frame_size;

			builder.create(data.VoxelReflectionNoise,
				{ ivec3(sz.x, sz.y, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
			builder.create(data.noise_dir_pdf,
				{ ivec3(sz.x, sz.y, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
			builder.need(data.BlueNoise,           ResourceFlags::ComputeRead);
			GBufferViewDesc::need(builder, data.gbuffer);
			builder.need(data.sky_cubemap_filtered, ResourceFlags::PixelRead);
			builder.need(data.VoxelLighted,         ResourceFlags::ComputeRead);
			return true;
		};

		pipeline.screenReflection.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& command_list = context.get_list();

			MeshRenderContext::ptr mesh_ctx(new MeshRenderContext());
			GBuffer gbuffer              = GBufferViewDesc::actualize(data.gbuffer);
			auto sky_cubemap_filtered    = *data.sky_cubemap_filtered;
			auto noisy_output            = *data.VoxelReflectionNoise;
			auto dir_and_pdf             = *data.noise_dir_pdf;

			auto& caminfo   = context.graph->get_context<CameraInfo>();
			auto& sceneinfo = context.graph->get_context<SceneInfo>();

			mesh_ctx->current_time = 0;
			mesh_ctx->priority     = TaskPriority::HIGH;
			mesh_ctx->list         = command_list;
			mesh_ctx->cam          = caminfo.cam;

			auto scene    = sceneinfo.scene;
			mesh_ctx->begin();

			auto& compute = mesh_ctx->list->get_compute();

			context.graph->set_slot(SlotID::FrameInfo,  compute);
			context.graph->set_slot(SlotID::SceneData,  compute);

			{
				Slots::VoxelScreen voxelScreen;
				gbuffer.SetTable(voxelScreen.GetGbuffer());
				voxelScreen.GetVoxels()   = tex_lighting.tex_result->texture_3d().texture3D;
				voxelScreen.GetTex_cube() = sky_cubemap_filtered.textureCube;
				compute.set(voxelScreen);
			}

			context.graph->set_slot(SlotID::VoxelInfo, compute);

			if (use_rtx)
			{
				{
					Slots::VoxelOutput output;
					output.GetNoise()       = noisy_output.rwTexture2D;
					output.GetDirAndPdf()   = dir_and_pdf.rwTexture2D;
					output.GetBlueNoise()   = data.BlueNoise->texture2D;
					compute.set(output);
				}
				RTX::get().render<Reflection>(compute, scene->raytrace_scene, noisy_output.get_size());
			}

			refl_index = 1 - refl_index;
		};

		// ---- ReflectionDenoiser_Reproject -----------------------------------

		pipeline.reflectionDenoiser_Reproject.flags      = PassFlags::Compute;
		pipeline.reflectionDenoiser_Reproject.setup_func =
			[this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			if (!reflecton) return false;

			auto& frame      = builder.graph->get_context<ViewportInfo>();
			auto  sz         = frame.frame_size;
			uint2 small_size = uint2(
				(sz.x + 7) / 8,
				(sz.y + 7) / 8);

			builder.need(data.GBuffer_DepthPrev,    ResourceFlags::ComputeRead);
			builder.need(data.GBuffer_NormalsPrev,  ResourceFlags::ComputeRead);
			builder.need(data.GBuffer_Depth,        ResourceFlags::ComputeRead);
			builder.need(data.GBuffer_Normals,      ResourceFlags::ComputeRead);
			builder.need(data.GBuffer_Speed,        ResourceFlags::ComputeRead);
			builder.need(data.BlueNoise,            ResourceFlags::ComputeRead);
			builder.need(data.VoxelReflectionNoise, ResourceFlags::UnorderedAccess);

			builder.create(data.ReflectionDenoiser_RadiancePrev,
				{ ivec3(sz, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 },
				ResourceFlags::UnorderedAccess | ResourceFlags::Static);
			builder.create(data.ReflectionDenoiser_AverageRadiance,
				{ ivec3(small_size, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 },
				ResourceFlags::UnorderedAccess);
			builder.create(data.ReflectionDenoiser_AverageRadiancePrev,
				{ ivec3(small_size, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 },
				ResourceFlags::UnorderedAccess | ResourceFlags::Static);
			builder.create(data.ReflectionDenoiser_Variance,
				{ ivec3(sz, 0), HAL::Format::R16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
			builder.create(data.ReflectionDenoiser_VariancePrev,
				{ ivec3(sz, 0), HAL::Format::R16_FLOAT, 1, 1 },
				ResourceFlags::UnorderedAccess | ResourceFlags::Static);
			builder.create(data.ReflectionDenoiser_SampleCount,
				{ ivec3(sz, 0), HAL::Format::R16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
			builder.create(data.ReflectionDenoiser_SampleCountPrev,
				{ ivec3(sz, 0), HAL::Format::R16_FLOAT, 1, 1 },
				ResourceFlags::UnorderedAccess | ResourceFlags::Static);
			builder.create(data.ReflectionDenoiser_ReprojectedRadiance,
				{ ivec3(sz, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 }, ResourceFlags::UnorderedAccess);
			return true;
		};

		pipeline.reflectionDenoiser_Reproject.render_func =
			[this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& list    = *context.get_list();
			auto& compute = list.get_compute();
			auto& copy    = context.get_list()->get_copy();
			auto& cam     = context.graph->get_context<CameraInfo>();
			auto  sz      = context.graph->get_context<ViewportInfo>().frame_size;

			if (data.ReflectionDenoiser_RadiancePrev.is_new())
			{
				list.clear_uav(data.ReflectionDenoiser_RadiancePrev->rwTexture2D,       vec4(0, 0, 0, 0));
				list.clear_uav(data.ReflectionDenoiser_AverageRadiance->rwTexture2D,    vec4(0, 0, 0, 0));
				list.clear_uav(data.ReflectionDenoiser_AverageRadiancePrev->rwTexture2D, vec4(0, 0, 0, 0));
				list.clear_uav(data.ReflectionDenoiser_Variance->rwTexture2D,           vec4(0, 0, 0, 0));
				list.clear_uav(data.ReflectionDenoiser_VariancePrev->rwTexture2D,       vec4(0, 0, 0, 0));
				list.clear_uav(data.ReflectionDenoiser_SampleCount->rwTexture2D,        vec4(0, 0, 0, 0));
				list.clear_uav(data.ReflectionDenoiser_SampleCountPrev->rwTexture2D,    vec4(0, 0, 0, 0));
				list.clear_uav(data.ReflectionDenoiser_ReprojectedRadiance->rwTexture2D, vec4(0, 0, 0, 0));
			}

			{
				Slots::DenoiserReflectionCommon common;
				common.g_buffer_dimensions     = sz;
				common.g_inv_buffer_dimensions = float2(1.0f, 1.0f) / float2(sz);
				common.g_view                  = cam.cam->camera_cb.current.GetView();
				common.g_inv_view              = cam.cam->camera_cb.current.GetInvView();
				common.g_prev_view_proj        = cam.cam->camera_cb.prev.GetViewProj();
				common.g_inv_proj              = cam.cam->camera_cb.current.GetInvProj();
				common.g_proj                  = cam.cam->camera_cb.current.GetProj();
				common.g_inv_view_proj         = cam.cam->camera_cb.current.GetInvViewProj();
				common.g_temporal_stability_factor = 0.7f;
				common.g_depth_buffer_thickness    = 0.015f;
				common.g_roughness_threshold       = 0.2f;
				common.g_temporal_variance_threshold = 0;
				common.g_frame_index               = 100;
				common.g_most_detailed_mip         = 0;
				common.g_samples_per_quad          = 1;
				common.g_temporal_variance_guided_tracing_enabled = true;
				compute.set(common);
			}

			{
				compute.set_pipeline<PSOS::DenoiserReflectionReproject>();
				Slots::DenoiserReflectionReproject reproject;
				reproject.g_depth_buffer           = data.GBuffer_Depth->texture2D;
				reproject.g_normal                 = data.GBuffer_Normals->texture2D;
				reproject.g_depth_buffer_history   = data.GBuffer_DepthPrev->texture2D;
				reproject.g_normal_history         = data.GBuffer_NormalsPrev->texture2D;
				reproject.g_in_radiance            = data.VoxelReflectionNoise->texture2D;
				reproject.g_radiance_history       = data.ReflectionDenoiser_RadiancePrev->texture2D;
				reproject.g_motion_vector          = data.GBuffer_Speed->texture2D;
				reproject.g_average_radiance_history = data.ReflectionDenoiser_AverageRadiancePrev->texture2D;
				reproject.g_variance_history       = data.ReflectionDenoiser_VariancePrev->texture2D;
				reproject.g_sample_count_history   = data.ReflectionDenoiser_SampleCountPrev->texture2D;
				reproject.g_blue_noise_texture     = data.BlueNoise->texture2D;
				reproject.g_out_reprojected_radiance = data.ReflectionDenoiser_ReprojectedRadiance->rwTexture2D;
				reproject.g_out_average_radiance   = data.ReflectionDenoiser_AverageRadiance->rwTexture2D;
				reproject.g_out_variance           = data.ReflectionDenoiser_Variance->rwTexture2D;
				reproject.g_out_sample_count       = data.ReflectionDenoiser_SampleCount->rwTexture2D;
				compute.set(reproject);
				compute.dispatch(uint3(sz, 1));
			}

			{
				compute.set_pipeline<PSOS::DenoiserReflectionPrefilter>();
				Slots::DenoiserReflectionPrefilter prefilter;
				prefilter.g_depth_buffer        = data.GBuffer_Depth->texture2D;
				prefilter.g_normal              = data.GBuffer_Normals->texture2D;
				prefilter.g_average_radiance    = data.ReflectionDenoiser_AverageRadiance->texture2D;
				prefilter.g_in_radiance         = data.VoxelReflectionNoise->texture2D;
				prefilter.g_in_variance         = data.ReflectionDenoiser_Variance->texture2D;
				prefilter.g_in_sample_count     = data.ReflectionDenoiser_SampleCount->texture2D;
				prefilter.g_out_radiance        = data.ReflectionDenoiser_RadiancePrev->rwTexture2D;
				prefilter.g_out_sample_count    = data.ReflectionDenoiser_SampleCountPrev->rwTexture2D;
				prefilter.g_out_variance        = data.ReflectionDenoiser_VariancePrev->rwTexture2D;
				compute.set(prefilter);
				compute.dispatch(uint3(sz, 1));
			}

			{
				compute.set_pipeline<PSOS::DenoiserReflectionResolve>();
				Slots::DenoiserReflectionResolve resolve;
				resolve.g_normal                  = data.GBuffer_Normals->texture2D;
				resolve.g_average_radiance        = data.ReflectionDenoiser_AverageRadiance->texture2D;
				resolve.g_in_radiance             = data.VoxelReflectionNoise->texture2D;
				resolve.g_in_reprojected_radiance = data.ReflectionDenoiser_ReprojectedRadiance->texture2D;
				resolve.g_in_variance             = data.ReflectionDenoiser_VariancePrev->texture2D;
				resolve.g_in_sample_count         = data.ReflectionDenoiser_SampleCountPrev->texture2D;
				resolve.g_out_radiance            = data.VoxelReflectionNoise->rwTexture2D;
				resolve.g_out_sample_count        = data.ReflectionDenoiser_SampleCount->rwTexture2D;
				resolve.g_out_variance            = data.ReflectionDenoiser_Variance->rwTexture2D;
				compute.set(resolve);
				compute.dispatch(uint3(sz, 1));
			}

			copy.copy_resource(data.ReflectionDenoiser_SampleCountPrev->resource,
			                   data.ReflectionDenoiser_SampleCount->resource);
			copy.copy_resource(data.ReflectionDenoiser_VariancePrev->resource,
			                   data.ReflectionDenoiser_Variance->resource);
			copy.copy_resource(data.ReflectionDenoiser_RadiancePrev->resource,
			                   data.VoxelReflectionNoise->resource);
			copy.copy_resource(data.ReflectionDenoiser_AverageRadiancePrev->resource,
			                   data.ReflectionDenoiser_AverageRadiance->resource);
		};

		// ---- ReflCombine ----------------------------------------------------

		pipeline.reflCombine.flags       = PassFlags::Compute;
		pipeline.reflCombine.setup_func  = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			if (!reflecton) return false;

			builder.need(data.ResultTexture,      ResourceFlags::UnorderedAccess);
			GBufferViewDesc::need(builder, data.gbuffer);
			builder.need(data.VoxelReflectionNoise, ResourceFlags::ComputeRead);
			return true;
		};

		pipeline.reflCombine.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& command_list = context.get_list();
			auto  target_tex   = *data.ResultTexture;
			GBuffer gbuffer    = GBufferViewDesc::actualize(data.gbuffer);
			auto  sz           = target_tex.get_size();
			auto& compute      = command_list->get_compute();

			context.graph->set_slot(SlotID::FrameInfo,  compute);
			context.graph->set_slot(SlotID::SceneData,  compute);

			compute.set_pipeline<PSOS::ReflectionCombine>();

			{
				Slots::ReflectionCombine combine;
				gbuffer.SetTable(combine.GetGbuffer());
				combine.GetReflection() = data.VoxelReflectionNoise->texture2D;
				combine.GetTarget()     = data.ResultTexture->rwTexture2D;
				compute.set(combine);
			}

			context.graph->set_slot(SlotID::FrameInfo, compute);
			compute.dispatch(sz);
		};

		// ---- VoxelDebug -----------------------------------------------------

		pipeline.voxelDebug.setup_func  = [this](auto& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			auto& frame = builder.graph->get_context<ViewportInfo>();
			auto  sz    = frame.frame_size;

			builder.create(data.VoxelDebug,
				{ ivec3(sz, 0), HAL::Format::R16G16B16A16_FLOAT, 1, 1 }, ResourceFlags::RenderTarget);
			builder.need(data.VoxelLighted, ResourceFlags::ComputeRead);
			GBufferViewDesc::need(builder, data.gbuffer);
			return true;
		};

		pipeline.voxelDebug.render_func = [this](auto& data, FrameGraph::FrameContext& context)
		{
			auto& command_list = context.get_list();
			auto  voxel_lighted = *data.VoxelLighted;
			auto& caminfo   = context.graph->get_context<CameraInfo>();
			auto& sceneinfo = context.graph->get_context<SceneInfo>();

			MeshRenderContext::ptr mesh_ctx(new MeshRenderContext());
			auto  target_tex = *data.VoxelDebug;
			GBuffer gbuffer  = GBufferViewDesc::actualize(data.gbuffer);

			mesh_ctx->current_time = 0;
			mesh_ctx->priority     = TaskPriority::HIGH;
			mesh_ctx->list         = command_list;
			mesh_ctx->cam          = caminfo.cam;
			mesh_ctx->begin();

			auto& graphics = mesh_ctx->list->get_graphics();

			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
			graphics.set_viewport(target_tex.get_viewport());
			graphics.set_scissor(target_tex.get_scissor());

			{
				RT::SingleColor rt;
				rt.GetColor() = target_tex.renderTarget;
				graphics.set_rtv(rt);
			}

			graphics.set_pipeline<PSOS::VoxelDebug>();
			context.graph->set_slot(SlotID::VoxelInfo,  graphics);
			context.graph->set_slot(SlotID::FrameInfo,  graphics);

			{
				Slots::VoxelDebug debug;
				debug.GetVolume() = voxel_lighted.texture3D;
				gbuffer.SetTable(debug.GetGbuffer());
				graphics.set(debug);
			}
			graphics.draw(4);
		};
	}
};
