module GUI:CheckBoxText;

import GUI;
namespace GUI
{
	namespace Elements
	{
		class clickable_label : public label
		{
			base* owner;
		public:
			clickable_label(base* owner) : owner(owner)
			{
				clickable = true;
			}

			virtual bool on_mouse_move(vec2 pos) override
			{
				return owner->on_mouse_move(pos);
			}

			virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos) override
			{
				return owner->on_mouse_action(action, button, pos);
			}

			virtual void on_mouse_enter(vec2 pos) override
			{
				owner->on_mouse_enter(pos);
			}

			virtual void on_mouse_leave(vec2 pos) override
			{
				owner->on_mouse_leave(pos);
			}
		};

		label::ptr check_box_text::get_label()
		{
			return label_text;
		}

		bool check_box_text::is_checked()
		{
			return check->is_checked();
		}

		toggle_switch::ptr check_box_text::get_check()
		{
			return check;
		}

		check_box_text::check_box_text()
		{
			check.reset(new toggle_switch());
			check->margin = { 0, 0, 3, 0 };
			check->on_toggle = [this](bool value) { if (on_check) on_check(value); };

			label_text.reset(new clickable_label(check.get()));
			label_text->text = "checkbox";
			label_text->magnet_text = FW1_LEFT | FW1_VCENTER;
			add_child(check);
			add_child(label_text);
		}

		check_box_text::check_box_text(Variable<bool>& v)
		{
			check.reset(new toggle_switch());
			check->margin = { 0, 0, 3, 0 };
			check->set_checked((bool)v);
			check->on_toggle = [&v](bool value) { v = value; };

			label_text.reset(new clickable_label(check.get()));
			label_text->magnet_text = FW1_LEFT | FW1_VCENTER;
			add_child(check);
			add_child(label_text);
			label_text->text = v.get_name();
			docking = GUI::dock::TOP;
			x_type = GUI::pos_x_type::LEFT;
		}
	}
}
