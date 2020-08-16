#include "Label.h"
namespace GUI
{
	namespace Elements
	{

		class scroll_container : public base
		{
		protected:
			GUI::Elements::scroll_bar::ptr vert;
			GUI::Elements::scroll_bar::ptr hor;
			GUI::base::ptr filled;
			GUI::base::ptr over_filled;
			bool allow_overflow = false;
			vec2 speed;
		public:
			GUI::base::ptr contents;

			using ptr = s_ptr<scroll_container>;
			using wptr = w_ptr<scroll_container>;
			bool draw_background;
			scroll_container();

			void add_child(base::ptr obj) override;

			void remove_child(base::ptr obj) override;

			void resized();

			virtual sizer update_layout(sizer r, float scale) override;

		
			void moving(vec2 pos);


			virtual	bool on_wheel(mouse_wheel type, float value, vec2 pos) override;


			virtual void think(float dt) override
			{
				base::think(dt);
				if (speed.length() > 0.3f)
				{
						moving(100 * speed*dt);	
						speed *= expf(-20 * dt);
			
				}
			}


		};

	}

}