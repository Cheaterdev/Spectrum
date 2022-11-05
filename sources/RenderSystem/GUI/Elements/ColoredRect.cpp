module GUI:ColoredRect;
import :Renderer;

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