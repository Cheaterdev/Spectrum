#pragma once
#include "../GUI.h"

namespace GUI
{
    namespace Elements
    {

        class colored_rect : public base
        {

            public:

                using ptr = s_ptr<colored_rect>;
                using wptr = w_ptr<colored_rect>;


                virtual void draw(DX12::context& c) override;
                float4 color;

        };
    }

}