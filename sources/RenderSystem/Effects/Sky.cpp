#include "pch_render.h"
#include "Sky.h"

#include "Helpers/MipMapGeneration.h"

using namespace FrameGraph;


SkyRender::SkyRender()
{
	transmittance = Graphics::Texture::get_resource({ to_path(L"textures\\Transmit.dds"), false, false });
	irradiance = Graphics::Texture::get_resource({ to_path(L"textures\\irradianceTexture.dds"), false, false });
	inscatter = Graphics::Texture::get_resource({ to_path(L"textures\\inscatterTexture.dds"), false, false });
}



void SkyRender::generate_sky(Graph& graph)
{

	struct SkyData
	{
		Handlers::Texture H(GBuffer_Depth);
		Handlers::Texture H(ResultTexture);

	};

	graph.add_pass<SkyData>("Sky", [this, &graph](SkyData& data, TaskBuilder& builder) {
		builder.need(data.GBuffer_Depth, ResourceFlags::PixelRead);
		builder.need(data.ResultTexture, ResourceFlags::RenderTarget);
		}, [this, &graph](SkyData& data, FrameContext& _context) {
			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();

			graphics.set_pipeline(GetPSO<PSOS::Sky>());
			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
			graphics.set_viewport(data.ResultTexture->get_viewport());
			graphics.set_scissor(data.ResultTexture->get_scissor());
			graphics.set_rtv(1, data.ResultTexture->renderTarget, HAL::Handle());

			{
				Slots::SkyData skydata;

				skydata.GetInscatter() = inscatter->texture_3d().texture3D;
				skydata.GetIrradiance() = irradiance->texture_2d().texture2D;
				skydata.GetTransmittance() = transmittance->texture_2d().texture2D;

				skydata.GetDepthBuffer() = data.GBuffer_Depth->texture2D;

				skydata.GetSunDir() = graph.sunDir;

				skydata.set(graphics);

			}

			graph.set_slot(SlotID::FrameInfo, graphics);

			graphics.draw(4);


		});
}

void SkyRender::generate(Graph& graph)
{

	struct SkyData
	{
		Handlers::Cube H(sky_cubemap);
	};

	graph.pass<SkyData>("CubeSky", [this, &graph](SkyData& data, TaskBuilder& builder) {
		builder.create(data.sky_cubemap, { ivec3(256, 256, 0), Graphics::Format::R11G11B10_FLOAT, 1 }, ResourceFlags::UnorderedAccess | ResourceFlags::RenderTarget | ResourceFlags::Static);
		bool changed = (graph.sunDir - dir).length() > 0.001;

		if (changed)
		{

			data.sky_cubemap.changed();
			dir = graph.sunDir;
		}

		return changed;
		}, [this, &graph](SkyData& data, FrameContext& _context) {



			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();


			graphics.set_pipeline(GetPSO<PSOS::SkyCube>());

			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);



			{
				Slots::SkyData data;

				data.GetInscatter() = inscatter->texture_3d().texture3D;
				data.GetIrradiance() = irradiance->texture_2d().texture2D;
				data.GetTransmittance() = transmittance->texture_2d().texture2D;
				data.GetSunDir() = graph.sunDir;
				data.set(graphics);

			}

			graph.set_slot(SlotID::FrameInfo, graphics);


			{

				graphics.set_viewport(data.sky_cubemap->get_viewport());
				graphics.set_scissor(data.sky_cubemap->get_scissor());


				for (unsigned int i = 0; i < 6; i++)
				{
					Graphics::TextureViewDesc subres;

					subres.ArraySize = 1;
					subres.FirstArraySlice = i;
					subres.MipLevels = 1;
					subres.MipSlice = 0;

					auto face = data.sky_cubemap->resource->create_view<Graphics::TextureView>(*graphics.get_base().frame_resources, subres);


					Slots::SkyFace skyFace;

					skyFace.GetFace() = i;

					skyFace.set(graphics);

					graphics.set_rtv(1, face.renderTarget, HAL::Handle());

					graphics.draw(4);
				}
			}


		});



	CubeMapEnviromentProcessor::get().generate(graph);

}


CubeMapEnviromentProcessor::CubeMapEnviromentProcessor()
{

}

