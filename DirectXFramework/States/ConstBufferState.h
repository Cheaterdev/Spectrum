class ConstBufferState: protected NativeContextAccessor
{
    std::vector<ID3D11Buffer*> data;
    std::vector<BufferBase*> buffers;
    bool changed;
public:

    ConstBufferState();


    class helper
    {
        friend class ConstBufferState;
        ID3D11Buffer* &data;
        BufferBase* &buffer;
        helper(ID3D11Buffer* &data, BufferBase* &buffer);
    public:
        template<class T>
        void operator = ( Buffer<T> &obj)
        {
            data = static_cast<ID3D11Buffer*>(obj.get_native());
            buffer = static_cast<BufferBase*>(&obj);
        }

        template<class T>
        void operator = (std::shared_ptr<Buffer<T>> obj)
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
    void set<pixel_shader>(NativeContext  &context, bool)
    {
        for (auto b : buffers)
            if (b)
                b->update(get_native_context(context));

        if (changed)
        {
            get_native_context(context)->PSSetConstantBuffers(0, data.size(), data.data());
            changed = false;
        }
        else
        {
            context.const_buffer_optimized_count++;
        }
    }

    template<>
    void set<vertex_shader>(NativeContext  &context, bool)
    {
        for (auto b : buffers)
            if (b)
                b->update(get_native_context(context));

        if (changed)
        {
            get_native_context(context)->VSSetConstantBuffers(0, data.size(), data.data());
            changed = false;
        }
        else
        {
            context.const_buffer_optimized_count++;
        }
    }

    template<>
    void set<geometry_shader>(NativeContext  &context, bool)
    {
        for (auto b : buffers)
            if (b)
                b->update(get_native_context(context));

        if (changed)
        {
            get_native_context(context)->GSSetConstantBuffers(0, data.size(), data.data());
            changed = false;
        }
        else
        {
            context.const_buffer_optimized_count++;
        }
    }
};