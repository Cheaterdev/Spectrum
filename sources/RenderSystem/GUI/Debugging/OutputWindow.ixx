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
                    // Every line is a label; without a cap a log spammed per draw
                    // grows the process by gigabytes.
                    static constexpr size_t LINES_PER_BLOCK = 100;
                    static constexpr size_t MAX_BLOCKS = 50;
                    std::deque<base::ptr> blocks;

                    virtual void draw(Context& c) override;
                    std::list< base::ptr> add;
                    std::mutex m;
                    virtual void think(float) override;
                public:
                    using ptr = s_ptr<OutputWindow>;

                    // File references in log lines ("path:line:col", "path(line,col)")
                    // to files that exist become links; clicking one calls this with
                    // the path and the 1-based line/column. UI thread.
                    static inline std::function<void(std::string file, int line, int column)> on_open_file;

                    OutputWindow();

                ~OutputWindow();
            };
        }
    }
}