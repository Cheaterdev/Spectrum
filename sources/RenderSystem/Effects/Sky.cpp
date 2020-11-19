#include "pch.h"




SkyRender::SkyRender()
{
	transmittance = Render::Texture::get_resource({ "textures\\Transmit.dds", false, false });
	irradiance = Render::Texture::get_resource({ "textures\\irradianceTexture.dds", false, false });
	inscatter = Render::Texture::get_resource({ "textures\\inscatterTexture.dds", false, false });
}



void SkyRender::generate(FrameGraph& graph)
{

	struct SkyData
	{
		ResourceHandler* depth;

		ResourceHandler* target_tex;


		ResourceHandler* sky_cubemap;


	};

	graph.add_pass<SkyData>("Sky", [this, &graph](SkyData& data, TaskBuilder& builder) {
		data.depth = builder.need_texture("GBuffer_Depth", ResourceFlags::PixelRead);
		data.target_tex = builder.need_texture("ResultTexture", ResourceFlags::RenderTarget);
		data.sky_cubemap = builder.create_texture("sky_cubemap", ivec2(256, 256), 6, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::UnorderedAccess | ResourceFlags::RenderTarget | ResourceFlags::Cube | ResourceFlags::Static);
		}, [this, &graph](SkyData& data, FrameContext& _context) {
			auto depth = _context.get_texture(data.depth);

			auto target_tex = _context.get_texture(data.target_tex);

			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();

			auto sky_cubemap = _context.get_texture(data.sky_cubemap);


			graphics.set_pipeline(GetPSO<PSOS::Sky>());


			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);



			graphics.set_viewport(target_tex.get_viewport());
			graphics.set_scissor(target_tex.get_scissor());
			graphics.set_rtv(1, target_tex.get_rtv(), Render::Handle());

			{
				Slots::SkyData data;

				data.GetInscatter() = inscatter->texture_3d()->get_static_srv();
				data.GetIrradiance() = irradiance->texture_2d()->get_static_srv();
				data.GetTransmittance() = transmittance->texture_2d()->get_static_srv();

				data.GetDepthBuffer() = depth.get_srv();

				data.GetSunDir() = graph.sunDir;

				data.set(graphics);

			}
			{
				Slots::FrameInfo frameInfo;

				auto camera = frameInfo.MapCamera();
				camera.cb = graph.cam->camera_cb.current;
				//memcpy(&camera.cb, &graph.cam->camera_cb.current, sizeof(camera.cb));
				frameInfo.set(graphics);
			}
			graphics.draw(4);



			{

				graphics.set_viewport(sky_cubemap.get_viewport());
				graphics.set_scissor(sky_cubemap.get_scissor());
				graphics.set_pipeline(GetPSO<PSOS::SkyCube>());


				for (unsigned int i = 0; i < 6; i++)
				{
					Render::ResourceViewDesc subres;
					subres.type = Render::ResourceType::TEXTURE2D;

					subres.Texture2D.ArraySize = 1;
					subres.Texture2D.FirstArraySlice = i;
					subres.Texture2D.MipLevels = 1;
					subres.Texture2D.MipSlice = 0;
					subres.Texture2D.PlaneSlice = 0;

					auto face = sky_cubemap.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres);


					Slots::SkyFace skyFace;

					skyFace.GetFace() = i;

					skyFace.set(graphics);

					graphics.set_rtv(1, face.get_rtv(), Render::Handle());

					graphics.draw(4);
				}
			}

			//		auto tex = std::dynamic_pointer_cast<Render::Texture>(sky_cubemap.resource);

		});






	graph.pass<SkyData>("CubeSky", [this, &graph](SkyData& data, TaskBuilder& builder) {
		data.sky_cubemap = builder.create_texture("sky_cubemap", ivec2(256, 256), 6, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::UnorderedAccess | ResourceFlags::RenderTarget | ResourceFlags::Cube | ResourceFlags::Static);
		bool changed = (graph.sunDir - dir).length() > 0.001;

		if (changed)
		{

			data.sky_cubemap->changed();
			dir = graph.sunDir;
		}

		return changed;
		}, [this, &graph](SkyData& data, FrameContext& _context) {



			auto& list = *_context.get_list();

			auto& graphics = list.get_graphics();

			auto sky_cubemap = _context.get_texture(data.sky_cubemap);

			graphics.set_pipeline(GetPSO<PSOS::SkyCube>());

			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);



			{
				Slots::SkyData data;

				data.GetInscatter() = inscatter->texture_3d()->get_static_srv();
				data.GetIrradiance() = irradiance->texture_2d()->get_static_srv();
				data.GetTransmittance() = transmittance->texture_2d()->get_static_srv();
				data.GetSunDir() = graph.sunDir;
				data.set(graphics);

			}

			{
				Slots::FrameInfo frameInfo;

				auto camera = frameInfo.MapCamera();
				camera.cb = graph.cam->camera_cb.current;
				//memcpy(&camera.cb, &graph.cam->camera_cb.current, sizeof(camera.cb));
				frameInfo.set(graphics);
			}


			{

				graphics.set_viewport(sky_cubemap.get_viewport());
				graphics.set_scissor(sky_cubemap.get_scissor());


				for (unsigned int i = 0; i < 6; i++)
				{
					Render::ResourceViewDesc subres;
					subres.type = Render::ResourceType::TEXTURE2D;

					subres.Texture2D.ArraySize = 1;
					subres.Texture2D.FirstArraySlice = i;
					subres.Texture2D.MipLevels = 1;
					subres.Texture2D.MipSlice = 0;
					subres.Texture2D.PlaneSlice = 0;

					auto face = sky_cubemap.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres);


					Slots::SkyFace skyFace;

					skyFace.GetFace() = i;

					skyFace.set(graphics);

					graphics.set_rtv(1, face.get_rtv(), Render::Handle());

					graphics.draw(4);
				}
			}


		});



	CubeMapEnviromentProcessor::get().generate(graph);

}


