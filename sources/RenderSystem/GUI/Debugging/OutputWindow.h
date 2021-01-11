#pragma once
#include "../Elements/ScrollContainer.h"


namespace GUI
{
    namespace Elements
    {
        namespace Debug
        {

            class OutputWindow: public scroll_container
            {

                    base::ptr current_block;

                    virtual void draw(DX12::context& c) override;
                    std::list< base::ptr> add;
                    std::mutex m;
                    virtual void think(float) override;
                public:
                    using ptr = s_ptr<OutputWindow>;

                    OutputWindow();
            };
        }
    }
}