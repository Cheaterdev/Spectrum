export module GUI:Debug.TaskViewer;
import :ScrollContainer;

import Tasks;


export namespace GUI
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