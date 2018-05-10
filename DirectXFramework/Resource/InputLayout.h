enum class  transfer_type : int {PER_VERTEX = 0, PER_INSTANCE = 1};



struct vertex_buffer_layout
{
    std::string name;
    unsigned int index;
    unsigned int offset;
    unsigned int slot;
    DXGI_FORMAT format;
    transfer_type type;
    vertex_buffer_layout() {}
    vertex_buffer_layout(std::string name,
                         unsigned int index,
                         unsigned int offset,
                         unsigned int slot,
                         DXGI_FORMAT format,
                         transfer_type type)
    {
        this->name = name;
        this->index = index;
        this->offset = offset;
        this->slot = slot;
        this->format = format;
        this->type = type;
    }

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
        ar &NVP(name);
        ar &NVP(index);
        ar &NVP(offset);
        ar &NVP(slot);
        ar &NVP(type);
        ar &NVP(format);
    }
};

bool operator<(const vertex_buffer_layout &l, const vertex_buffer_layout &r);
struct shader_input_desc
{
    std::string name;
    unsigned int index;


private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &NVP(name);
        ar &NVP(index);
    }
};

struct shader_inputs
{
    std::vector<shader_input_desc> inputs;
};


bool operator<(const shader_inputs &l, const shader_inputs &r);

bool operator==(const shader_inputs &l, const shader_inputs &r);

class layout_id_generator : public Singleton<layout_id_generator>
{
    friend class Singleton<layout_id_generator>;

    std::map<shader_inputs, std::shared_ptr<shader_inputs> > descs;
    std::map<vertex_buffer_layout, std::shared_ptr<vertex_buffer_layout>> layouts;

public:
    std::shared_ptr<shader_inputs> get_unique(const shader_inputs &desc)
    {
        auto result = descs.find(desc);

        if (result == descs.end())
        {
            std::shared_ptr<shader_inputs> r(new shader_inputs(desc));
            descs[desc] = r;
            return r;
        }

        return result->second;
    }

    std::shared_ptr<vertex_buffer_layout> get_unique(const vertex_buffer_layout &layout)
    {
        auto result = layouts.find(layout);

        if (result == layouts.end())
        {
            std::shared_ptr<vertex_buffer_layout> r(new vertex_buffer_layout(layout));
            layouts[layout] = r;
            return r;
        }

        return result->second;
    }
};

bool operator<(const shader_input_desc &l, const shader_input_desc &r);
bool operator==(const shader_input_desc &l, const shader_input_desc &r);

struct input_layout_header
{
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputs;

private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar &NVP(inputs);
    }
};
bool operator<(const input_layout_header &l, const input_layout_header &r);

class vertex_shader;
class Context;
class input_layout: protected NativeContextAccessor
{
    DX11_InputLayout native_layout;
    input_layout_header desc;
    friend class Context;
public:
    using ptr = s_ptr<input_layout>;
    static const ptr null;

    static	std::shared_ptr<input_layout> get_resource(const input_layout_header &h, std::shared_ptr<vertex_shader> shader);
    static	std::shared_ptr<input_layout> get_resource(const input_layout_header &h, vertex_shader* shader);
    static std::shared_ptr<input_layout> create_new(const input_layout_header &header, vertex_shader* shader);
private:
    void set(DX11::Context &context);
};

