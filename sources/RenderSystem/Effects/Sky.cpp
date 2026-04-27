module Graphics:Sky;

import <RenderSystem.h>;
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
}


// ---- PassDefault<Passes::CubeMapDownsample> --------------------------------
// Generates mipmaps for the sky cubemap whenever it has been re-baked.

bool PassDefault<Passes::CubeMapDownsample>::setup(
	Passes::CubeMapDownsample::Context& data, TaskBuilder& builder)
{
	builder.need(data.sky_cubemap, ResourceFlags::UnorderedAccess);
	return data.sky_cubemap.is_changed();
}

void PassDefault<Passes::CubeMapDownsample>::render(
	Passes::CubeMapDownsample::Context& data, FrameContext& context)
{
	MipMapGenerator::get().generate_cube(context.get_list()->get_compute(), *data.sky_cubemap);
}


// ---- PassDefault<Passes::CubeMapEnviromentProcessor> ----------------------
// Filters the sky cubemap into specular and diffuse IBL targets.

bool PassDefault<Passes::CubeMapEnviromentProcessor>::setup(
	Passes::CubeMapEnviromentProcessor::Context& data, TaskBuilder& builder)
{
	builder.need(data.sky_cubemap, ResourceFlags::PixelRead);
	builder.create(data.sky_cubemap_filtered,
	               { ivec3(64, 64, 0), HAL::Format::R11G11B10_FLOAT, 1 },
	               ResourceFlags::RenderTarget | ResourceFlags::Static);
	builder.create(data.sky_cubemap_filtered_diffuse,
	               { ivec3(64, 64, 0), HAL::Format::R11G11B10_FLOAT, 1 },
	               ResourceFlags::RenderTarget | ResourceFlags::Static);
	return data.sky_cubemap.is_changed();
}

void PassDefault<Passes::CubeMapEnviromentProcessor>::render(
	Passes::CubeMapEnviromentProcessor::Context& data, FrameContext& context)
{
	auto& graphics = context.get_list()->get_graphics();

	graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
	graphics.set_signature(Layouts::DefaultLayout);

	{
		Slots::EnvSource downsample;
		downsample.GetSourceTex() = data.sky_cubemap->textureCube;
		graphics.set(downsample);
	}

	// Specular filtered cubemap — one draw per mip level per face.
	UINT count = data.sky_cubemap_filtered->resource->get_desc().as_texture().MipLevels;
	for (unsigned int m = 0; m < count; m++)
	{
		graphics.set_pipeline<PSOS::CubemapENV>(PSOS::CubemapENV::Level(std::min(m, 4u)));

		for (unsigned int i = 0; i < 6; i++)
		{
			HAL::TextureViewDesc subres;
			subres.ArraySize       = 1;
			subres.FirstArraySlice = i;
			subres.MipLevels       = 1;
			subres.MipSlice        = m;

			auto face = data.sky_cubemap_filtered->resource->create_view<HAL::Texture2DView>(
				graphics.get_base(), subres);

			{
				RT::SingleColor rt;
				rt.GetColor() = face.renderTarget;
				graphics.set_rtv(rt, i == 0 ? RTOptions::Default : RTOptions::SetHandles);
			}

			Slots::EnvFilter filter;
			filter.GetFace().x   = i;
			filter.GetScaler().x = (float(m) + 0.5f) / count;
			filter.GetSize().x   = (UINT)data.sky_cubemap->resource->get_desc().as_texture().Dimensions.x;
			graphics.set(filter);

			graphics.draw(4);
		}
	}

	// Diffuse filtered cubemap — one draw per face.
	graphics.set_pipeline<PSOS::CubemapENVDiffuse>();
	for (unsigned int i = 0; i < 6; i++)
	{
		HAL::TextureViewDesc subres;
		subres.ArraySize       = 1;
		subres.FirstArraySlice = i;
		subres.MipLevels       = 1;
		subres.MipSlice        = 0;

		auto face = data.sky_cubemap_filtered_diffuse->resource->create_view<HAL::Texture2DView>(
			graphics.get_base(), subres);

		if (i == 0)
		{
			graphics.set_viewport(face.get_viewport());
			graphics.set_scissor(face.get_scissor());
		}

		{
			RT::SingleColor rt;
			rt.GetColor() = face.renderTarget;
			graphics.set_rtv(rt);
		}

		Slots::EnvFilter filter;
		filter.GetFace().x   = i;
		filter.GetScaler().x = (float(0) + 0.5f) / count;
		filter.GetSize().x   = (UINT)data.sky_cubemap_filtered_diffuse->resource->get_desc().as_texture().Dimensions.x;
		graphics.set(filter);

		graphics.draw(4);
	}
}