void CubeMapEnviromentProcessor::generate(Graph& graph)
{

	struct EnvData
	{
		Handlers::Cube H(sky_cubemap);
		Handlers::Cube H(sky_cubemap_filtered);
		Handlers::Cube H(sky_cubemap_filtered_diffuse);
	};

	graph.pass<EnvData>("CubeMapDownsample", [this, &graph](EnvData& data, TaskBuilder& builder) {
		builder.need(data.sky_cubemap, ResourceFlags::UnorderedAccess);
		if (data.sky_cubemap.is_changed())
		{
			return true;
		}
		return false;
		}, [this, &graph](EnvData& data, FrameContext& _context) {
			auto& list = *_context.get_list();
			MipMapGenerator::get().generate_cube(list.get_compute(), *data.sky_cubemap);
		}, PassFlags::Compute
			);


	graph.pass<EnvData>("CubeMapEnviromentProcessor", [this, &graph](EnvData& data, TaskBuilder& builder) {
		builder.need(data.sky_cubemap, ResourceFlags::PixelRead);

		builder.create(data.sky_cubemap_filtered, { ivec3(64, 64,0),  Graphics::Format::R11G11B10_FLOAT,1 }, ResourceFlags::RenderTarget | ResourceFlags::Static);
		builder.create(data.sky_cubemap_filtered_diffuse, { ivec3(64, 64,0),  Graphics::Format::R11G11B10_FLOAT,1 }, ResourceFlags::RenderTarget | ResourceFlags::Static);

		if (data.sky_cubemap.is_changed())
		{

			return true;
		}

		return false;
		}, [this, &graph](EnvData& data, FrameContext& _context) {


			auto& list = *_context.get_list();
			auto& graphics = list.get_graphics();

			graphics.set_topology(HAL::PrimitiveTopologyType::TRIANGLE, HAL::PrimitiveTopologyFeed::STRIP);
			graphics.set_signature(get_Signature(Layouts::DefaultLayout));

			Slots::EnvSource downsample;
			downsample.GetSourceTex() = data.sky_cubemap->textureCube;
			downsample.set(graphics);



			UINT count = data.sky_cubemap_filtered->resource->get_desc().as_texture().MipLevels;
			for (unsigned int m = 0; m < count; m++)
			{
				graphics.set_pipeline(GetPSO<PSOS::CubemapENV>(PSOS::CubemapENV::Level(std::min(m, 4u))));
				for (unsigned int i = 0; i < 6; i++)
				{
					Graphics::TextureViewDesc subres;

					subres.ArraySize = 1;
					subres.FirstArraySlice = i;
					subres.MipLevels = 1;
					subres.MipSlice = m;

					auto face = data.sky_cubemap_filtered->resource->create_view<Graphics::TextureView>(*graphics.get_base().frame_resources, subres);

					if (i == 0) {
						graphics.set_viewport(face.get_viewport());
						graphics.set_scissor(face.get_scissor());
					}
					graphics.set_rtv(1, face.renderTarget, HAL::Handle());

					Slots::EnvFilter filter;
					filter.GetFace().x = i;

					filter.GetScaler().x = (float(m) + 0.5f) / count;
					filter.GetSize().x = (UINT)data.sky_cubemap->resource->get_desc().as_texture().Dimensions.x;
					filter.set(graphics);

					graphics.draw(4);
				}
			}



			graphics.set_pipeline(GetPSO<PSOS::CubemapENVDiffuse>());

			for (unsigned int i = 0; i < 6; i++)
			{
				Graphics::TextureViewDesc subres;

				subres.ArraySize = 1;
				subres.FirstArraySlice = i;
				subres.MipLevels = 1;
				subres.MipSlice = 0;

				auto face = data.sky_cubemap_filtered_diffuse->resource->create_view<Graphics::TextureView>(*graphics.get_base().frame_resources, subres);

				if (i == 0) {
					graphics.set_viewport(face.get_viewport());
					graphics.set_scissor(face.get_scissor());
				}
				graphics.set_rtv(1, face.renderTarget, HAL::Handle());

				Slots::EnvFilter filter;
				filter.GetFace().x = i;

				filter.GetScaler().x = (float(0) + 0.5f) / count;
				filter.GetSize().x = (UINT)data.sky_cubemap_filtered_diffuse->resource->get_desc().as_texture().Dimensions.x;
				filter.set(graphics);

				graphics.draw(4);

			}
		});



}
