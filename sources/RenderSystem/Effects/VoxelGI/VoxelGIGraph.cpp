module;

// TEMP (voxel_vsm_diag.temp diagnostic) -- global module fragment needed for
// <fstream>/<cfloat>, same pattern as HAL.Streamline.cpp's own std::ofstream
// usage. Remove alongside the diagnostic block once the shadow-coverage
// investigation is done.
#include <fstream>
#include <cfloat>

module Graphics:VoxelGI;
import RenderSystem;


import Graphics;
import :UpscalingDLSS;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;


// ---- Texture3DMultiTiles ----------------------------------------------------

void Texture3DMultiTiles::flush(HAL::CommandList& list)
{
	tilings_info.resource = tex_result->resource.get();
	list.update_tilings(std::move(tilings_info));
}

void Texture3DMultiTiles::set(HAL::ResourceDesc desc)
{
	desc.Flags |= ResFlags::Virtual;
	tex_dynamic.reset(new HAL::Texture(RenderSystem::get().device(), desc, TextureLayout::SHADER_RESOURCE));
	tex_static.reset(new HAL::Texture(RenderSystem::get().device(), desc, TextureLayout::SHADER_RESOURCE));

	tex_result.reset(new HAL::Texture(RenderSystem::get().device(), desc, TextureLayout::SHADER_RESOURCE));

	// Allocated outside the FrameGraph's allocator, but only ever transitioned
	// by its passes -- so the graph owns their state, which is what the
	// DisableStateTracking flag used to say.
	tex_dynamic->resource->frame_graph_managed = true;
	tex_static->resource->frame_graph_managed  = true;
	tex_result->resource->frame_graph_managed  = true;

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
		if (tex_static->resource->get_tiled_manager().is_mapped(pos.xyz, 0, pos.w))
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
		if (!tex_dynamic->resource->get_tiled_manager().is_mapped(pos.xyz, 0, pos.w))
		{
			auto heap_pos = tex_static->resource->get_tiled_manager().tiles[0][pos.xyz].heap_position;
			heap_pos.handle = ResourceHandle();
			tex_result->resource->get_tiled_manager().map_tile(tilings_info, pos.xyz, heap_pos);
		}
	};

	tex_static->resource->get_tiled_manager().on_zero = [this](ivec4 pos)
	{
		if (!tex_dynamic->resource->get_tiled_manager().is_mapped(pos.xyz, 0, pos.w))
		{
			tex_result->resource->get_tiled_manager().zero_tile(tilings_info, pos.xyz, 0);
		}
	};
}

void Texture3DMultiTiles::zero_tiles(HAL::CommandList& list)
{
	tex_dynamic->resource->get_tiled_manager().zero_tiles((list));
	tex_static->resource->get_tiled_manager().zero_tiles((list));

	tilings_info.tiles.clear();
	//	flush(list);
}


// ---- Texture3DRefTiles ------------------------------------------------------

void Texture3DRefTiles::flush(HAL::CommandList& list)
{
	tilings_info.resource = tex_result->resource.get();
	list.update_tilings(std::move(tilings_info));
}

void Texture3DRefTiles::set(HAL::ResourceDesc desc)
{
	desc.Flags |= ResFlags::Virtual;
	tex_result.reset(new HAL::Texture(RenderSystem::get().device(), desc, TextureLayout::SHADER_RESOURCE));

	// See Texture3DMultiTiles::set -- FrameGraph passes own its transitions.
	tex_result->resource->frame_graph_managed = true;

	static_tiles.resize(tex_result->resource->get_tiled_manager().get_tiles_count(), 0);
	dynamic_tiles.resize(tex_result->resource->get_tiled_manager().get_tiles_count(), 0);
}

void Texture3DRefTiles::load_static(std::list<uint3>& tiles)
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

void Texture3DRefTiles::zero_static(std::list<uint3>& tiles)
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

