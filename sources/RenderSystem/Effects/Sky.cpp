module Graphics:Sky;


import :FrameGraphContext;
import :MipMapGenerator;
import FrameGraph;
import HAL;

#include "../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

#define STRINGIFY(x) #x
#define PPCAT_NX(A, B) A ## B

#define SKY_TEXTURE(name) \
    EngineAsset<TextureAsset> name(PPCAT_NX(L,#name), [] { \
		return new TextureAsset(to_path(L"textures/" STRINGIFY(name) ".dds")); \
    })

namespace EngineAssets
{
	SKY_TEXTURE(Transmit);
	SKY_TEXTURE(irradianceTexture);
	SKY_TEXTURE(inscatterTexture);
}

SkyRender::SkyRender()
{
	transmittance = EngineAssets::Transmit.get_asset()->get_texture();
	irradiance    = EngineAssets::irradianceTexture.get_asset()->get_texture();
	inscatter     = EngineAssets::inscatterTexture.get_asset()->get_texture();

	// ---- Pass function members ------------------------------------------------

	// CubeSky: renders the atmospheric sky into a static cubemap, re-baked only
	// when the sun direction has changed enough to warrant it.
	m_cubesky_setup = [this](Passes::CubeSky::Context& data, FrameGraph::TaskBuilder& builder) -> FrameGraph::SetupResult
	{
		auto& sky = builder.graph->get_context<SkyInfo>();

		bool changed = ((sky.sunDir - dir).length() > 0.001f);
		if (changed) dir = sky.sunDir;

		// Mirrored into Table::SkyState -- see its own comment (sky.sig) for
		// why this can't just be a SkyInfo field: CubeMapDownsample/
		// CubeMapEnviromentProcessor's [RenderCondition] reads it from
		// autogen/pass_defaults.cpp, which only sees Table:: contexts.
		builder.graph->get_context<Table::SkyState>().sky_changed = changed;

		return changed ? FrameGraph::SetupResult::NeedsRender : FrameGraph::SetupResult::IgnoreRender;
	};

	m_cubesky_render = [this](Passes::CubeSky::Context& data, FrameGraph::FrameContext& context)
	{
		auto& sky     = context.graph->get_context<SkyInfo>();
		auto& compute = context.get_list()->get_compute();

		compute.set_pipeline<PSOS::SkyCube>();

		{
			PROFILE(L"cube_sky_setup");

			Slots::SkyData skydata;
			skydata.GetInscatter()     = inscatter->texture_3d().texture3D;
			skydata.GetIrradiance()    = irradiance->texture_2d().texture2D;
			skydata.GetTransmittance() = transmittance->texture_2d().texture2D;
			skydata.GetSunDir()        = sky.sunDir;
			compute.set(skydata);

			context.graph->set_slot(SlotID::FrameInfo, compute);
		}

		{
			PROFILE(L"cube_sky_faces");

			auto& cube = *data.sky_cubemap;
			auto  size = cube.get_size();

			// Mip 0 of all six faces as one array UAV, so the whole cube is a
			// single dispatch with the face in Z.
			HAL::TextureViewDesc subres;
			subres.MipSlice        = 0;
			subres.MipLevels       = 1;
			subres.FirstArraySlice = 0;
			subres.ArraySize       = 6;

			Slots::SkyFace skyFace;
			skyFace.GetFaces() = cube.resource->create_view<HAL::Texture2DView>(
				compute.get_base(), subres).rwTexture2DArray;
			compute.set(skyFace);

			compute.dispatch(ivec3(size.x, size.y, 6), ivec3(8, 8, 1));
		}
	};

	// Sky: full-screen sky pass that composites over the GBuffer depth.
	m_sky_setup = [](Passes::Sky::Context& data, FrameGraph::TaskBuilder& builder) -> FrameGraph::SetupResult
	{
		return true;
	};

	m_sky_render = [this](Passes::Sky::Context& data, FrameGraph::FrameContext& context)
	{
		auto& sky     = context.graph->get_context<SkyInfo>();
		auto& compute = context.get_list()->get_compute();

		context.graph->set_slot(SlotID::FrameInfo, compute);

		{
			Slots::SkyData skydata;
			skydata.GetInscatter()     = inscatter->texture_3d().texture3D;
			skydata.GetIrradiance()    = irradiance->texture_2d().texture2D;
			skydata.GetTransmittance() = transmittance->texture_2d().texture2D;
			skydata.GetDepthBuffer()   = data.GBuffer_Depth->texture2D;
			skydata.GetSunDir()        = sky.sunDir;
			skydata.GetResult()        = data.ResultTexture->rwTexture2D;
			compute.set(skydata);
		}

		compute.set_pipeline<PSOS::SkyCompute>();
		compute.dispatch(context.graph->get_context<ViewportInfo>().frame_size, ivec2{ 16, 16 });
	};
}


