class SamplerState: protected NativeContextAccessor
{
    std::array<ID3D11SamplerState*, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT> data;
    bool changed;
public:

    SamplerState();


    bool is_changed()
    {
        return changed;
    }
    class helper
    {
        friend class SamplerState;
        ID3D11SamplerState* &data;
        helper(ID3D11SamplerState* &_data);
    public:

        void operator = (sampler_state &obj)
        {
            data = obj.get_native().p;
        }

        void operator = (std::shared_ptr<sampler_state> obj)
        {
            (*this) = *obj;
        }


    };

    helper operator[](unsigned int i);

protected:
    friend class Context;
    template<class T>
    void set(NativeContext &context, bool force = false);


    template<>
    void set<pixel_shader>(NativeContext &context, bool force)
    {
        if (changed || force)
        {
            get_native_context(context)->PSSetSamplers(0, data.size(), data.data());
            changed = false;
        }
        else
        {
            context.sampler_optimized_count++;
        }
    }
    template<>
    void set<vertex_shader>(NativeContext &context, bool force)
    {
        if (changed || force)
        {
			get_native_context(context)->VSSetSamplers(0, data.size(), data.data());
            changed = false;
        }
        else
        {
            context.sampler_optimized_count++;
        }
    }

    template<>
    void set<geometry_shader>(NativeContext &context, bool force)
    {
        if (changed || force)
        {
			get_native_context(context)->GSSetSamplers(0, data.size(), data.data());
            changed = false;
        }
        else
        {
            context.sampler_optimized_count++;
        }
    }

};