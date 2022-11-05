export module GUI:Resizable;
import :Base;
import :Resizer;


export namespace GUI
{
	namespace Elements
	{
		class resizable : public base
		{
		public:
			std::array<resizer::ptr, 10> resizers;

			using ptr = s_ptr<resizable>;
			using wptr = w_ptr<resizable>;
		
			resizable();

			void allow_resize(bool value)
			{
				for (auto r : resizers)
				{
					if (!r) continue;


					r->visible = value;
				}
			}
		};
	}
}