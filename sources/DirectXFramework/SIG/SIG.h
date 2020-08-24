namespace Render
{
	using Bindless = std::vector<Handle>;


}


using uint = UINT;
using uint2 = ivec2;
using uint3 = ivec3;
using uint4 = ivec4;

using float4x4 = mat4x4;

using GPUAddress = D3D12_GPU_VIRTUAL_ADDRESS;

class DrawIndexedArguments : public D3D12_DRAW_INDEXED_ARGUMENTS
{
public:
	static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
	{
		D3D12_INDIRECT_ARGUMENT_DESC desc;
		desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
		return desc;
	}
};

class DispatchArguments : public D3D12_DISPATCH_ARGUMENTS
{
public:
	static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
	{
		D3D12_INDIRECT_ARGUMENT_DESC desc;
		desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
		return desc;
	}
};

struct Empty
{};

using DefaultCB = Render::Resource*;// std::vector<std::byte>;
using DynamicData = std::vector<std::byte>;// std::vector<std::byte>;



template<typename T> concept HasData =
requires (T t) {
	t.data;
};


template<typename T> concept HasBindless =
requires (T t) {
	t.bindless;
};


template<typename T> concept HasSRV =
requires (T t) {
	t.srv;
};

template<typename T> concept HasUAV =
requires (T t) {
	t.uav;
};

template<typename T> concept HasSMP =
requires (T t) {
	t.smp;
};

template<typename T> concept HasCB =
requires (T t) {
	t.cb;
};

template<typename T> concept HasRTV =
requires (T t) {
	t.rtv;
};

template<typename T> concept HasDSV =
requires (T t) {
	t.dsv;
};


template<typename T> concept TableHasSRV =
requires (T t) {
	t.SRV;
};

template<typename T> concept TableHasUAV =
requires (T t) {
	t.UAV;
};

template<typename T> concept TableHasSMP =
requires (T t) {
	t.SMP;
};

template<typename T> concept TableHasCB =
requires (T t) {
	t.CB;
};



