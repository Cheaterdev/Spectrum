class BufferBase : public Resource
{
	std::vector<ShaderResourceView> shader_resource_views;

protected:
    D3D11_BUFFER_DESC desc;
    DX11_Buffer native_buffer;

    bool changed;
    virtual void init_views()
    {

		if (desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
		{

			D3D11_SHADER_RESOURCE_VIEW_DESC DescSR;
			DescSR.ViewDimension = D3D_SRV_DIMENSION_BUFFER;

			DescSR.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			DescSR.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
			DescSR.Buffer.ElementOffset = 0;
			DescSR.Buffer.ElementWidth = desc.ByteWidth/sizeof(vec4);

			shader_resource_views.emplace_back(this, DescSR);
		}


    }
    virtual ~BufferBase() {}

public:
	virtual void* get_data() = 0;


    virtual native_resource get_native() override
    {
        return native_buffer;
    }

    virtual unsigned int get_stride()
    {
        return desc.StructureByteStride;
    }

    void update(DX11_DeviceContext context)
    {
        if (!changed) return;

        if (desc.Usage == D3D11_USAGE_DEFAULT)
        {
            D3D11_BOX b;
            b.bottom = 1;
            b.back = 1;
            b.top = 0;
            b.front = 0;
            b.left = 0;
            b.right = desc.ByteWidth;
            context->UpdateSubresource(native_buffer, 0, 0, get_data(), desc.ByteWidth, 0);
        }
        else if (desc.Usage == D3D11_USAGE_DYNAMIC)
        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            D3D11_MAP		 map_type = D3D11_MAP_WRITE_DISCARD;
			TEST(context->Map(native_buffer, 0, map_type, 0, &mapped));
            memcpy(mapped.pData, get_data(), desc.ByteWidth);
            context->Unmap(native_buffer, 0);
        }

        changed = false;
    }

	ShaderResourceView& get_shader_view(unsigned int i = 0)
	{
		return shader_resource_views[i];
	}

};
template<class T = unsigned char>
class Buffer: public BufferBase
{
    std::vector<T> internal_data;
protected:

    void* get_data()
    {
		return reinterpret_cast<void*>(internal_data.data());
    }

    template<class T2 = T>
    void init(D3D11_USAGE usage, unsigned int bind_flag, unsigned int cpu_flag, unsigned int resource_flag, T2* _data = nullptr, unsigned int count = 1, unsigned int stride = sizeof(T2))
    {
        changed = false;

        if (_data)
			this->internal_data.assign(reinterpret_cast<const T*>(_data), reinterpret_cast<const T*>(_data + count));
        else
			this->internal_data.resize(count);

        desc.ByteWidth = sizeof(T2) * count;

        if (bind_flag & D3D11_BIND_CONSTANT_BUFFER)
        {
            desc.ByteWidth =	desc.ByteWidth + 16 - 1 - (desc.ByteWidth - 1) % 16;
        }

        desc.StructureByteStride = stride;
        desc.Usage = usage;
        desc.BindFlags = bind_flag;
        desc.CPUAccessFlags = cpu_flag;
        desc.MiscFlags = resource_flag;
        D3D11_SUBRESOURCE_DATA init_data;
		init_data.pSysMem = internal_data.data();
        init_data.SysMemPitch = sizeof(T2) * count / stride;
		init_data.SysMemSlicePitch = stride;
		{auto& lock = Device::get().lock();
		TEST(Device::get().get_native_device()->CreateBuffer(&desc, &init_data, &native_buffer)); }
        init_views();
    }
public:

    using ptr = s_ptr<Buffer<T>>;
    template<class T2 = T>
    Buffer(D3D11_USAGE usage, unsigned int bind_flag, unsigned int cpu_flag, unsigned int resource_flag, const std::vector<T2> &data, unsigned int stride = sizeof(T2))
    {
        init(usage, bind_flag, cpu_flag, resource_flag, data.data(), data.size(), stride);
    }

    template<class T2 = T>
    Buffer(D3D11_USAGE usage, unsigned int bind_flag, unsigned int cpu_flag, unsigned int resource_flag, T2* _data = nullptr, unsigned int count = 1, unsigned int stride = sizeof(T2))
    {
        init(usage, bind_flag, cpu_flag, resource_flag, _data, count, stride);
    }

    Buffer(D3D11_USAGE usage, unsigned int bind_flag, unsigned int cpu_flag, unsigned int resource_flag, unsigned int count = 1, unsigned int stride = sizeof(T))
    {
        init(usage, bind_flag, cpu_flag, resource_flag, static_cast<T*>(nullptr), count, stride);
    }

    virtual ~Buffer() {}

    T &operator[](unsigned int i)
    {
        changed = true;
		return internal_data[i];
    }

	T* operator->()
	{
		changed = true;
		return internal_data.data();
	}

    template <class T2>
    const T &operator=(const T2 &r)
    {
        changed = true;
		return internal_data[0] = r;
    }

    template<class T2>
    void set_data(T2* data, unsigned int count)
    {
		memcpy(this->internal_data.data(), data, count * sizeof(T2));
        changed = true;
    }

	T* data()
	{
		changed = true;
		return internal_data.data();
	}


};