// ---- PassDefault<Passes::CubeMapDownsample> --------------------------------
// Generates mipmaps for the sky cubemap whenever it has been re-baked.
// setup() is fully generated (sky.sig's own [RenderCondition]).

void PassDefault<Passes::CubeMapDownsample>::render(
	Passes::CubeMapDownsample::Context& data, FrameContext& context)
{
	MipMapGenerator::get().generate_cube(context.get_list()->get_compute(), *data.sky_cubemap);
}


// ---- PassDefault<Passes::CubeMapEnviromentProcessor> ----------------------
// Filters the sky cubemap into specular and diffuse IBL targets.
// setup() is fully generated (sky.sig's own [RenderCondition]).

void PassDefault<Passes::CubeMapEnviromentProcessor>::render(
	Passes::CubeMapEnviromentProcessor::Context& data, FrameContext& context)
{
	auto& compute = context.get_list()->get_compute();

	compute.set_signature(Layouts::DefaultLayout);

	{
		Slots::EnvSource downsample;
		downsample.GetSourceTex() = data.sky_cubemap->textureCube;
		compute.set(downsample);
	}

	// One mip of a cubemap as a single UAV spanning all six faces.
	auto slice_span = [&](HAL::CubeView& cube, UINT mip)
	{
		HAL::TextureViewDesc subres;
		subres.MipSlice        = mip;
		subres.MipLevels       = 1;
		subres.FirstArraySlice = 0;
		subres.ArraySize       = 6;

		return cube.resource->create_view<HAL::Texture2DView>(compute.get_base(), subres);
	};

	// Specular filtered cubemap -- every mip of every face in one dispatch. The
	// shader walks a flat index over the concatenated per-mip spans, so the
	// mismatched mip sizes still map to one texel per thread.
	{
		PROFILE(L"env_specular_filter");

		auto& filtered = *data.sky_cubemap_filtered;

		const UINT count = filtered.get_desc().as_texture().MipLevels;
		const UINT base  = (UINT)filtered.get_size().x;

		// EnvFilter carries one array UAV per mip; the 64^2 cubemap is 7 mips.
		ASSERT(count <= 8);

		compute.set_pipeline<PSOS::CubemapENV>();

		// Value-initialised: only the first `count` targets are filled.
		Slots::EnvFilter filter{};
		filter.GetSize().x = (UINT)data.sky_cubemap->resource->get_desc().as_texture().Dimensions.x;
		filter.GetSize().y = count;
		filter.GetSize().z = base;

		UINT total = 0;
		for (UINT m = 0; m < count; m++)
		{
			UINT size = std::max(base >> m, 1u);

			filter.GetTargets()[m] = slice_span(filtered, m).rwTexture2DArray;
			total += size * size * 6;
		}
		compute.set(filter);

		compute.dispatch(ivec2(total, 1), ivec2(64, 1));
	}

	// Diffuse filtered cubemap -- mip 0 only, so one dispatch with the face in Z.
	{
		PROFILE(L"env_diffuse_filter");

		auto& diffuse = *data.sky_cubemap_filtered_diffuse;
		auto  size    = diffuse.get_size();

		compute.set_pipeline<PSOS::CubemapENVDiffuse>();

		Slots::EnvFilter filter{};
		filter.GetSize().x     = (UINT)size.x;
		filter.GetSize().y     = 1;
		filter.GetSize().z     = (UINT)size.x;
		filter.GetTargets()[0] = slice_span(diffuse, 0).rwTexture2DArray;
		compute.set(filter);

		compute.dispatch(ivec3(size.x, size.y, 6), ivec3(8, 8, 1));
	}
}
