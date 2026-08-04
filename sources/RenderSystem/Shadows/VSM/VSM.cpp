module Graphics:VSM;

import :VSM;
import :BRDF;
import :EngineAssets;
import :FrameGraphContext;

import HAL;

import Graphics;
using namespace FrameGraph;
using namespace HAL;

float3 VSM::get_position()
{
	pos_mutex.lock();
	auto p = position;
	pos_mutex.unlock();
	return p;
}

void VSM::set_position(float3 p)
{
	pos_mutex.lock();
	bool changed = position.x != p.x || position.y != p.y || position.z != p.z;
	position = p;
	if (changed)
		light_change_pending.fill(true);
	pos_mutex.unlock();
}

namespace
{
	// Shared by every page's render() and by combine(): the rotation-only
	// light-space view used purely to classify "which page/level is this
	// world position in", built the same way PSSM builds its shadow camera.
	camera make_light_view_camera(float3 position)
	{
		camera light_cam;
		light_cam.set_projection_params(0, 1, 0, 1, 1, 1000);
		light_cam.position = position;
		light_cam.up = (float3(0.01, 1, 0.023)).normalize();
		light_cam.update();
		return light_cam;
	}
}

void VSM::attach_scene(std::shared_ptr<Scene> scene)
{
	// Per-page invalidation (Phase 2): map a touched object's world AABB to
	// the (level, page) bits it overlaps, via the same light-view-space
	// projection/clamp math VSMClipmap already uses for page selection --
	// world_to_page's edge-clamp means an object slightly outside a coarse
	// level's grid still marks that level's nearest edge page (conservative,
	// not a correctness bug).
	tracker.attach(scene, [this](scene_object* object)
	{
		vec3 wmin = object->get_min();
		vec3 wmax = object->get_max();

		camera light_view_cam = make_light_view_camera(get_position());
		box bounds_ls = light_view_cam.get_points(wmin, wmax).get_bounds_in(light_view_cam.get_view());
		float2 ls_min(bounds_ls.left, bounds_ls.top);
		float2 ls_max(bounds_ls.right, bounds_ls.bottom);

		for (int level = 0; level < page_table.clipmap.level_count; level++)
		{
			// A level that hasn't rendered yet has no meaningful
			// cached_origin -- it'll get a full-mask render (recentered)
			// the first time it does run, so there's nothing useful to
			// mark here yet.
			if (!level_initialized[level])
				continue;

			ivec2 p0 = page_table.clipmap.world_to_page(level, ls_min, cached_origin[level]);
			ivec2 p1 = page_table.clipmap.world_to_page(level, ls_max, cached_origin[level]);

			uint32_t mask = 0;
			for (int py = p0.y; py <= p1.y; py++)
				for (int px = p0.x; px <= p1.x; px++)
					mask |= 1u << (py * page_table.clipmap.pages_per_level + px);

			tracker.mark_pages(level, mask);
		}
	});
}

