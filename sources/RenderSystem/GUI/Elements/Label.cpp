#include "pch_render.h"
#include "Helpers/MipMapGeneration.h"
#include "Label.h"

#include "GUI/Renderer/Renderer.h"

namespace GUI
{
	namespace Elements
	{


		void label::draw(Graphics::context& c)
		{
			//	return;
			recalculate(c);

			PROFILE(L"label");

			rect p = render_bounds.get();
			auto orig = c.ui_clipping;
			c.ui_clipping = intersect(c.ui_clipping, math::convert(p));
			p.w = std::min(text_size.x, render_bounds->w);
			p.h = std::min(text_size.y, render_bounds->h);

			if (c.ui_clipping.left < c.ui_clipping.right)
				if (c.ui_clipping.top < c.ui_clipping.bottom)
				{
					p.w = std::ceil(p.w);
					p.h = std::ceil(p.h);

					if ((magnet_text & FW1_CENTER) && text_size.x < render_bounds->w)
						p.x += (p.w - std::ceil(text_size.x)) / 2;

					if ((magnet_text & FW1_VCENTER) && text_size.y < render_bounds->h)
						p.y += (p.h - std::ceil(text_size.y)) / 2;
					sizer intersected = intersect(math::convert(p), c.ui_clipping);
					if ((intersected.top < intersected.bottom && intersected.left < intersected.right))
						renderer->draw(c, cache, p);
				}

			c.ui_clipping = orig;
		}



		label::label() : text(std::bind(&label::on_text_changed, this, std::placeholders::_1)), font_size(std::bind(&label::on_size_changed, this, std::placeholders::_1))
		{
			clickable = false;
			magnet_text = FW1_CENTER | FW1_VCENTER;
			font = Fonts::FontSystem::get().get_font("Segoe UI Light");
			color = float4(1, 1, 1, 1);
			geomerty.reset(new Fonts::FontGeometry());
			geomerty_shadow.reset(new Fonts::FontGeometry());
			font_size = 15;
			text_size = font->measure("", font_size.get() /** scaled*/, magnet_text);
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
		void label::recalculate(Graphics::context& c)
		{
			if (!need_recalculate && ((w == ivec2(render_bounds->size / scaled) || (magnet_text & FW1_NOWORDWRAP))))
			{
				w = ivec2(render_bounds->size / scaled);
				return;
			}
			//if (!cache.texture) {

			rect p = { 0,0,text_size };
			//	rect p =  render_bounds.get() / scaled;
			p.w = std::min(text_size.x, render_bounds->w / scaled);
			p.h = std::min(text_size.y, render_bounds->h / scaled);
			p.x = 0;
			p.y = 0;
			lay2 = math::convert(p);
			p.x += 1;
			p.y += 1;
			lay1 = math::convert(p);
			need_recalculate = false;
			calculated = true;
			w = vec2(render_bounds->size);
			wptr _THIS = get_ptr<label>();

			Graphics::Texture::ptr new_preview = cache.texture;
			lay2.right = std::ceil(lay2.right);
			lay2.bottom = std::ceil(lay2.bottom);

			if (lay2.right == 0) lay2.right = 1;
			if (lay2.bottom == 0) lay2.bottom = 1;

			if (!isnan(lay2.right))
				if (!cache.texture || cache.texture->get_desc().as_texture().Dimensions.x < lay2.right || cache.texture->get_desc().as_texture().Dimensions.y < lay2.bottom)
					cache.texture.reset(new Graphics::Texture(HAL::ResourceDesc::Tex2D(Format::R8G8B8A8_UNORM, { lay2.right, (UINT)lay2.bottom }, 1, 0, HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess)));

			auto _command_list = c.command_list_label;// c.command_list->get_sub_list();


			c.labeled->add([this, _THIS, _command_list]
				{
					ptr my_ptr = _THIS.lock();
					if (!my_ptr) return;
					auto command_list = const_cast<Graphics::CommandList*>(_command_list.get())->get_ptr<Graphics::CommandList>(); //wtf is this
					geomerty->set(command_list, convert(text.get()), font, font_size.get(), lay2, color, magnet_text);
					command_list->clear_rtv(cache.texture->texture_2d().renderTarget);
					auto rtv = cache.texture->texture_2d().renderTarget;
					command_list->get_graphics().set_rtv(1, rtv, Handle());

					PROFILE(L"label");

					Graphics::Viewport vps;
					vps.size = cache.texture->get_desc().as_texture().Dimensions;
					vps.pos = { 0,0 };
					vps.depths = { 0,1 };

					command_list->get_graphics().set_viewports({ vps });
					sizer_long s = { 0, 0, vps.size };
					command_list->get_graphics().set_scissors({ s });
					geomerty->draw(command_list, lay2, 0, { 0,0 });
					MipMapGenerator::get().generate(command_list->get_compute(), cache.texture);
				});
			cache.tc = vec4{ 0,0, lay2.right_bottom / vec2(cache.texture->get_desc().as_texture().Dimensions.x,cache.texture->get_desc().as_texture().Dimensions.y) };
		}

		Fonts::FontGeometry::ptr label::get_geometry()
		{
			return geomerty;
		}

		unsigned int label::get_index(vec2 at)
		{
			return geomerty->get_index(at);
		}

		void label::on_bounds_changed(const rect& r)
		{
			base::on_bounds_changed(r);

		}

		sizer label::update_layout(sizer r, float scale)
		{
			scaled = scale;
			return	base::update_layout(r, scale);
		}

	}
}