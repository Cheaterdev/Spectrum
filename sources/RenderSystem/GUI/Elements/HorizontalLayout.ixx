export module GUI:HorizontalLayout;
import :Base;

export namespace GUI
{
	namespace Elements
	{

		namespace layouts
		{
			class horizontal : public base
			{
			public:
				using ptr = s_ptr<horizontal>;
				using wptr = w_ptr<horizontal>;

				horizontal();

				void on_bounds_changed(const rect &r) override;
				virtual sizer update_layout(sizer r,float scale) override;

		
				virtual void add_child(base::ptr obj) override;

			};
		}
	
	}
}