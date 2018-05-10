class RenderTargetState
{
    std::array<ID3D11RenderTargetView*, 8> data;
	DepthStencilView *depth;
	bool changed;
public:

    RenderTargetState();

	bool is_changed()
	{
		return changed;
	}
    class helper
    {
        friend class RenderTargetState;
        ID3D11RenderTargetView* &data;
        helper(ID3D11RenderTargetView* &_data);
    public:
        template<class T>
		void operator = (const T &obj){
			data = *obj;
		}
    };

    helper operator[](unsigned int i);

	DepthStencilView & operator=(DepthStencilView & view)
	{
		depth = &view;
		changed = true;
		return view;
	}
	DepthStencilView * get_depth()
	{
		return depth;
	}

	void operator=(const RenderTargetState& other)
	{
		data = other.data;
		depth = other.depth;
		changed = true;
	}
protected:
	friend class Context;
	void set(DX11_DeviceContext context, bool force = false);
	
};