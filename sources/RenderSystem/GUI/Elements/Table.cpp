module GUI:Table;

import GUI;
import :Renderer;

namespace GUI
{
    namespace Elements
    {

        void Table::Header::init_label(const std::string& name)
        {
            auto lbl = std::make_shared<Elements::label>();
            lbl->docking = dock::FILL;
            lbl->text = name;
            add_child(lbl);
        }

        void Table::Header::draw(Context& c)
        {
            c.renderer->draw_color(c, float4(53, 114, 202, 255) / 255.0f, get_render_bounds());
        }

        Table::Header::Header(const std::string& name, bool top, Table* table) : table(table)
        {
            skin = Skin::get().DefaultStatusBar;
            init_label(name);
            if (top)
            {
                docking = dock::LEFT;
                width_size = size_type::FIXED;
                height_size = size_type::FIXED;
                minimal_size.x = 100;
                size = { 32,28 };
                y_type = pos_y_type::TOP;
            }
            else
            {
                docking = dock::TOP;
                height_size = size_type::FIXED;
                width_size = size_type::FIXED;

                size = { 32,28 };
                x_type = pos_x_type::LEFT;
            }
            minimal_size.y = 28;
            padding = { 4,4,4,4 };
            draw_helper = true;

            dragger.reset(new Elements::resizer());
            if (top)
            {
                dragger->docking = dock::LEFT;
                dragger->height_size = size_type::MATCH_PARENT;
                dragger->dir = Elements::direction::RIGHT;
                dragger->y_type = pos_y_type::TOP;
            }
            else
            {
                dragger->docking = dock::TOP;
                dragger->width_size = size_type::MATCH_PARENT;
                dragger->dir = Elements::direction::BOTTOM;
                dragger->x_type = pos_x_type::LEFT;
            }

            dragger->target = this;
            dragger->draw_color = true;
        }

        void Table::Header::on_bounds_changed(const rect& r)
        {
            base::on_bounds_changed(r);
            table->recalculate();
        }

        void Table::recalculate()
        {
            auto top_left = columns.front()->get_render_bounds();
            auto left_top = rows.front()->get_render_bounds();

            for (auto&& c : rows)
                c->size = { top_left.w, c->size.get().y };

            for (auto&& c : columns)
                c->size = { c->size.get().x,left_top.h };

            for (uint i = 0; i < rows.size() - 1; i++)
            {
                for (uint j = 0; j < columns.size() - 1; j++)
                {
                    auto cell = cells[j + i * (columns.size() - 1)];
                    if (cell)
                    {
                        cell->pos = float2{ columns[j + 1]->get_render_bounds().x, rows[i + 1]->get_render_bounds().y } - contents->get_render_bounds().pos;
                        cell->size = float2{ columns[j + 1]->get_render_bounds().w, rows[i + 1]->get_render_bounds().h };
                    }

                }
            }
        }

        Table::Table()
        {
            base::ptr layer1 = std::make_shared<base>();
            layer1->docking = dock::FILL;
            layer1->clickable = false;
            add_child(layer1);


            layer2 = std::make_shared<base>();
            layer2->docking = dock::NONE;
            layer2->size = { 0,0 };
            layer2->width_size = size_type::MATCH_PARENT;
            layer2->height_size = size_type::MATCH_CHILDREN;
            layer2->clickable = false;
            add_child(layer2);

            layer3 = std::make_shared<base>();
            layer3->docking = dock::NONE;
            layer3->size = { 0,0 };
            layer3->width_size = size_type::MATCH_CHILDREN;
            layer3->height_size = size_type::MATCH_PARENT;
            layer3->clickable = false;
            layer3->debug = true;
            add_child(layer3);

            // get rid of this
            AddRow("");
            AddColumn("");
            cells.resize(0);
        }

        void Table::AddColumn(const std::string& name)
        {
            columns.emplace_back(std::make_shared<Header>(name, true, this));
            layer3->add_child(columns.back());
            layer3->add_child(columns.back()->dragger);
        }

        uint Table::AddRow(std::string name)
        {
            cells.resize(cells.size() + columns.size(), nullptr);
            rows.emplace_back(std::make_shared<Header>(name, false, this));
            layer2->add_child(rows.back());
            layer2->add_child(rows.back()->dragger);

            return rows.size() - 1;
        }

        void Table::SetCell(uint2 pos, base::ptr elem)
        {
            cells[pos.x + pos.y * (columns.size() - 1)] = (elem);
            contents->add_child(elem);
        }

        void Table::InitTest()
        {
            ::Table::Camera cam;

            AddColumn("Name");
            AddColumn("Time");
            AddColumn("Count");

            for (uint i = 0;i < 20;i++)
            {
                AddRow(std::to_string(i));

                for (uint j = 0;j < 3;j++)
                {
                    uint r = rand() % 3;

                    if (r == 0) {
                        auto label = std::make_shared<Elements::label>();
                        label->text = std::to_string(i) + "_" + std::to_string(j);
                        SetCell(uint2(j, i), label);
                    }
                    if (r == 1) {
                        auto button = std::make_shared<Elements::button>();
                        button->get_label()->text = std::to_string(i) + "_" + std::to_string(j);
                        SetCell(uint2(j, i), button);
                    }

                    if (r == 2) {
                        auto combo_box = std::make_shared<Elements::combo_box>();
                        combo_box->get_label()->text = std::to_string(i) + "_" + std::to_string(j);

                        combo_box->add_item("1");
                        combo_box->add_item("2");
                        combo_box->add_item("3");

                        SetCell(uint2(j, i), combo_box);
                    }
                }

            }
        }

    }
}