VSM::VSM()
{
	position = float3(200, 400, 200);

	page_table.clipmap.level_count = 3;
	page_table.clipmap.pages_per_level = 4;
	// Coarsest level spans pages_per_level * base_page_world_size * 2^(level_count-1)
	// world units, centered on the camera -- with 3 levels / 4 pages and base=32
	// that's 512u total (256u radius). Tune to taste; too small and geometry past
	// the edge of the coarsest level clamps into an unrelated page (looks like
	// shadows vanishing with distance -- see also the out-of-range check in
	// get_shadow_vsm, which now returns unshadowed instead of clamp-sampling).
	page_table.clipmap.base_page_world_size = 32.0f;
	// pages_per_level is pinned at 4 (4x4=16), exactly the D3D12 16-viewport
	// cap the single-draw-per-level batching depends on -- so page_size
	// (texels/page) is the resolution lever, not page count. Atlas is
	// atlas_pages_per_side * page_size texels square; 7*512=3584, well
	// under the 16384 D3D12 max texture dimension, so there's headroom to
	// go higher (1024 -> 7168) if this still isn't sharp enough.
	page_table.page_size = 512;
	page_table.atlas_pages_per_side = 7;

	const int pages_side       = page_table.clipmap.pages_per_level;
	const int pages_per_level  = pages_side * pages_side;

	// One Hi-Z pyramid slice per physical atlas slot, mip chain down to an
	// 8x8 floor (1x1 wastes dispatches/VRAM for no real occluder benefit).
	const int physical_slots = page_table.atlas_pages_per_side * page_table.atlas_pages_per_side;
	int pyramid_mip_count = 1;
	for (int s = page_table.page_size; s > 8; s >>= 1)
		pyramid_mip_count++;

	// ---- Page rendering (one pass per clipmap LEVEL, all of that level's
	// pages_per_level^2 pages rendered in a single mesh-shader dispatch via
	// SV_ViewportArrayIndex -- see mesh_shader_vsm.hlsl) -----------------------

	for (int level = 0; level < page_table.clipmap.level_count; level++)
	{
		m_level_setup[level] = [this, level, physical_slots, pyramid_mip_count](Passes::VSM_RenderPage::Context& data, FrameGraph::TaskBuilder& builder) -> bool
		{
			int atlas_size = page_table.atlas_size_texels();

			if (level == 0)
			{
				// Static: Phase 2 caching relies on this atlas (and the page
				// table / page cameras) surviving across frames -- a level
				// that isn't dirty this frame does nothing and relies on its
				// pages still holding last frame's content.
				builder.create(data.VSM_Atlas, { ivec3(atlas_size, atlas_size, 0), HAL::Format::R32_TYPELESS, 1, 1 }, FrameGraph::ResourceFlags::DepthStencil | FrameGraph::ResourceFlags::Static);
				builder.create(data.VSM_PageTable, { ivec3(page_table.clipmap.pages_per_level, page_table.clipmap.pages_per_level, 0), HAL::Format::R32_UINT, (UINT)page_table.clipmap.level_count, 1 }, FrameGraph::ResourceFlags::CopyDest | FrameGraph::ResourceFlags::Static);
				builder.create(data.VSM_PageCameras, { (size_t)MaxPages }, FrameGraph::ResourceFlags::CopyDest | FrameGraph::ResourceFlags::Static);
				// Static like VSM_Atlas: must survive until this page is next dirty.
				builder.create(data.VSM_PageHiZ, { ivec3(page_table.page_size, page_table.page_size, 0), HAL::Format::R32_FLOAT, (UINT)physical_slots, (UINT)pyramid_mip_count }, FrameGraph::ResourceFlags::UnorderedAccess | FrameGraph::ResourceFlags::Static);
			}
			else
			{
				builder.need(data.VSM_Atlas, FrameGraph::ResourceFlags::DepthStencil);
				builder.need(data.VSM_PageTable, FrameGraph::ResourceFlags::CopyDest);
				builder.need(data.VSM_PageCameras, FrameGraph::ResourceFlags::CopyDest);
				builder.need(data.VSM_PageHiZ, FrameGraph::ResourceFlags::UnorderedAccess);
			}
			return true;
		};

		m_level_render[level] = [this, level, pages_side, pages_per_level, pyramid_mip_count](Passes::VSM_RenderPage::Context& data, FrameGraph::FrameContext& context)
		{
			auto& sceneinfo = context.graph->get_context<SceneInfo>();
			auto& caminfo   = context.graph->get_context<CameraInfo>();
			auto  cam       = caminfo.cam;
			auto  scene     = sceneinfo.scene;

			// Phase 2 caching: only pay for a re-render when something
			// actually requires it -- first time this level is ever
			// rendered, the clipmap grid recentered (crossed a page
			// boundary, so the local page indices now mean a different
			// world region than what's cached there), or any scene change
			// happened (coarse whole-VSM invalidation, see
			// VSMInvalidationTracker). Otherwise this level's pages keep
			// exactly what they held last frame -- zero GPU work.
			float3 current_light_pos = get_position();
			camera light_view_cam = make_light_view_camera(current_light_pos);
			float2 cam_pos_ls = (float4(cam->position, 1) * light_view_cam.get_view()).xy;
			float2 origin     = page_table.clipmap.grid_origin(level, cam_pos_ls);

			bool recentered = !level_initialized[level] || origin.x != cached_origin[level].x || origin.y != cached_origin[level].y;

			// Read-and-clear this level's sticky flag (see VSM.ixx).
			pos_mutex.lock();
			bool light_moved = light_change_pending[level];
			light_change_pending[level] = false;
			pos_mutex.unlock();

			// recentered/light_moved invalidate every page of this level;
			// otherwise only the pages VSMInvalidationTracker actually
			// marked (from scene events, mapped to pages via world_to_page
			// in attach_scene's callback) need a refresh. Always drain the
			// tracker's mask (even when unused below) so stale bits from a
			// recenter/light-move frame don't linger into the next one.
			uint32_t all_pages_mask = (1u << pages_per_level) - 1;
			uint32_t scene_mask     = tracker.take_dirty(level) & all_pages_mask;
			uint32_t dirty_mask     = (recentered || light_moved) ? all_pages_mask : scene_mask;

			if (dirty_mask == 0)
				return;

			level_initialized[level] = true;
			cached_origin[level]     = origin;

			auto& command_list = context.get_list();
			auto& graphics      = command_list->get_graphics();
			auto& compute       = command_list->get_compute();

			graphics.set_signature(Layouts::DefaultLayout);
			compute.set_signature(Layouts::DefaultLayout);

			auto min = scene->get_min();
			auto max = scene->get_max();
			auto points_all = cam->get_points(min, max);
			auto bounds_all = points_all.get_bounds_in(light_view_cam.get_view());

			int base_slot = level * pages_per_level;
			std::vector<HAL::Viewport> viewports(pages_per_level);
			std::vector<sizer_long> scissors(pages_per_level);

			for (int local = 0; local < pages_per_level; local++)
			{
				ivec2 page = ivec2(local % pages_side, local / pages_side);
				float2 page_min = page_table.clipmap.page_min(level, page, origin);
				float2 page_max = page_min + float2(page_table.clipmap.page_world_size(level));

				camera page_cam = make_light_view_camera(get_position());
				page_cam.set_projection_params(
					page_min.x, page_max.x,
					page_min.y, page_max.y,
					bounds_all.znear - 10, bounds_all.zfar);
				page_cam.update();

				int slot = base_slot + local;
				command_list->get_copy().update(*data.VSM_PageCameras, slot, std::span{ &page_cam.camera_cb.current, 1 });

				ivec2 atlas_origin = page_table.atlas_slot_origin(slot);
				viewports[local].pos    = float2((float)atlas_origin.x, (float)atlas_origin.y);
				viewports[local].size   = float2((float)page_table.page_size, (float)page_table.page_size);
				viewports[local].depths = float2(0, 1);

				// D3D12 always requires NumScissorRects == NumViewports (no
				// rasterizer ScissorEnable toggle exists in D3D12) -- match each
				// viewport's own bounds exactly so nothing gets clipped beyond
				// what the viewport already constrains.
				scissors[local] = sizer_long{
					atlas_origin.x, atlas_origin.y,
					atlas_origin.x + page_table.page_size, atlas_origin.y + page_table.page_size };
			}

			if (level == 0)
			{
				std::vector<uint32_t> indirection = page_table.build_indirection();
				UINT row_stride = (UINT)(pages_side * sizeof(uint32_t));
				for (int lvl = 0; lvl < page_table.clipmap.level_count; lvl++)
				{
					command_list->get_copy().update_texture(
						(*data.VSM_PageTable).resource, ivec3(0, 0, 0), ivec3(pages_side, pages_side, 1), (UINT)lvl,
						reinterpret_cast<const char*>(indirection.data() + lvl * pages_per_level),
						row_stride);
				}

				// Fresh/resized pyramid holds garbage -- clear to far (0,
				// reversed-Z) so a page's occlusion test can't falsely cull
				// against it before that page has ever rendered real depth.
				// One array-spanning UAV clear per mip (all physical slots
				// at once) -- only runs once ever (cold start / resize).
				if (data.VSM_PageHiZ.is_new())
				{
					for (int mip = 0; mip < pyramid_mip_count; mip++)
						command_list->clear_uav(data.VSM_PageHiZ->create_mip(mip, *command_list).rwTexture2DArray, vec4(0, 0, 0, 0));
				}
			}

			{
				RT::DepthOnly rt;
				rt.GetDepth() = data.VSM_Atlas->depthStencil;
				// No RTOptions::ClearDepth here: VSM_Atlas is shared and
				// Static across all levels now (Phase 2 caching), so a
				// whole-resource clear would wipe out other levels' cached
				// pages. Clear only the pages actually being re-rendered
				// this frame (dirty_mask) -- a page kept cached must NOT be
				// cleared, or its surviving depth data would be wiped even
				// though the AS skips rendering into it.
				std::vector<sizer_long> dirty_scissors;
				dirty_scissors.reserve(pages_per_level);
				for (int local = 0; local < pages_per_level; local++)
				{
					if ((dirty_mask >> local) & 1)
						dirty_scissors.push_back(scissors[local]);
				}

				command_list->get_graphics().set_rtv(rt, RTOptions::Default);
				graphics.clear_depth_rects(dirty_scissors, 0.0f);
			}

			graphics.set_viewports(viewports);
			graphics.set_scissors(scissors);
			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::LIST);
			graphics.set_pipeline<PSOS::VSMDepthDraw>();

			{
				Slots::VSMPageTableData pageTableData;
				pageTableData.GetPage_table()   = data.VSM_PageTable->texture2DArray;
				pageTableData.GetPage_cameras() = data.VSM_PageCameras->structuredBuffer;
				graphics.set(pageTableData);
			}
			{
				Slots::VSMPageBatch batch;
				batch.GetPage_base_slot()  = base_slot;
				batch.GetDirty_mask()      = (int)dirty_mask;
				// Stale-pyramid case -- see the field's comment in vsm.sig.
				batch.GetSkip_occlusion()  = (recentered || light_moved) ? 1 : 0;
				graphics.set(batch);
			}

			// No manual transition needed: like VSM_PageTable, graphics.set()
			// tracks the read state for this SRV bind itself.
			{
				Slots::VSMPageHiZ pageHiZ;
				pageHiZ.GetPage_hiz() = data.VSM_PageHiZ->texture2DArray;
				graphics.set(pageHiZ);
			}

			graphics.set(scene->compiledScene);

			scene->iterate_meshes(MESH_TYPE::STATIC | MESH_TYPE::DYNAMIC, [&](scene_object::ptr obj)
			{
				auto mesh = dynamic_cast<MeshAssetInstance*>(obj.get());
				if (!mesh) return;

				for (auto& m : mesh->rendering)
				{
					graphics.set(m.compiled_mesh_info);
					graphics.set(m.mesh_instance_info);
					// Dispatch AS threadgroups, not MS ones directly: 32
					// (meshlet, page) pairs tested per group, AS compacts
					// visible pairs into a payload and calls DispatchMesh
					// itself (see mesh_shader_vsm.hlsl). Count must cover
					// meshlet_count * pages_per_level total pairs.
					UINT pair_count = (UINT)m.meshlet_count * (UINT)pages_per_level;
					UINT as_groups  = (pair_count + 31) / 32;
					graphics.dispatch_mesh(ivec3{ (int)as_groups, 1, 1 });
				}
			});

			// Rebuild each just-redrawn page's pyramid for next time it's dirty.
			// VSM_Atlas was just a DSV -- nudge readable for the copy shader
			// below, same one-off idiom PSSM.cpp uses; the next draw into it
			// transitions it back automatically via set_rtv.
			command_list->transition(data.VSM_Atlas->resource, HAL::ResourceStates::SHADER_RESOURCE);

			for (int local = 0; local < pages_per_level; local++)
			{
				if (!((dirty_mask >> local) & 1))
					continue;

				int slot = base_slot + local;
				ivec2 atlas_origin = page_table.atlas_slot_origin(slot);
				auto slice = data.VSM_PageHiZ->create_2d_slice(slot, *command_list);

				compute.set_pipeline<PSOS::VSMCopyPageDepth>();
				{
					Slots::VSMCopyPageDepth copy;
					copy.GetAtlas()        = data.VSM_Atlas->texture2D;
					copy.GetAtlas_origin() = atlas_origin;
					copy.GetDst_mip0()     = slice.create_mip(0, *command_list).rwTexture2D;
					compute.set(copy);
				}
				compute.dispatch(ivec2(page_table.page_size, page_table.page_size), ivec2(8, 8));

				MipMapGenerator::get().build_hiz_pyramid(compute, slice);

				// Every mip but the last is later read as another mip's
				// SrcMip, so it naturally rests as SHADER_RESOURCE; only the
				// coarsest mip's write is never followed by a read.
				UINT last_mip_subres = (UINT)(pyramid_mip_count - 1) + (UINT)slot * (UINT)pyramid_mip_count;
				command_list->transition(data.VSM_PageHiZ->resource, HAL::ResourceStates::SHADER_RESOURCE, last_mip_subres);
			}
		};
	}

	// ---- Combine lighting ------------------------------------------------

	m_combine_setup = [this](Passes::VSM_Combine::Context& data, FrameGraph::TaskBuilder& builder) -> bool
	{
		GBufferViewDesc::need(builder, data.gbuffer);
		builder.need(data.ResultTexture, FrameGraph::ResourceFlags::UnorderedAccess);
		builder.need(data.VSM_Atlas, FrameGraph::ResourceFlags::ComputeRead);
		builder.need(data.VSM_PageTable, FrameGraph::ResourceFlags::ComputeRead);
		builder.need(data.VSM_PageCameras, FrameGraph::ResourceFlags::ComputeRead);
		return true;
	};

	m_combine_render = [this](Passes::VSM_Combine::Context& data, FrameGraph::FrameContext& context)
	{
		GBuffer gbuffer = GBufferViewDesc::actualize(data.gbuffer);

		auto& list    = *context.get_list();
		auto& compute = list.get_compute();

		auto& caminfo = context.graph->get_context<CameraInfo>();
		auto  cam     = caminfo.cam;

		context.graph->set_slot(SlotID::FrameInfo, compute);

		camera light_cam = make_light_view_camera(get_position());
		float2 cam_pos_ls = (float4(cam->position, 1) * light_cam.get_view()).xy;

		{
			Slots::VSMLighting lighting;
			gbuffer.SetTable(lighting.GetGbuffer());
			lighting.GetVsm_atlas()    = data.VSM_Atlas->texture2D;
			lighting.GetPage_table()   = data.VSM_PageTable->texture2DArray;
			lighting.GetPage_cameras() = data.VSM_PageCameras->structuredBuffer;
			lighting.GetResult()       = data.ResultTexture->rwTexture2D;
			compute.set(lighting);
		}

		{
			Slots::VSMConstants constants;
			constants.GetLevel_count()          = page_table.clipmap.level_count;
			constants.GetPage_size()            = page_table.page_size;
			constants.GetPages_per_level()      = page_table.clipmap.pages_per_level;
			constants.GetAtlas_pages_per_side() = page_table.atlas_pages_per_side;
			constants.GetLight_view()           = light_cam.get_view();

			for (int level = 0; level < page_table.clipmap.level_count; level++)
			{
				float2 origin = page_table.clipmap.grid_origin(level, cam_pos_ls);
				constants.GetLevel_info()[level] = float4(origin.x, origin.y, page_table.clipmap.page_world_size(level), 0);
			}

			compute.set(constants);
		}

		compute.set_pipeline<PSOS::VSMApplyCompute>();
		compute.dispatch(context.graph->get_context<ViewportInfo>().frame_size, ivec2{ 16, 16 });
	};
}