CubeMapEnviromentProcessor::CubeMapEnviromentProcessor()
{

}

void CubeMapEnviromentProcessor::generate(FrameGraph& graph)
{

	struct EnvData
	{

		ResourceHandler* sky_cubemap;

		ResourceHandler* sky_cubemap_filtered;
		ResourceHandler* sky_cubemap_filtered_diffuse;

	};

	graph.pass<EnvData>("CubeMapDownsample", [this, &graph](EnvData& data, TaskBuilder& builder) {
		data.sky_cubemap = builder.need_texture("sky_cubemap", ResourceFlags::UnorderedAccess);
		if (data.sky_cubemap->is_changed())
		{
			return true;
		}
		return false;
		}, [this, &graph](EnvData& data, FrameContext& _context) {
			auto sky_cubemap = _context.get_texture(data.sky_cubemap);
			auto& list = *_context.get_list();

			MipMapGenerator::get().generate_cube(list.get_compute(), sky_cubemap);
		}, PassFlags::Compute
			);


	graph.pass<EnvData>("CubeMapEnviromentProcessor", [this, &graph](EnvData& data, TaskBuilder& builder) {
		data.sky_cubemap = builder.need_texture("sky_cubemap", ResourceFlags::PixelRead);

		data.sky_cubemap_filtered = builder.create_texture("sky_cubemap_filtered", ivec2(64, 64), 6, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::RenderTarget | ResourceFlags::Cube | ResourceFlags::Static);
		data.sky_cubemap_filtered_diffuse = builder.create_texture("sky_cubemap_filtered_diffuse", ivec2(64, 64), 6, DXGI_FORMAT::DXGI_FORMAT_R11G11B10_FLOAT, ResourceFlags::RenderTarget | ResourceFlags::Cube | ResourceFlags::Static);

		if (data.sky_cubemap->is_changed())
		{

			return true;
		}

		return false;
		}, [this, &graph](EnvData& data, FrameContext& _context) {


			auto sky_cubemap = _context.get_texture(data.sky_cubemap);
			auto sky_cubemap_filtered = _context.get_texture(data.sky_cubemap_filtered);
			auto sky_cubemap_filtered_diffuse = _context.get_texture(data.sky_cubemap_filtered_diffuse);


			auto& list = *_context.get_list();
			auto& graphics = list.get_graphics();

			MipMapGenerator::get().generate_cube(list.get_compute(), sky_cubemap);
			graphics.set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			graphics.set_signature(get_Signature(Layouts::DefaultLayout));

			Slots::EnvSource downsample;
			downsample.GetSourceTex() = sky_cubemap.get_srv();
			downsample.set(graphics);



			UINT count = sky_cubemap_filtered.resource->get_desc().MipLevels;
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

					auto face = sky_cubemap_filtered.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres);

					if (i == 0) {
						graphics.set_viewport(face.get_viewport());
						graphics.set_scissor(face.get_scissor());
					}
					graphics.set_rtv(1, face.get_rtv(), Render::Handle());

					Slots::EnvFilter filter;
					filter.GetFace().x = i;

					filter.GetScaler().x = (float(m) + 0.5f) / count;
					filter.GetSize().x = sky_cubemap.resource->get_desc().Width;
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

				auto face = sky_cubemap_filtered_diffuse.resource->create_view<Render::TextureView>(*graphics.get_base().frame_resources, subres);

				if (i == 0) {
					graphics.set_viewport(face.get_viewport());
					graphics.set_scissor(face.get_scissor());
				}
				graphics.set_rtv(1, face.get_rtv(), Render::Handle());

				Slots::EnvFilter filter;
				filter.GetFace().x = i;

				filter.GetScaler().x = (float(0) + 0.5f) / count;
				filter.GetSize().x = sky_cubemap.resource->get_desc().Width;
				filter.set(graphics);

				graphics.draw(4);

			}
		});



}
