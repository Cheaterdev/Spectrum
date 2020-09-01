#include "pch.h"
namespace GUI
{
    namespace Elements
    {
        namespace Debug
        {


            TaskViewer::TaskViewer()
            {
                draw_helper = true;
                TaskInfoManager::get().on_start_task.register_handler(this, [this](Task * task)
                {
					GUI::base::ptr row(new GUI::base);

					row->docking = dock::TOP;

					row->width_size = size_type::MATCH_CHILDREN;
					row->height_size = size_type::MATCH_CHILDREN;



                    GUI::Elements::label::ptr task_name(new GUI::Elements::label);
                    task_name->text = convert(task->name);
                    task_name->docking = GUI::dock::LEFT;
                   

					GUI::Elements::label::ptr task_percent(new GUI::Elements::label);
					task_percent->docking = GUI::dock::LEFT;

					task_percent->margin = {0,0,20,0};
				
					

					GUI::Elements::button::ptr butt_stop(new GUI::Elements::button);
					butt_stop->docking = GUI::dock::LEFT;

					butt_stop->on_click = [task](button::ptr) {
						task->active = false;
					};

					butt_stop->size = { 50,50 };
					row->add_child(task_percent);
					row->add_child(butt_stop);
					row->add_child(task_name);
				
					task->percent.register_change(row.get(),[this, task_percent](float p)
					{
						run_on_ui([task_percent,p]() {
							task_percent->text = std::to_string(int(p * 100));

						});
				
					});

						run_on_ui([this, row, task]()
					{
						std::lock_guard<std::mutex> g(m);
						auto&& elem = tasks[task];
						elem = row;
						contents->add_child(elem);
					});
                });
                TaskInfoManager::get().on_end_task.register_handler(this, [this](Task * task)
                {
                    run_on_ui([this, task]()
                    {
                        std::lock_guard<std::mutex> g(m);
                        auto&& elem = tasks[task];

                        if (elem)
                            elem->remove_from_parent();

                        elem = nullptr;
                    });
                });
            }

        }
    }

}