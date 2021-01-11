#pragma once
#include "HorizontalLayout.h"
#include "Label.h"
#include "CheckBox.h"
#include "Log/Events.h"
namespace GUI
{
	namespace Elements
	{

		class check_box_text : public layouts::horizontal
		{
			check_box::ptr check;
			label::ptr label_text;


		protected:
		//	virtual void on_check_changed(const bool &r);
		public:
			using ptr = s_ptr<check_box_text>;
			using wptr = w_ptr<check_box_text>;
			std::function<void(bool)> on_check;
			check_box_text();
			check_box_text(Variable<bool>&);

			check_box::ptr get_check();
			bool is_checked();
			label::ptr get_label();
		};
	}
}