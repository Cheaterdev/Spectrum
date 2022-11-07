export module GUI:Debug.OutputWindow;
import :ScrollContainer;

export namespace GUI
{
    namespace Elements
    {
        namespace Debug
        {

            class OutputWindow: public scroll_container
            {

                    base::ptr current_block;

                    virtual void draw(Context& c) override;
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