void Texture3DRefTiles::load_dynamic(std::list<uint3>& tiles)
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

void Texture3DRefTiles::zero_dynamic(std::list<uint3>& tiles)
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

void Texture3DRefTiles::zero_tiles(HAL::CommandList& list)
{
	tilings_info.tiles.clear();

	tex_result->resource->get_tiled_manager().zero_tiles(list);

	static_tiles.fill(false);
	dynamic_tiles.fill(false);
}


// ---- VoxelGI::pass_data -----------------------------------------------------

void VoxelGI::pass_data(FrameGraph::TaskBuilder& builder)
{
	// Always register external textures with the frame graph
	builder.pass_texture(FrameGraph::ResourceID::VoxelAlbedo,        albedo.tex_result->resource);
	builder.pass_texture(FrameGraph::ResourceID::VoxelNormal,        normal.tex_result->resource);
	builder.pass_texture(FrameGraph::ResourceID::VoxelLighted,       tex_lighting.tex_result->resource);
	builder.pass_texture(FrameGraph::ResourceID::VoxelAlbedoStatic,  albedo.tex_static->resource);
	builder.pass_texture(FrameGraph::ResourceID::VoxelNormalStatic,  normal.tex_static->resource);
	builder.pass_texture(FrameGraph::ResourceID::VoxelAlbedoDynamic, albedo.tex_dynamic->resource);
	builder.pass_texture(FrameGraph::ResourceID::VoxelNormalDynamic, normal.tex_dynamic->resource);
}


// [Static] (see voxel.sig) -- this used to be a runtime-wired
// add_library_pass with no call site left anywhere assigning it into a
// pipeline (dead: never ran, before or after the compute rewrite). It is
// fully stateless, so PassDefault<Passes::GBufferDownsampler> + a
// MainPipeline listing (test.sig) is the right shape, matching IndirectRTX.
bool PassDefault<Passes::GBufferDownsampler>::setup(
	Passes::GBufferDownsampler::Context& data, FrameGraph::TaskBuilder& builder)
{

	return true;
}

namespace
{
	// Free-standing (GBufferDownsampler is a stateless [Static] PassDefault,
	// not a VariableContext-derived object) -- Meyer's-singleton VariableContext
	// so these attach under a stable, named GUI node regardless of static
	// init order across translation units (see VariableContext::create's own
	// comment for why this is the sanctioned way to get one outside a class
	// that derives it).
	VariableContext& tile_classify_context()
	{
		static auto ctx = VariableContext::create(L"Tile Classify");
		return *ctx;
	}

	// Pure eyeball-tuned values -- see TileClassifyData's own comment
	// (pssm.sig) for what they gate.
	Variable<float> g_roughness_threshold = { 0.5f,  "Reflection roughness threshold", &tile_classify_context(), 0.0f, 1.0f };
	Variable<float> g_metallic_threshold  = { 0.05f, "Reflection metallic threshold",  &tile_classify_context(), 0.0f, 1.0f };
}

