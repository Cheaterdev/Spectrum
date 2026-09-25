module GUI:Debug.OutputWindow;
import :Renderer;
import :Label;
import TextEngine;

namespace
{
    struct FileRef
    {
        Text::Link  span;
        std::string file;
        int         line = 0;
        int         column = 0;
    };

    bool is_path_char(char c)
    {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == '.' || c == '/' || c == '\\';
    }

    int read_number(const std::string& s, size_t& i)
    {
        int value = 0;
        const size_t start = i;
        while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i])))
            value = value * 10 + (s[i++] - '0');
        return i > start ? value : -1;
    }

    // Source references as compilers print them: "path:line[:col]" (DXC/clang)
    // and "path(line[,col])" (MSVC, Prism). Only paths with a source extension
    // that exist on disk count, so ordinary "a:1" text doesn't turn into links.
    std::vector<FileRef> find_file_refs(const std::string& s)
    {
        static constexpr std::string_view extensions[] = { ".hlsli", ".hlsl", ".prism", ".ixx", ".cpp", ".h" };

        std::vector<FileRef> refs;
        size_t i = 0;
        while (i < s.size())
        {
            size_t ext_end = std::string::npos;
            for (auto ext : extensions)
                if (s.compare(i, ext.size(), ext) == 0 && (i + ext.size() >= s.size() || !std::isalnum(static_cast<unsigned char>(s[i + ext.size()]))))
                {
                    ext_end = i + ext.size();
                    break;
                }

            if (ext_end == std::string::npos)
            {
                ++i;
                continue;
            }

            size_t begin = i;
            while (begin > 0 && is_path_char(s[begin - 1])) --begin;
            // Windows drive letter: "C:\..."
            if (begin >= 2 && s[begin - 1] == ':' && std::isalpha(static_cast<unsigned char>(s[begin - 2])))
                begin -= 2;

            FileRef ref;
            ref.file = s.substr(begin, ext_end - begin);

            size_t e = ext_end;
            if (e < s.size() && s[e] == ':')
            {
                ++e;
                ref.line = read_number(s, e);
                if (ref.line > 0 && e + 1 < s.size() && s[e] == ':' && std::isdigit(static_cast<unsigned char>(s[e + 1])))
                {
                    ++e;
                    ref.column = read_number(s, e);
                }
            }
            else if (e < s.size() && s[e] == '(')
            {
                ++e;
                ref.line = read_number(s, e);
                if (ref.line > 0 && e < s.size() && s[e] == ',')
                {
                    ++e;
                    ref.column = read_number(s, e);
                }
                if (e < s.size() && s[e] == ')') ++e;
            }

            std::error_code ec;
            if (ref.line > 0 && std::filesystem::exists(ref.file, ec))
            {
                ref.span = { uint32_t(begin), uint32_t(e) };
                refs.push_back(std::move(ref));
            }
            i = std::max(e, ext_end);
        }
        return refs;
    }
}


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
            void OutputWindow::draw(Context& c)
            {
                c.renderer->draw_area(get_ptr(), c);
            }

            OutputWindow::~OutputWindow()
            {
                Log::get()<<"DELETING"<<Log::endl;
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
                        item->color = float4(200, 0, 0, 255)/float4(255,255,255,255);

                    if (block->get_level_internal() == log_level_internal::level_debug)
                        item->color = float4(0, 100, 0, 255)/float4(255,255,255,255);

                    if (block->get_level_internal() == log_level_internal::level_warning)
                        item->color = float4(150, 150, 0, 255)/float4(255,255,255,255);

                    if (block->get_level_internal() == log_level_internal::level_none)
                        item->color = float4(200, 200, 200, 255)/float4(255,255,255,255);

                    if (block->get_level_internal() == log_level_internal::level_info)
                        item->color = float4(0, 0, 200, 255)/float4(255,255,255,255);

                    if (auto refs = find_file_refs(item->text.get()); !refs.empty())
                    {
                        std::vector<Text::Link> spans;
                        for (auto& r : refs)
                            spans.push_back(r.span);

                        item->set_links(std::move(spans), [refs](size_t index)
                            {
                                if (on_open_file && index < refs.size())
                                    on_open_file(refs[index].file, refs[index].line, std::max(refs[index].column, 1));
                            });
                    }

                    std::lock_guard<std::mutex> g(m);
                    add.emplace_back(item);
                });
            }                                                                                           
        }
    }

}