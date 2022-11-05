#include "pch_render.h"
#include "ColoredRect.h"
import GUI;

namespace GUI
{
	namespace Elements
	{

		void colored_rect::draw(Graphics::context& c)
		{
			renderer->draw_color(c, color, get_render_bounds());
		}
	}

}