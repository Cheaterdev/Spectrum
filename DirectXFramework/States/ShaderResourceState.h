class ShaderResourceState:protected NativeContextAccessor
{
    std::array<ID3D11ShaderResourceView*, 32> data;
    bool changed;
public:

    ShaderResourceState();

    bool is_changed()
    {
        return changed;
    }
    class helper
    {
        friend class ShaderResourceState;
        ID3D11ShaderResourceView* &data;
        helper(ID3D11ShaderResourceView* &_data);
    public:
        template<class T>
        void operator = (const T &obj)
        {
            data = *obj;
        }


        template<class T>
        void operator = (std::shared_ptr<T> obj)
        {
            (*this) = *obj;
        }


    };

    helper operator[](unsigned int i);



protected:
    friend class Context;
    template<class T>
    void set(NativeContext& context, bool force = false);


    template<>
	void set<pixel_shader>(NativeContext& context, bool )
    {
        if (changed)
        {
			get_native_context(context)->PSSetShaderResources(0, static_cast<UINT>(data.size()), data.data());
            changed = false;
		}
else
{
	context.srv_optimized_count++;
}
	}

    template<>
	void set<vertex_shader>(NativeContext& context, bool )
    {
        if (changed)
        {
			get_native_context(context)->VSSetShaderResources(0, static_cast<UINT>(data.size()), data.data());
            changed = false;
		}
		else
		{
			context.srv_optimized_count++;
		}
    }

    template<>
	void set<geometry_shader>(NativeContext& context, bool )
    {
        if (changed)
        {
			get_native_context(context)->GSSetShaderResources(0, static_cast<UINT>(data.size()), data.data());
            changed = false;
		}
		else
		{
			context.srv_optimized_count++;
		}
    }
};