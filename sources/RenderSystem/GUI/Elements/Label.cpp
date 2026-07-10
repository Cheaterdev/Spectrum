module GUI:Label;
import RenderSystem;


import HAL;
import GUI;
import Graphics;

import aho_corasick;
using namespace HAL;



namespace GUI
{
	namespace Elements
	{

		void label::on_pre_render(Context& context)
		{
			recalculate(context);

		}

		void label::draw(Context& c)
		{
			  base::draw(c);
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
					{
					//	 if(GetAsyncKeyState('U'))
						ASSERT(cache.texture.texture2D && "label::draw: cache texture not initialized — recalculate not called?");
						c.renderer->draw(c, cache, p);
					//	 else
					//		 geomerty->draw(c.command_list, c.ui_clipping, 0, p.pos);
					}
				}

			c.ui_clipping = orig;
		}



		label::~label()
		{
			font = nullptr;
		}

		label::label() : text(std::bind(&label::on_text_changed, this, std::placeholders::_1)), font_size(std::bind(&label::on_size_changed, this, std::placeholders::_1))
		{
			clickable = false;
			magnet_text = FW1_LEFT | FW1_TOP | FW1_NOWORDWRAP;
			font = Fonts::FontSystem::get().get_font("Segoe UI Light");
			color = float4(1, 1, 1, 1);
			geomerty.reset(new Fonts::FontGeometry());
			geomerty_shadow.reset(new Fonts::FontGeometry());
			font_size = 16;
			text_size = font->measure("", font_size.get() /** scaled*/, magnet_text)+ vec2(2, 2);
			text_size.x = std::ceil(text_size.x);
			text_size.y = std::ceil(text_size.y);
		}

		void label::on_text_changed(const std::string& str)
		{
			text_size = font->measure(str, font_size.get() /** scaled*/, magnet_text)+ vec2(2, 2);
			size = text_size;
			need_recalculate = true;
		}
		void label::on_size_changed(const float& new_size)
		{
			on_text_changed(text.get());
		}
		void label::recalculate(Context& c)
		{

			if (!need_recalculate && ((w == ivec2(render_bounds->size / scaled) /*|| (magnet_text & FW1_NOWORDWRAP)*/)))
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

			HAL::Texture::ptr new_preview = cache_resource;
			lay2.right = std::ceil(lay2.right);
			lay2.bottom = std::ceil(lay2.bottom);

			if (lay2.right == 0) lay2.right = 1;
			if (lay2.bottom == 0) lay2.bottom = 1;

			if (!isnan(lay2.right))
				if (!cache_resource || cache_resource->get_desc().as_texture().Dimensions.x < lay2.right || cache_resource->get_desc().as_texture().Dimensions.y < lay2.bottom)
				{
					cache_resource.reset(new HAL::Texture(RenderSystem::get().device(), HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { lay2.right, (UINT)lay2.bottom }, 1, 0, HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess)));
					cache_resource->resource->set_name("Label::cache");
					cache.texture = cache_resource->texture_2d();
				}
			//auto _command_list = c.command_list_label;// c.command_list->get_sub_list();
			user_ui->pre_draw_infos.emplace_back(this);

