#include "pch.h"

namespace GUI
{
    namespace Elements
    {

        void label::draw(Render::context& c)
        {
            //	draw_helper = true;
            //	base::draw(c);
            recalculate(c);
          //  area1 = intersect(lay1, c.ui_clipping - vec4(render_bounds->pos, render_bounds->pos) - vec4(c.offset, c.offset));
        //    area2 = intersect(lay2, c.ui_clipping - vec4(render_bounds->pos, render_bounds->pos) - vec4(c.offset, c.offset));
            //   geomerty_shadow->draw(c.command_list, area1, 0, c.offset + vec2(render_bounds->pos));
			renderer->draw(c, cache, get_render_bounds());
		//	renderer->flush(c);
       //    geomerty->draw(c.command_list, area2, 0, c.offset + vec2(render_bounds->pos));
     //       c.command_list->get_native_list()->IASetVertexBuffers(0, 0, nullptr);
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
                w = ivec2(render_bounds->size / scaled) ;
                return;
            }
			//if (!cache.texture) {
				auto command_list = c.command_list->get_sub_list();
				command_list->begin("Label");
            rect p = render_bounds.get()/scaled;
            p.x = 0;
            p.y = 0;
            lay2 = math::convert(p);
            p.x += 1;
            p.y += 1;
            lay1 = math::convert(p);
          //  geomerty_shadow->set(command_list, convert(text.get()), font, font_size.get() * scaled, lay1, rgba8(0, 0, 0, 100), magnet_text);
            geomerty->set(command_list, convert(text.get()), font, font_size.get() /* scaled*/, lay2, color, magnet_text);
            need_recalculate = false;
            calculated = true;
            w = vec2(render_bounds->size);

		
				Render::Texture::ptr new_preview= cache.texture;
				lay2.right = std::ceil(lay2.right);
				lay2.bottom = std::ceil(lay2.bottom);

				if(!new_preview|| new_preview->get_desc().Width<lay2.right || new_preview->get_desc().Height<lay2.bottom)
				new_preview.reset(new Render::Texture(CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, lay2.right, lay2.bottom, 1, 0, 1, 0, D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)));
				{
				



					command_list->get_graphics().set_heap(Render::DescriptorHeapType::SAMPLER, Render::DescriptorHeapManager::get().get_samplers());
					command_list->transition(new_preview, Render::ResourceState::RENDER_TARGET);
					command_list->clear_rtv(new_preview->texture_2d()->get_rtv());

					//  target->change_state(c, Render::ResourceState::PIXEL_SHADER_RESOURCE, Render::ResourceState::RENDER_TARGET);
					command_list->get_native_list()->OMSetRenderTargets(1, &new_preview->texture_2d()->get_rtv().cpu, FALSE, nullptr);
				
				//	c->get_graphics().set_pipeline(state);
					Render::Viewport vps;

					vps.Width = static_cast<float>(new_preview->get_desc().Width);
					vps.Height = static_cast<float>(new_preview->get_desc().Height);
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

					geomerty->draw(command_list, lay2, 0, { 0,0 });

					MipMapGenerator::get().generate(command_list->get_compute(), new_preview);

					command_list->transition(new_preview, Render::ResourceState::PIXEL_SHADER_RESOURCE);
				
					command_list->end();
					command_list->execute();
				}

				cache.texture = new_preview;
				cache.tc = vec4{ 0,0, lay2.right_bottom/vec2(new_preview->get_desc().Width,new_preview->get_desc().Height)};
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