void PassDefault<Passes::GBufferDownsampler>::render(
	Passes::GBufferDownsampler::Context& data, FrameGraph::FrameContext& context)
{
	auto& command_list = context.get_list();
	auto tempColor = *data.GBuffer_TempColor;
	GBuffer gbuffer = GBufferViewDesc::actualize(data);
	auto& graphics = command_list->get_graphics();
	auto& compute  = command_list->get_compute();

	graphics.set_signature(Layouts::DefaultLayout);

	context.graph->set_slot(SlotID::FrameInfo, graphics);

	//MipMapGenerator::get().generate_quality(graphics, nullptr, gbuffer, tempColor);

	{
		PROFILE_GPU(L"gbuffer_tile_classify");

		compute.set_signature(Layouts::DefaultLayout);
		// Every root-signature change invalidates prior root arguments --
		// FrameInfo must be re-bound even though this shader doesn't sample
		// it, or an unbound CBV slot trips GPU-based validation #939 (see
		// FrameClassification's own render() for the same lesson learned
		// the hard way).
		context.graph->set_slot(SlotID::FrameInfo, compute);
		compute.clear_counter(*data.TileClassifyHi);
		compute.clear_counter(*data.TileClassifyLow);
		compute.clear_counter(*data.TileRoughnessHi);
		compute.clear_counter(*data.TileRoughnessLow);

		{
			Slots::TileClassifyData params;
			gbuffer.SetTable(params.GetGbuffer());
			params.GetHalf_depth()   = data.GBuffer_HalfDepth->rwTexture2D;
			params.GetHalf_normals() = data.GBuffer_HalfNormals->rwTexture2D;
			params.GetTile_hi()      = data.TileClassifyHi->appendStructuredBuffer;
			params.GetTile_low()     = data.TileClassifyLow->appendStructuredBuffer;
			params.GetTile_mask()    = data.TileClassifyMask->rwTexture2D;
			params.GetTile_flags()   = data.TileClassifyTiles->rwTexture2D;

			params.GetTile_roughness_hi()    = data.TileRoughnessHi->appendStructuredBuffer;
			params.GetTile_roughness_low()   = data.TileRoughnessLow->appendStructuredBuffer;
			params.GetTile_roughness_flags() = data.TileRoughnessTiles->rwTexture2D;
			params.GetRoughness_threshold()  = g_roughness_threshold;
			params.GetMetallic_threshold()   = g_metallic_threshold;
			compute.set(params);
		}

		compute.set_pipeline<PSOS::GBufferDownsample>();
		compute.dispatch(context.graph->get_context<ViewportInfo>().frame_size, ivec2{ 8, 8 });
	}
}



