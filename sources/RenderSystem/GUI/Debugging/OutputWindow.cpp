#include "pch_render.h"

#include "OutputWindow.h"
#include "GUI/Elements/ScrollContainer.h"
import GUI;

namespace GUI
{
    namespace Elements
    {
        namespace Debug
        {


            void OutputWindow::think(float dt)
            {
                std::lock_guard<std::mutex> g(m);
                //	run_on_ui([this]
                //	{

                while (add.size())
                {
                    auto item = add.front();
                    add.pop_front();

                    if (!current_block || current_block->get_childs().size() >= 100)
                    {
                        current_block.reset(new base());
                        current_block->docking = dock::TOP;
                        current_block->x_type = pos_x_type::LEFT;
                        current_block->width_size = size_type::MATCH_CHILDREN;
                        current_block->height_size = size_type::MATCH_CHILDREN;
                        add_child(current_block);
                    }

                    current_block->add_child(item);
                }

                scroll_container::think(dt);
                //});
            }
            void OutputWindow::draw(Graphics::context& c)
            {
                renderer->draw_area(get_ptr(), c);
            }

            OutputWindow::OutputWindow()
            {
                contents->width_size = size_type::MATCH_PARENT_CHILDREN;
                contents->height_size = size_type::MATCH_CHILDREN;
                padding = { 5, 5, 5, 5 };
                thinkable = true;
                Log::get().on_log.register_handler(this, [this](const LogBlock * block)
                {
                    label::ptr item(new label);
                    item->docking = dock::TOP;
                    item->text = block->get_string();
                    item->x_type = pos_x_type::LEFT;
                    item->magnet_text = FW1_TEXT_FLAG::FW1_LEFT | FW1_TEXT_FLAG::FW1_VCENTER;

                    if (block->get_level_internal() == log_level_internal::level_error)
                        item->color = rgba8(200, 0, 0, 255);

                    if (block->get_level_internal() == log_level_internal::level_debug)
                        item->color = rgba8(0, 100, 0, 255);

                    if (block->get_level_internal() == log_level_internal::level_warning)
                        item->color = rgba8(150, 150, 0, 255);

                    if (block->get_level_internal() == log_level_internal::level_none)
                        item->color = rgba8(200, 200, 200, 255);

                    if (block->get_level_internal() == log_level_internal::level_info)
                        item->color = rgba8(0, 0, 200, 255);

                    std::lock_guard<std::mutex> g(m);
                    add.emplace_back(item);
                });
            }
        }
    }

}