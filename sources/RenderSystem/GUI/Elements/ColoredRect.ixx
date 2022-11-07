export module GUI:ColoredRect;
import :Base;

export namespace GUI
{
    namespace Elements
    {

        class colored_rect : public base
        {

            public:

                using ptr = s_ptr<colored_rect>;
                using wptr = w_ptr<colored_rect>;


                virtual void draw(Context& c) override;
                float4 color;

        };
    }

}