#include "pch.h"
#include "Sky.h"

#include "Helpers/MipMapGeneration.h"

using namespace FrameGraph;


SkyRender::SkyRender()
{
	transmittance = Render::Texture::get_resource({ "textures\\Transmit.dds", false, false });
	irradiance = Render::Texture::get_resource({ "textures\\irradianceTexture.dds", false, false });
	inscatter = Render::Texture::get_resource({ "textures\\inscatterTexture.dds", false, false });
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
			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			graphics.set_viewport(data.ResultTexture->get_viewport());
			graphics.set_scissor(data.ResultTexture->get_scissor());
			graphics.set_rtv(1, data.ResultTexture->renderTarget, Render::Handle());

			{
				Slots::SkyData skydata;

				skydata.GetInscatter() = inscatter->texture_3d()->texture3D;
				skydata.GetIrradiance() = irradiance->texture_2d()->texture2D;
				skydata.GetTransmittance() = transmittance->texture_2d()->texture2D;

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
		Handlers::Texture H(sky_cubemap);
	};

	graph.pass<SkyData>("CubeSky", [this, &graph](SkyData& data, TaskBuilder& builder) {
		builder.create(data.sky_cubemap, {ivec3(256, 256, 1), DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, 6}, ResourceFlags::UnorderedAccess | ResourceFlags::RenderTarget | ResourceFlags::Cube | ResourceFlags::Static);
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

			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);



			{
				Slots::SkyData data;

				data.GetInscatter() = inscatter->texture_3d()->texture3D;
				data.GetIrradiance() = irradiance->texture_2d()->texture2D;
				data.GetTransmittance() = transmittance->texture_2d()->texture2D;
				data.GetSunDir() = graph.sunDir;
				data.set(graphics);

			}

			graph.set_slot(SlotID::FrameInfo, graphics);


			{

				graphics.set_viewport(data.sky_cubemap->get_viewport());
				graphics.set_scissor(data.sky_cubemap->get_scissor());


				for (unsigned int i = 0; i < 6; i++)
				{
					Render::ResourceViewDesc subres;
					subres.type = Render::ResourceType::TEXTURE2D;

					subres.Texture2D.ArraySize = 1;
					subres.Texture2D.FirstArraySlice = i;
					subres.Texture2D.MipLevels = 1;
					subres.Texture2D.MipSlice = 0;
					subres.Texture2D.PlaneSlice = 0;

					auto face = data.sky_cubemap->resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres);


					Slots::SkyFace skyFace;

					skyFace.GetFace() = i;

					skyFace.set(graphics);

					graphics.set_rtv(1, face.renderTarget, Render::Handle());

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
		Handlers::Texture H(sky_cubemap);
		Handlers::Texture H(sky_cubemap_filtered);
		Handlers::Texture H(sky_cubemap_filtered_diffuse);
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

		 builder.create(data.sky_cubemap_filtered, { ivec3(64, 64,1),  DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT,6 }, ResourceFlags::RenderTarget | ResourceFlags::Cube | ResourceFlags::Static);
		 builder.create(data.sky_cubemap_filtered_diffuse, { ivec3(64, 64,1),  DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT,6 }, ResourceFlags::RenderTarget | ResourceFlags::Cube | ResourceFlags::Static);

		if (data.sky_cubemap.is_changed())
		{

			return true;
		}

		return false;
		}, [this, &graph](EnvData& data, FrameContext& _context) {


			auto& list = *_context.get_list();
			auto& graphics = list.get_graphics();

			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			graphics.set_signature(get_Signature(Layouts::DefaultLayout));

			Slots::EnvSource downsample;
			downsample.GetSourceTex() = data.sky_cubemap->textureÑube;
			downsample.set(graphics);



			UINT count = data.sky_cubemap_filtered->resource->get_desc().MipLevels;
			for (unsigned int m = 0; m < count; m++)
			{
				graphics.set_pipeline(GetPSO<PSOS::CubemapENV>(PSOS::CubemapENV::Level(std::min(m, 4u))));
				for (unsigned int i = 0; i < 6; i++)
				{
					Render::ResourceViewDesc subres;
					subres.type = Render::ResourceType::TEXTURE2D;

					subres.Texture2D.ArraySize = 1;
					subres.Texture2D.FirstArraySlice = i;
					subres.Texture2D.MipLevels = 1;
					subres.Texture2D.MipSlice = m;
					subres.Texture2D.PlaneSlice = 0;

					auto face = data.sky_cubemap_filtered->resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres);

					if (i == 0) {
						graphics.set_viewport(face.get_viewport());
						graphics.set_scissor(face.get_scissor());
					}
					graphics.set_rtv(1, face.renderTarget, Render::Handle());

					Slots::EnvFilter filter;
					filter.GetFace().x = i;

					filter.GetScaler().x = (float(m) + 0.5f) / count;
					filter.GetSize().x = (UINT)data.sky_cubemap->resource->get_desc().Width;
					filter.set(graphics);

					graphics.draw(4);
				}
			}



			graphics.set_pipeline(GetPSO<PSOS::CubemapENVDiffuse>());

			for (unsigned int i = 0; i < 6; i++)
			{
				Render::ResourceViewDesc subres;
				subres.type = Render::ResourceType::TEXTURE2D;

				subres.Texture2D.ArraySize = 1;
				subres.Texture2D.FirstArraySlice = i;
				subres.Texture2D.MipLevels = 1;
				subres.Texture2D.MipSlice = 0;
				subres.Texture2D.PlaneSlice = 0;

				auto face = data.sky_cubemap_filtered_diffuse->resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres);

				if (i == 0) {
					graphics.set_viewport(face.get_viewport());
					graphics.set_scissor(face.get_scissor());
				}
				graphics.set_rtv(1, face.renderTarget, Render::Handle());

				Slots::EnvFilter filter;
				filter.GetFace().x = i;

				filter.GetScaler().x = (float(0) + 0.5f) / count;
				filter.GetSize().x = (UINT)data.sky_cubemap_filtered_diffuse->resource->get_desc().Width;
				filter.set(graphics);

				graphics.draw(4);

			}
		});



}
