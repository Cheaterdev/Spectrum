export module GUI:CheckBoxText;
import :Image;
import :Skin;
import :HorizontalLayout;
import :Label;
import :ToggleSwitch;

export namespace GUI
{
	namespace Elements
	{

		class check_box_text : public layouts::horizontal
		{
			toggle_switch::ptr check;
			label::ptr label_text;

		public:
			using ptr = s_ptr<check_box_text>;
			using wptr = w_ptr<check_box_text>;
			std::function<void(bool)> on_check;
			check_box_text();
			check_box_text(Variable<bool>&);

			toggle_switch::ptr get_check();
			bool is_checked();
			label::ptr get_label();
		};
	}
}