export module GUI:Table;

import :Base;
import :ScrollContainer;
import :Image;
import :Label;
import :Resizer;


import :Button;

import :ComboBox;
export namespace GUI
{

    namespace Elements
    {


		class Table :public GUI::Elements::scroll_container
		{
			base::ptr top_row;
			base::ptr left_column;



		public:

			class Header :public base
			{
				Table* table;
				Skin::StatusBar skin;
			public:
				using ptr = std::shared_ptr<Header>;
				//	Elements::label::ptr header;
				Elements::resizer::ptr dragger;

				void init_label(const std::string& name);

				void draw(Context& c) override;

				Header(const std::string& name, bool top, Table* table);

				virtual void on_bounds_changed(const rect& r) override;
			};




			std::vector<Header::ptr> columns;
			std::vector<Header::ptr> rows;
			std::vector<base::ptr> cells;
			using ptr = std::shared_ptr<Table>;
			base::ptr layer2;
			base::ptr layer3;


			void recalculate();


			Table();

			void AddColumn(const std::string& name);

			uint AddRow(std::string name);

			void SetCell(uint2 pos, base::ptr elem);

			void InitTest();

		};
    }
}