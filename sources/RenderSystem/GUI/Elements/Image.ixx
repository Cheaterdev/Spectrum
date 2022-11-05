export module GUI:Image;
import :Base;

export namespace GUI
{
    namespace Elements
    {
        class image : public base
        {
            public:
                using ptr = s_ptr<image>;
                using wptr = w_ptr<image>;

                Texture texture;
                virtual void draw(Graphics::context& c);
                image()
                {
                    clickable = false;
                }
        };



    }
}