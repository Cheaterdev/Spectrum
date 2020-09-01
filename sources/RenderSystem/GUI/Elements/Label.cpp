#include "pch.h"

namespace GUI
{
	namespace Elements
	{


		void label::draw(Render::context& c)
		{
		//	return;
			recalculate(c);		
			rect p = render_bounds.get();
			auto orig = c.ui_clipping;
			c.ui_clipping = intersect(c.ui_clipping ,math::convert(p));
			p.w = std::min(text_size.pos.x, render_bounds->w);
			p.h = std::min(text_size.pos.y, render_bounds->h);
		
			if (c.ui_clipping.left < c.ui_clipping.right)
				if (c.ui_clipping.top < c.ui_clipping.bottom)
			{
				p.w = std::ceil(p.w);
				p.h = std::ceil(p.h);

				if ((magnet_text&FW1_CENTER)&&text_size.pos.x < render_bounds->w)
					p.x += (p.w - std::ceil(text_size.pos.x)) / 2;

				if ((magnet_text&FW1_VCENTER)&&text_size.pos.y < render_bounds->h)
					p.y += (p.h - std::ceil(text_size.pos.y)) / 2;
				sizer intersected = intersect(math::convert(p), c.ui_clipping);
					if((intersected.top < intersected.bottom && intersected.left < intersected.right))
				renderer->draw(c, cache, p);
			}

			c.ui_clipping = orig;
		}



		label::label() : text(std::bind(&label::on_text_changed, this, std::placeholders::_1)), font_size(std::bind(&label::on_size_changed, this, std::placeholders::_1))

		{
			clickable = false;
			magnet_text = FW1_CENTER | FW1_VCENTER;
			font = Fonts::FontSystem::get().get_font("Segoe UI Light");
			color = rgba8(255, 255, 255, 255);
			geomerty.reset(new Fonts::FontGeometry());
			geomerty_shadow.reset(new Fonts::FontGeometry());
			font_size = 15;
			text_size = font->measure("", font_size.get() /** scaled*/, magnet_text);

/*
			internal_label = std::make_shared<label_internal>();
			internal_label->docking = dock::FILL;
			add_child(internal_label);*/
		}

		void label::on_text_changed(const std::string& str)
		{
			text_size = font->measure(str, font_size.get() /** scaled*/, magnet_text);// +vec2(5 / scaled, 0);
			size = text_size;
			need_recalculate = true;
		}
		void label::on_size_changed(const float& new_size)
		{
			on_text_changed(text.get());
		}
		void label::recalculate(Render::context& c)
		{
			if (!need_recalculate && ((w == ivec2(render_bounds->size / scaled) || (magnet_text & FW1_NOWORDWRAP))))
			{
				w = ivec2(render_bounds->size / scaled);
				return;
			}
			//if (!cache.texture) {
		
			rect p = {0,0,text_size.pos};
		//	rect p =  render_bounds.get() / scaled;
			p.w = std::min(text_size.pos.x, render_bounds->w/ scaled);
			p.h = std::min(text_size.pos.y, render_bounds->h / scaled);
			p.x = 0;
			p.y = 0;
			lay2 = math::convert(p);
			p.x += 1;
			p.y += 1;
			lay1 = math::convert(p);
			//  geomerty_shadow->set(command_list, convert(text.get()), font, font_size.get() * scaled, lay1, rgba8(0, 0, 0, 100), magnet_text);
			need_recalculate = false;
			calculated = true;
			w = vec2(render_bounds->size);
			wptr _THIS = get_ptr<label>();

			Render::Texture::ptr new_preview = cache.texture;
			lay2.right = std::ceil(lay2.right);
			lay2.bottom = std::ceil(lay2.bottom);

			if (lay2.right == 0) lay2.right = 1;
			if (lay2.bottom == 0) lay2.bottom = 1;

			if(!isnan(lay2.right))
				if (!cache.texture || cache.texture->get_desc().Width < lay2.right || cache.texture->get_desc().Height < lay2.bottom)
				cache.texture.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, (UINT)lay2.right, (UINT)lay2.bottom, 1, 0, 1, 0, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));

			auto _command_list = c.command_list_label;// c.command_list->get_sub_list();

			
			c.labeled->add([this, _THIS, _command_list]
			{
				ptr my_ptr = _THIS.lock();
				if (!my_ptr) return;
				auto command_list = const_cast<CommandList*>(_command_list.get())->get_ptr(); //wtf is this

				geomerty->set(command_list, convert(text.get()), font, font_size.get() , lay2, color, magnet_text);



			//	command_list->set_heap(Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapManager::get().get_samplers());
			//	command_list->transition(cache.texture, Render::ResourceState::RENDER_TARGET);
				command_list->clear_rtv(cache.texture->texture_2d()->get_rtv());

				//  target->change_state(c, Render::ResourceState::PIXEL_SHADER_RESOURCE, Render::ResourceState::RENDER_TARGET);
				command_list->get_native_list()->OMSetRenderTargets(1, &cache.texture->texture_2d()->get_rtv().cpu, FALSE, nullptr);

				//	c->get_graphics().set_pipeline(state);
				Render::Viewport vps;

				vps.Width = static_cast<float>(cache.texture->get_desc().Width);
				vps.Height = static_cast<float>(cache.texture->get_desc().Height);
				vps.TopLeftX = 0;
				vps.TopLeftY = 0;
				vps.MinDepth = 0.0f;
				vps.MaxDepth = 1.0f;
				command_list->get_graphics().set_viewports({ vps });
				sizer_long s = { 0, 0, vps.Width , vps.Height };
				command_list->get_graphics().set_scissors({ s });
				//	ccommand_list->get_graphics().set_dynamic(0, 0, asset->get_texture()->texture_2d()->get_srv());
				//	command_list->get_graphics().set(1, sampler_table);
				//	command_list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				//	command_list->get_graphics().draw(4, 0);
				command_list->get_graphics().use_dynamic = false;
				geomerty->draw(command_list, lay2, 0, { 0,0 });
				command_list->get_graphics().use_dynamic = true;
				MipMapGenerator::get().generate(command_list->get_compute(), cache.texture);

			//	command_list->transition(cache.texture, Render::ResourceState::PIXEL_SHADER_RESOURCE);

			
			});
			
	
			cache.tc = vec4{ 0,0, lay2.right_bottom / vec2(cache.texture->get_desc().Width,cache.texture->get_desc().Height) };
			//}
		}

		Fonts::FontGeometry::ptr label::get_geometry()
		{
			return geomerty;
		}

		unsigned int label::get_index(vec3 at)
		{
			return geomerty->get_index(at);
		}

		void label::on_bounds_changed(const rect& r)
		{
			base::on_bounds_changed(r);
			need_recalculate = true;
		}

		sizer label::update_layout(sizer r, float scale)
		{
			/* if (scale != scaled)
			 {
				 calculated = false;
				 scaled = scale;
				 on_text_changed(text.get());
			 }
			 */
			scaled = scale;

			return	base::update_layout(r, scale);
		}




	}
}