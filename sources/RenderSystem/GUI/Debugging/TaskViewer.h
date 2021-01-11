#pragma once
#include "../Elements/ScrollContainer.h"
#include "Log/Tasks.h"


namespace GUI
{
    namespace Elements
    {
        namespace Debug
        {
            class TaskViewer : public  GUI::Elements::scroll_container
            {

                    std::map<Task*, base::ptr> tasks;
                    std::mutex m;
                public:
                    using ptr = s_ptr<TaskViewer>;

                    TaskViewer();

            };
        }
    }
}