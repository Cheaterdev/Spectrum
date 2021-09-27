#pragma once
#include "Image.h"

namespace GUI
{
	namespace Elements
	{
	
		class circle_selector : public image
		{
			bool movable = false;
			image::ptr center;
		public:
		
			
			using ptr = s_ptr<circle_selector>;
			using wptr = w_ptr<circle_selector>;

		
			//virtual void draw(Render::context& c) override;
			circle_selector();
			Events::Event<float2> on_change;

			virtual bool on_mouse_move(vec2 pos) override;


			virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override;

			void set_value(float2);
		};


	}
}