VoxelGI::VoxelGI(Scene::ptr& scene, VSM& vsm) :scene(scene), vsm(vsm), VariableContext(L"VoxelGI")
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
			gpu_tiles_buffer[i]->set_size(tex_lighting.tex_result->resource->get_tiled_manager().get_tiles_count(0, i), tex_lighting.tex_result->resource->get_tiled_manager().get_tile_shape());
		}


	}

	// ---- Voxelize -------------------------------------------------------

	m_voxelize_setup = [this](Passes::Voxelize::Context& data, FrameGraph::TaskBuilder& builder) -> bool
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
			tex_lighting.tex_result->resource->get_tiled_manager().get_tiles_count();
		voxel_info.GetVoxels_per_tile().xyz =
			tex_lighting.tex_result->resource->get_tiled_manager().get_tile_shape();

		this->scene->voxels_compiled =
			this->scene->voxel_info.compile(*builder.graph->builder.current_frame);
		builder.graph->register_slot_setter(this->scene->voxels_compiled);

		if (!voxelize_scene) return false;

		return true;
	};

	m_voxelize_render = [this](Passes::Voxelize::Context& data, FrameGraph::FrameContext& context)
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

	m_lighting_setup = [this](Passes::Lighting::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		light_counter = (light_counter + 1) % 5;
		if (!light_scene) return false;

		return true;
	};

	m_lighting_render = [this](Passes::Lighting::Context& data, FrameGraph::FrameContext& context)
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

			auto& vsm_lookup = ligthing.GetVsm();
			// this-> needed: the ctor parameter also named "vsm" shadows the
			// member of the same name within this lexical scope, and a
			// [this]-only lambda can't implicitly capture a local parameter.
			this->vsm.fill_shadow_lookup_constants(vsm_lookup, cam.cam->position);
			vsm_lookup.GetVsm_atlas()    = data.VSM_Atlas->texture2DArray;
			vsm_lookup.GetPage_table()   = data.VSM_PageTable->texture2DArray;
			vsm_lookup.GetPage_cameras() = data.VSM_PageCameras->structuredBuffer;

			compute.set(ligthing);

			// TEMP diagnostic (voxel_vsm_diag.temp) -- checking whether VSM's
			// active window actually covers the (fixed, whole-scene-bounds)
			// voxel GI volume, since "no shadows" after switching Lighting
			// from PSSM_Global to VSM would be exactly what a coverage gap
			// looks like. Logged once.
			{
				static bool logged_once = false;
				if (!logged_once)
				{
					logged_once = true;
					float3 corners[8];
					for (int i = 0; i < 8; i++)
						corners[i] = this->min + this->size * float3((i & 1) ? 1.f : 0.f, (i & 2) ? 1.f : 0.f, (i & 4) ? 1.f : 0.f);

					float2 ls_min = float2( FLT_MAX,  FLT_MAX);
					float2 ls_max = float2(-FLT_MAX, -FLT_MAX);
					for (int i = 0; i < 8; i++)
					{
						float2 ls = (float4(corners[i], 1) * vsm_lookup.GetLight_view()).xy;
						ls_min = float2(std::min(ls_min.x, ls.x), std::min(ls_min.y, ls.y));
						ls_max = float2(std::max(ls_max.x, ls.x), std::max(ls_max.y, ls.y));
					}

					std::ofstream f("voxel_vsm_diag.temp", std::ios::app);
					f << "voxel bounds: min=(" << this->min.x << "," << this->min.y << "," << this->min.z
					  << ") size=(" << this->size.x << "," << this->size.y << "," << this->size.z << ")\n";
					f << "voxel bounds in light-space XY: min=(" << ls_min.x << "," << ls_min.y
					  << ") max=(" << ls_max.x << "," << ls_max.y << ")\n";
					f << "active_min=" << vsm_lookup.GetActive_min() << " active_max=" << vsm_lookup.GetActive_max() << "\n";
					for (int level = vsm_lookup.GetActive_min(); level <= vsm_lookup.GetActive_max(); level++)
					{
						float4 info = vsm_lookup.GetLevel_info()[level];
						float extent = info.z * vsm_lookup.GetPages_per_level();
						f << "  level " << level << ": origin=(" << info.x << "," << info.y
						  << ") extent=" << extent << " covers=["
						  << info.x << "," << (info.x + extent) << "]x[" << info.y << "," << (info.y + extent) << "]\n";
					}
					f.close();
				}
			}
		}

		context.graph->set_slot(SlotID::VoxelInfo, compute);
		context.graph->set_slot(SlotID::FrameInfo, compute);
		compute.exec_indirect(gpu_tiles_buffer[0]->dispatch_buffer, 1);
	};

	// ---- Mipmapping -----------------------------------------------------

	m_mipmapping_setup = [this](Passes::Mipmapping::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		if (!light_scene) return false;
		return true;
	};

	m_mipmapping_render = [this](Passes::Mipmapping::Context& data, FrameGraph::FrameContext& context)
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

		const uint mip_levels = tex_lighting.tex_result->get_desc().as_texture().MipLevels;

		{
			// Only the tail mips without a tile list (packed) still need a
			// clear — they are never downsampled and the cone trace samples
			// them at high LODs. Tiny textures, cost is negligible.
			PROFILE_GPU(L"clear tail mips");
			for (uint i = 1; i < mip_levels; i++)
				if (i >= gpu_tiles_buffer.size() || !gpu_tiles_buffer[i])
					list.clear_uav(tex_lighting.tex_result->texture_3d().mips[i].rwTexture3D,
						vec4(0, 0, 0, 0));
		}

		{
			// One dispatch per mip over that mip's OWN tile list: every texel
			// of every loaded tile is written, and unmapped source tiles read
			// as 0 — so the old per-tile VoxelZero pre-pass is unnecessary.
			// (The old 3-mips-per-dispatch batches covered mips 2,3,5,6 only
			// under loaded finer tiles, which is what the zero pass patched.)
			PROFILE_GPU(L"EXEC");
			compute.set_pipeline<PSOS::VoxelDownsample>(PSOS::VoxelDownsample::Count(1));

			for (uint mip = 1; mip < mip_levels; mip++)
			{
				if (mip >= gpu_tiles_buffer.size() || !gpu_tiles_buffer[mip]) break;

				{
					Slots::VoxelMipMap mipmapping;
					mipmapping.GetSrcMip() =
						tex_lighting.tex_result->texture_3d().mips[mip - 1].texture3D;
					mipmapping.GetOutMips()[0] =
						tex_lighting.tex_result->texture_3d().mips[mip].rwTexture3D;
					auto& params = mipmapping.GetParams();
					params.GetTiles() = gpu_tiles_buffer[mip]->buffer;
					params.GetVoxels_per_tile().xyz =
						tex_lighting.tex_result->resource->get_tiled_manager().get_tile_shape();
					compute.set(mipmapping);
				}
				static constexpr LiteralWStr mip_names[] = {
					L"mip_0",  L"mip_1",  L"mip_2",  L"mip_3",
					L"mip_4",  L"mip_5",  L"mip_6",  L"mip_7",
					L"mip_8",  L"mip_9",  L"mip_10", L"mip_11",
					L"mip_12", L"mip_13", L"mip_14", L"mip_15",
				};
				PROFILE_GPU(mip_names[mip < 16 ? mip : 0]);
				compute.exec_indirect(gpu_tiles_buffer[mip]->dispatch_buffer, 1);
			}
		}
	};

	// ---- NormalRoughnessRepack -------------------------------------------
	// Decodes GBuffer_Normals' best-fit-compressed normal into a Streamline-
	// compatible unpacked buffer, and derives SpecularAlbedo (F0) from
	// GBuffer_Albedo in the same dispatch. Both feed UpscalingDLSSRR's
	// evaluate() call (see UpscalingDLSSRR.cpp / HAL.DLSSRR.cpp). Both are
	// pure GBuffer-derived material properties, not reflection-specific, so
	// this doesn't gate on `reflecton` -- only on DLSS-RR being the user's
	// selected upscaler (g_upscaler_type) and actually available.

	m_normalroughnessrepack_setup = [this](Passes::NormalRoughnessRepack::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		if (g_upscaler_type != UpscalerType::DLSSRR || !nvidia::DLSSRR::get().available()) return false;

		return true;
	};

	m_normalroughnessrepack_render = [this](Passes::NormalRoughnessRepack::Context& data, FrameGraph::FrameContext& context)
	{
		auto& compute = context.get_list()->get_compute();
		auto  sz      = context.graph->get_context<ViewportInfo>().frame_size;

		compute.set_pipeline<PSOS::NormalRoughnessRepack>();
		Slots::NormalRoughnessRepackParams params;
		params.GBuffer_Normals        = data.GBuffer_Normals->texture2D;
		params.GBuffer_Albedo         = data.GBuffer_Albedo->texture2D;
		params.Output                 = data.NormalRoughness->rwTexture2D;
		params.SpecularAlbedoOutput   = data.SpecularAlbedo->rwTexture2D;
		compute.set(params);
		// dispatch(a, b) takes the full pixel size and divides by the group
		// size itself (ceil(a/b)) -- passing an already-divided group count
		// as a single-arg call left it dividing by the DEFAULT group size
		// (4,4,4) a second time instead of matching this shader's actual
		// [numthreads(8,8,1)], covering only a fraction of the screen.
		compute.dispatch(uint3(sz, 1), uint3(8, 8, 1));
	};

	// ---- ReflCombine ----------------------------------------------------

	m_reflcombine_setup = [this](Passes::ReflCombine::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		// RTXCombine (voxel.sig) takes over this job -- reflections plus
		// indirect GI plus shadow, all three -- whenever the user has picked
		// DLSS-RR via g_upscaler_type; same gate as its own setup, kept in
		// lockstep here. NRD REBLUR_SPECULAR is the only reflection denoiser
		// now (see [[project-nrd-integration]]) -- always used when this
		// pass runs at all.
		if (!reflecton ||
		    (g_upscaler_type == UpscalerType::DLSSRR &&
		     RenderSystem::get().device().is_rtx_supported() && nvidia::DLSSRR::get().available()))
			return false;

		return true;
	};

	m_reflcombine_render = [this](Passes::ReflCombine::Context& data, FrameGraph::FrameContext& context)
	{
		auto& command_list = context.get_list();
		auto  target_tex   = *data.ResultTexture;
		GBuffer gbuffer    = GBufferViewDesc::actualize(data);
		auto  sz           = target_tex.get_size();
		auto& compute      = command_list->get_compute();

		context.graph->set_slot(SlotID::FrameInfo, compute);
		context.graph->set_slot(SlotID::SceneData, compute);

		compute.set_pipeline<PSOS::ReflectionCombine>();

		{
			Slots::ReflectionCombine combine;
			gbuffer.SetTable(combine.GetGbuffer());
			combine.GetReflection() = data.RTXReflectionDenoised->texture2D;
			combine.GetTarget()     = data.ResultTexture->rwTexture2D;
			compute.set(combine);
		}

		context.graph->set_slot(SlotID::FrameInfo, compute);
		compute.dispatch(sz);
	};

	// ---- VoxelDebug -----------------------------------------------------

	m_voxeldebug_setup = [this](Passes::VoxelDebug::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		// The only remaining consumer of the 3D voxel-lighting volume (see
		// [[project-nrd-integration]] -- the old voxel-cone-traced indirect/
		// reflection denoiser that used to also read it is gone). Gating
		// this on the debug view actually being selected, rather than
		// running unconditionally, is what lets Voxelize/Lighting/Mipmapping
		// go idle automatically the rest of the time -- their own need()s
		// only fire because this pass needs their output.
		if (builder.graph->get_context<FrameGraph::DebugContext>().mode != FrameGraph::DebugMode::VoxelTrace)
			return false;

		return true;
	};

	m_voxeldebug_render = [this](Passes::VoxelDebug::Context& data, FrameGraph::FrameContext& context)
	{
		auto& command_list = context.get_list();
		auto  voxel_lighted = *data.VoxelLighted;
		auto& caminfo   = context.graph->get_context<CameraInfo>();
		auto& sceneinfo = context.graph->get_context<SceneInfo>();

		MeshRenderContext::ptr mesh_ctx(new MeshRenderContext());
		auto  target_tex = *data.VoxelDebug;
		GBuffer gbuffer  = GBufferViewDesc::actualize(data);

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
		context.graph->set_slot(SlotID::VoxelInfo, graphics);
		context.graph->set_slot(SlotID::FrameInfo, graphics);

		{
			Slots::VoxelDebug debug;
			debug.GetVolume() = voxel_lighted.texture3D;
			gbuffer.SetTable(debug.GetGbuffer());
			graphics.set(debug);
		}
		graphics.draw(4);
	};

	// ---- VoxelScreen (voxel-cone-traced indirect GI, NRD source) --------

	m_voxelscreen_setup = [this](Passes::VoxelScreen::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		// Alternative to IndirectRTX as NRD REBLUR_DIFFUSE's input, selected
		// via g_indirect_source (see [[project-nrd-integration]]). Only runs
		// when actually selected -- this is a full RTX-primary +
		// cone-trace-fallback dispatch, not free, and its only consumer
		// (NRD_GBufferPack -> NRD_REBLUR_Execute) is itself off under
		// DLSS-RR (see NRD_GBufferPack's own comment). Skipping the dispatch
		// here, not just skipping the pack of its output downstream, is
		// what actually avoids the wasted GPU work -- a downstream need()
		// gate alone would still force this pass to run and be discarded.
		if (builder.graph->get_context<Table::IndirectGISelectors>().indirect_source != IndirectSource::MyVCT ||
		    g_upscaler_type == UpscalerType::DLSSRR ||
		    !RenderSystem::get().device().is_rtx_supported() || !nvidia::DLSSRR::get().available())
			return false;

		return true;
	};

	m_voxelscreen_render = [this](Passes::VoxelScreen::Context& data, FrameGraph::FrameContext& context)
	{
		auto& command_list = context.get_list();

		GBuffer gbuffer    = GBufferViewDesc::actualize(data);
		auto noisy_output  = *data.VoxelIndirectNoiseRaw;
		auto voxel_lighted = *data.VoxelLighted;

		auto& sceneinfo = context.graph->get_context<SceneInfo>();

		command_list->get_compute().set_signature(RTX::get().rtx.m_root_sig);

		auto& compute = command_list->get_compute();

		context.graph->set_slot(SlotID::FrameInfo, compute);
		context.graph->set_slot(SlotID::SceneData, compute);
		context.graph->set_slot(SlotID::VoxelInfo, compute);

		{
			Slots::VoxelScreen voxelScreen;
			gbuffer.SetTable(voxelScreen.GetGbuffer());
			voxelScreen.GetVoxels() = voxel_lighted.texture3D;
			compute.set(voxelScreen);
		}

		{
			PROFILE_GPU(L"voxel_screen_indirect");
			{
				Slots::VoxelOutput output;
				output.GetNoiseRaw()  = noisy_output.rwTexture2D;
				output.GetBlueNoise() = data.BlueNoise->texture2D;
				compute.set(output);
			}
			RTX::get().render<Indirect>(compute, sceneinfo.scene->raytrace_scene, noisy_output.get_size());
		}
	};

	// ---- ScreenReflection (voxel-cone-traced reflection, NRD source) ----

	m_screenreflection_setup = [this](Passes::ScreenReflection::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		// Alternative to ReflectionRTX as NRD REBLUR_SPECULAR's input,
		// selected via g_reflection_source (see [[project-nrd-integration]]).
		// See VoxelScreen's own comment on why this is gated on actual
		// selection now, not just "RTX pipeline viable".
		if (builder.graph->get_context<Table::IndirectGISelectors>().reflection_source != ReflectionSource::MyReflection ||
		    g_upscaler_type == UpscalerType::DLSSRR ||
		    !RenderSystem::get().device().is_rtx_supported() || !nvidia::DLSSRR::get().available())
			return false;

		return true;
	};

	m_screenreflection_render = [this](Passes::ScreenReflection::Context& data, FrameGraph::FrameContext& context)
	{
		auto& command_list = context.get_list();

		GBuffer gbuffer    = GBufferViewDesc::actualize(data);
		auto noisy_output  = *data.VoxelReflectionNoiseRaw;
		auto voxel_lighted = *data.VoxelLighted;

		auto& sceneinfo = context.graph->get_context<SceneInfo>();

		command_list->get_compute().set_signature(RTX::get().rtx.m_root_sig);

		auto& compute = command_list->get_compute();

		context.graph->set_slot(SlotID::FrameInfo, compute);
		context.graph->set_slot(SlotID::SceneData, compute);
		context.graph->set_slot(SlotID::VoxelInfo, compute);

		{
			Slots::VoxelScreen voxelScreen;
			gbuffer.SetTable(voxelScreen.GetGbuffer());
			voxelScreen.GetVoxels() = voxel_lighted.texture3D;
			compute.set(voxelScreen);
		}

		{
			PROFILE_GPU(L"screen_reflection");
			{
				Slots::VoxelOutput output;
				output.GetNoiseRaw()  = noisy_output.rwTexture2D;
				output.GetBlueNoise() = data.BlueNoise->texture2D;
				compute.set(output);
			}
			RTX::get().render<Reflection>(compute, sceneinfo.scene->raytrace_scene, noisy_output.get_size());
		}
	};

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
