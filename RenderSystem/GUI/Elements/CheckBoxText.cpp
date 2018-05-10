#include "pch.h"
namespace GUI
{
	namespace Elements
	{

		class clickable_label : public label
		{
			check_box* owner;
		public:
			clickable_label(check_box* owner)
			{
				this->owner = owner;
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

		check_box::ptr check_box_text::get_check()
		{
			return check;
		}

		check_box_text::check_box_text()
		{
			check.reset(new check_box());
			check->margin = { 0, 0, 3, 0 };
			check->on_check = [this](bool value) {if (on_check) on_check(value); };

			label_text.reset(new clickable_label(check.get()));
			label_text->text = "this is checkbox!";
			label_text->magnet_text = FW1_LEFT | FW1_VCENTER;
			//label_text->size = { label_text->size->x, 200 };
			//	label_text->docking = dock::FILL;
			add_child(check);
			add_child(label_text);
		}

		check_box_text::check_box_text(Variable<bool>& v)
		{
			check.reset(new check_box(v));
			check->margin = { 0, 0, 3, 0 };
		//	check->on_check = [this](bool value) {if (on_check) on_check(value); };

			label_text.reset(new clickable_label(check.get()));
			label_text->magnet_text = FW1_LEFT | FW1_VCENTER;
			//label_text->size = { label_text->size->x, 200 };
			//	label_text->docking = dock::FILL;
			add_child(check);
			add_child(label_text);
			label_text->text = v.get_name();
			docking = GUI::dock::TOP;
			x_type = GUI::pos_x_type::LEFT;
		}
	}
}
