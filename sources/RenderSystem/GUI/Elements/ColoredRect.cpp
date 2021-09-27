#include "pch.h"
#include "ColoredRect.h"
#include "GUI/Renderer/Renderer.h"

namespace GUI
{
	namespace Elements
	{

		void colored_rect::draw(Render::context& c)
		{
			renderer->draw_color(c, color, get_render_bounds());
		}
	}

}