			cache.tc = vec4{ 0,0, lay2.right_bottom / vec2(cache_resource->get_desc().as_texture().Dimensions.x, cache_resource->get_desc().as_texture().Dimensions.y) };
		}
		void label::pre_draw(HAL::CommandList::ptr command_list)
		{
//		cache.texture->resource->get_state_manager().prepare_state(command_list.get(), { HAL::BarrierSync::NONE, HAL::BarrierAccess::NO_ACCESS, HAL::TextureLayout::SHADER_RESOURCE });	
			//						   command_list->transition(cache.texture->resource, );
			geomerty->clear();

				  /*
			std::map<std::string, rgba8> tokens;
			tokens["{"] = rgba8(120, 120, 255, 255);
			tokens["}"] = rgba8(120, 120, 255, 255);
				tokens["->"] = rgba8(120, 120, 255, 255);
				 tokens[":"] = rgba8(120, 120, 255, 255);
				    tokens[";"] = rgba8(120, 120, 255, 255);
					 tokens[","] = rgba8(120, 120, 255, 255);
					 tokens["+"] = rgba8(120, 120, 255, 255);
tokens["-"] = rgba8(120, 120, 255, 255);
tokens["*"] = rgba8(120, 120, 255, 255);
	   tokens["/"] = rgba8(120, 120, 255, 255);
		   tokens["="] = rgba8(120, 120, 255, 255);
			   tokens["."] = rgba8(120, 120, 255, 255);
				   tokens["&"] = rgba8(120, 120, 255, 255);

			tokens["("] = rgba8(120, 120, 255, 255);
			tokens[")"] = rgba8(120, 120, 255, 255);

			tokens["\""] = rgba8(100, 255, 100, 255);
			tokens["\'"] = rgba8(100, 255, 100, 255);

			tokens["//"] = rgba8(100, 100, 100, 255);
			tokens["/*"] = rgba8(100, 100, 100, 255);
			tokens["import"] = rgba8(255, 0, 255, 255);
		 			tokens["for"] = rgba8(255, 0, 255, 255);
					 			tokens["if"] = rgba8(255, 0, 255, 255);
					    		tokens["else"] = rgba8(255, 0, 255, 255);
					    			tokens["return"] = rgba8(255, 0, 255, 255);
	   				    			tokens["auto"] = rgba8(255, 0, 255, 255);
				    			tokens["void"] = rgba8(255, 0, 255, 255);
								 				    			tokens["struct"] = rgba8(255, 0, 255, 255);
			tokens["class"] = rgba8(255, 0, 255, 255);
									   		    			tokens["public"] = rgba8(255, 0, 255, 255);
		    			tokens["private"] = rgba8(255, 0, 255, 255);
						tokens["protected"] = rgba8(255, 0, 255, 255);
					 		  	tokens["new"] = rgba8(255, 0, 255, 255);
					 		     	tokens["using"] = rgba8(255, 0, 255, 255);
					 				 tokens["const"] = rgba8(255, 0, 255, 255);
					 

									bool override_color = false;
									float4 _color;
			aho_corasick::trie trie;
			trie.remove_overlaps()
				.only_whole_words();
   // .case_insensitive();

for (auto& token : tokens)
{
	trie.insert(token.first);
}


auto parsed = trie.tokenise(text.get());
//std::stringstream html;
//html << "<html><body><p>";
std::stringstream res;
float4 c = float4(color)/float4(255,255,255,255);

auto l = lay2;
bool quote = false;
for (const auto& token : parsed) {
	

		if (token.is_match())
		{
			c = float4(tokens[token.get_fragment()])/float4(255,255,255,255);
			if (token.get_fragment() == "//" || token.get_fragment() == "/*")
			{
				override_color = true;
				_color = c;
			}
				if (token.get_fragment() == "\""||token.get_fragment() == "\'" )
			{
				
				quote = !quote;
					override_color = quote;
				_color = c;
			}
			//else
				//override_color = false;
		}
		else 
			c = color;



	if (override_color)
		c = _color;

				   
		geomerty->add(command_list, convert(token.get_fragment()), font, font_size.get(), l, c, magnet_text);

		res << token.get_fragment();

		l.left = font->measure(res.str(), font_size.get() /** scaled/, magnet_text).x;
	//html << token.get_fragment();
	//if (token.is_match()) html << "</i>";
}
//html << "</p></body></html>";
//std::cout << html.str();
			//text.get().fi
								 /*/
			//command_list->clear_rtv(cache.texture.renderTarget);
		   geomerty->set(command_list, convert(text.get()), font, font_size.get(), lay2, color, magnet_text);
		
			{
				RT::SingleColor rt;
				rt.GetColor() = cache.texture.renderTarget;
				command_list->get_graphics().set_rtv(rt,RTOptions::Default| RTOptions::ClearAll);
			}

			PROFILE(L"label");
			geomerty->draw(command_list, lay2, 0, { 0,0 });
			MipMapGenerator::get().generate(command_list->get_compute(), cache.texture);

								//	   command_list->transition(cache.texture->resource, { HAL::BarrierSync::NONE, HAL::BarrierAccess::NO_ACCESS, HAL::TextureLayout::SHADER_RESOURCE });

		};
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

			
		void MultiLineLabel::on_text_changed(const std::string& str)
		{

			contents->remove_all();
			const std::string delim = "\n";

    for (const auto word : std::views::split(str, delim))
    {
		auto word_view = std::string_view(word);
		auto line = std::make_shared<label>();
		line->text = std::string(word_view.substr(0,word_view.length()-1));
		line->docking = dock::TOP;
		add_child(line);


    }
   
		}

		MultiLineLabel::MultiLineLabel()	: text(std::bind(&MultiLineLabel::on_text_changed, this, std::placeholders::_1))
		{


		}
	}
}