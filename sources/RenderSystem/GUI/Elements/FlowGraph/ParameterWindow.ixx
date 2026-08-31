export module GUI:FlowGraph.ParameterWindow;

import :ScrollContainer;
import :TabControl;
import :CheckBoxText;
import :FloatSlider;
import :EditText;
import :ComboBox;
import :Label;
import :HorizontalLayout;
import :FlowGraph.Canvas;
import TextSystem;



export namespace GUI
{
	namespace Elements
	{

		inline base::ptr create_property_internal(Variable<bool>& elem)
		{
			auto label = std::make_shared<GUI::Elements::check_box_text>(elem);
			label->docking = GUI::dock::TOP;
			return label;
		}

		// Constrained (constructed with the {min, max} overload) gets a slider;
		// unconstrained has no range to hand that widget, so it gets a free-form
		// numeric text box instead -- either way a Variable<float> is editable,
		// unlike the plain read-only label other not-specifically-handled types get.
		inline base::ptr create_property_internal(Variable<float>& elem)
		{
			auto row = std::make_shared<GUI::Elements::layouts::horizontal>();
			row->docking = GUI::dock::TOP;
			row->x_type = GUI::pos_x_type::LEFT;

			auto label = std::make_shared<GUI::Elements::label>();
			label->text = elem.get_name();
			row->add_child(label);

			if (elem.has_range())
			{
				auto slider = std::make_shared<GUI::Elements::float_slider>();
				slider->min = elem.get_min();
				slider->max = elem.get_max();
				slider->value = (float)elem;
				slider->on_change = [&elem](float value) { elem = value; };
				row->add_child(slider);
			}
			else
			{
				auto edit = std::make_shared<GUI::Elements::edit_text>();
				edit->size = { 60, 0 };
				edit->set_text(std::to_string((float)elem));
				// Only characters a float literal can contain -- on_change still
				// fires per keystroke (edit_text has no commit-on-enter), so an
				// in-progress partial value like "-" or "1." is expected and just
				// silently skipped below rather than applied.
				edit->filter = [](char ch) { return (ch >= '0' && ch <= '9') || ch == '.' || ch == '-'; };
				edit->on_change = [&elem](const std::string& text)
				{
					try { elem = std::stof(text); } catch (...) {}
				};
				row->add_child(edit);
			}

			return row;
		}


		template<class T>
		inline base::ptr create_property_internal(T& elem)
		{
			auto label = std::make_shared<GUI::Elements::label>();
			label->text = elem.get_name();
			label->docking = GUI::dock::TOP;
			return label;
		}

#define CHECK_PROPERTY(x) {  auto e = dynamic_cast<Variable<x>*>(&elem);    if (e) return create_property_internal(*e);}
		template<class T>
		inline base::ptr create_property(T& elem)
		{

			CHECK_PROPERTY(bool);
			CHECK_PROPERTY(float);

			// Enums: dispatched through VariableBase's virtuals, not CHECK_PROPERTY
			// -- there's no concrete enum type to dynamic_cast to here, since any
			// number of distinct enum types can back a Variable. A non-enum
			// Variable reports an empty name list, so this is a no-op for those.
			auto enum_names = elem.get_enum_names();
			if (!enum_names.empty())
			{
				auto row = std::make_shared<GUI::Elements::layouts::horizontal>();
				row->docking = GUI::dock::TOP;
				row->x_type = GUI::pos_x_type::LEFT;

				auto label = std::make_shared<GUI::Elements::label>();
				label->text = elem.get_name();
				row->add_child(label);

				auto combo = std::make_shared<GUI::Elements::combo_box>();
				int current = elem.get_enum_index();
				for (size_t i = 0; i < enum_names.size(); i++)
				{
					auto item = combo->add_item(enum_names[i]);
					int index = (int)i;
					item->on_select = [&elem, index]() { elem.set_enum_index(index); };
				}
				if (current >= 0 && current < (int)enum_names.size())
					combo->get_label()->text = enum_names[current];

				// combo_box has no autosize-to-content -- it's a fixed-size (width_size
				// == NONE, docked LEFT) element, so it keeps its 25px ctor default
				// regardless of what's inside it. Preset a width from the widest option
				// text (same font/size the label itself uses), so option names like
				// "Quad-shared blocker search" aren't clipped in the closed box or the
				// open dropdown, which mirrors this same width for its own items.
				{
					auto font = Fonts::FontSystem::get().get_font("Segoe UI Light");
					float max_text_w = 0;
					for (auto& name : enum_names)
						max_text_w = std::max(max_text_w, font->measure(name, 16).x);

					const float combo_padding = 5 + 30; // combo_box's own padding.left + padding.right
					combo->size = { max_text_w + combo_padding, combo->size->y };
				}
				row->add_child(combo);

				return row;
			}


			auto label = std::make_shared<GUI::Elements::label>();
			label->text = elem.get_name();
			label->docking = GUI::dock::TOP;
			return label;
		}

	}
}