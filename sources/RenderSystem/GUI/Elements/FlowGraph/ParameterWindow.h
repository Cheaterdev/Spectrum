#pragma once
//#include "GUI/Elements/CheckBoxText.h"

import GUI;

namespace GUI
{
	namespace Elements
	{

		inline base::ptr create_property_internal(Variable<bool>& elem)
		{
			auto label = std::make_shared<GUI::Elements::check_box_text>(elem);
			label->docking = GUI::dock::TOP;
			return label;
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


			auto label = std::make_shared<GUI::Elements::label>();
			label->text = elem.get_name();
			label->docking = GUI::dock::TOP;
			return label;
		}

	}
}