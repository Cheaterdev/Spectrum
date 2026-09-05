export module GUI:FlowGraph.ParameterWindow;

import Core;
import :ScrollContainer;
import :TabControl;
import :CheckBoxText;
import :FloatSlider;
import :EditText;
import :ComboBox;
import :Label;
import :Button;
import :HorizontalLayout;
import :FlowGraph.Canvas;
import TextSystem;



export namespace GUI
{
	namespace Elements
	{

		// Shared by every property row: a small button that resets `elem`
		// back to the value it was constructed with. Takes VariableBase&, not
		// a concrete Variable<T>&, so the same helper covers the enum row in
		// create_property() below too, where the concrete T isn't known.
		inline void add_revert_button(layouts::horizontal::ptr row, VariableBase& elem)
		{
			auto revert = std::make_shared<GUI::Elements::button>();
			revert->size = { 45, 0 };
			revert->get_label()->text = "Reset";
			revert->on_click = [&elem](button::ptr) { elem.reset_to_default(); };
			row->add_child(revert);
		}

		inline base::ptr create_property_internal(Variable<bool>& elem)
		{
			auto row = std::make_shared<GUI::Elements::layouts::horizontal>();
			row->docking = GUI::dock::TOP;
			row->x_type = GUI::pos_x_type::LEFT;

			auto check = std::make_shared<GUI::Elements::check_box_text>(elem);
			row->add_child(check);

			// check_box_text(Variable<bool>&) only seeds the checkbox's displayed
			// state once, at construction time -- if elem changes from elsewhere
			// (the revert button below, or any other code path touching this
			// same Variable), the checkbox would otherwise go stale silently.
			elem.on_change.register_handler(row.get(), [check](bool v) { check->get_check()->set_checked(v); });

			add_revert_button(row, elem);

			return row;
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
				// Keep the thumb in sync if elem changes from elsewhere (revert
				// button, other code) -- same reasoning as the bool row above.
				// property<float>::operator= only fires the slider's own
				// on_change when the value actually differs, so writing back
				// the value this handler was itself just notified of is a
				// harmless no-op, not a feedback loop.
				elem.on_change.register_handler(row.get(), [slider](float v) { slider->value = v; });
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
				elem.on_change.register_handler(row.get(), [edit](float v) { edit->set_text(std::to_string(v)); });
				row->add_child(edit);
			}

			add_revert_button(row, elem);

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

				// Keep the displayed selection in sync if elem changes from
				// elsewhere (revert button, other code) -- same reasoning as
				// the bool/float rows above, but through VariableBase's
				// type-erased on_changed rather than a typed on_change: elem
				// here is a VariableBase&, not the concrete Variable<EnumT>&,
				// so re-reading get_enum_index() is the only option.
				elem.on_changed.register_handler(row.get(), [&elem, combo, enum_names]()
				{
					int i = elem.get_enum_index();
					if (i >= 0 && i < (int)enum_names.size())
						combo->get_label()->text = enum_names[i];
				});

				add_revert_button(row, elem);

				return row;
			}


			auto label = std::make_shared<GUI::Elements::label>();
			label->text = elem.get_name();
			label->docking = GUI::dock::TOP;
			return label;
		}

	}
}