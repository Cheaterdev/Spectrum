	export module GUI:ScrollContainer;
import :Base;
import :ScrollBar;


export namespace GUI
{
	namespace Elements
	{

		class scroll_container : public base
		{
		public:
			GUI::Elements::scroll_bar::ptr vert;
			GUI::Elements::scroll_bar::ptr hor;
			GUI::base::ptr filled;
			GUI::base::ptr over_filled;
			bool allow_overflow = false;
			vec2 speed;
			vec2 fling_velocity;   // dp/s, set when a touch drag is released; decays in think()
		public:
			GUI::base::ptr contents;

			using ptr = s_ptr<scroll_container>;
			using wptr = w_ptr<scroll_container>;
			bool draw_background;
			bool auto_size = false;
			scroll_container();

			void add_child(base::ptr obj) override;

			void remove_child(base::ptr obj) override;

			virtual void resized();

			virtual sizer update_layout(sizer r, float scale) override;

		
			virtual void moving(vec2 pos);

			// Two-finger pinch over the content: ratio is the distance change since the last call,
			// center is in pixels; ended is set once when the fingers lift.
			virtual void on_pinch(vec2 center, float ratio, bool ended) {}


			virtual	bool on_wheel(mouse_wheel type, float value, vec2 pos) override;


			virtual void think(float dt) override;


		};

	}

}