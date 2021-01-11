#pragma once
namespace RT
{
	namespace Table
	{
		struct GBuffer
		{
			struct RTV
			{
				DX12::Handle albedo;
				DX12::Handle normals;
				DX12::Handle specular;
				DX12::Handle motion;
			} &rtv;
			struct DSV
			{
				DX12::Handle depth;
			}&dsv;
			DX12::Handle& GetAlbedo() { return rtv.albedo; }
			DX12::Handle& GetNormals() { return rtv.normals; }
			DX12::Handle& GetSpecular() { return rtv.specular; }
			DX12::Handle& GetMotion() { return rtv.motion; }
			DX12::Handle& GetDepth() { return dsv.depth; }
		GBuffer(RTV & rtv, DSV & dsv):rtv(rtv), dsv(dsv){}
		};
	}
	namespace Slot
	{
		struct GBuffer: public RTHolder<Table::GBuffer>
		{
			RTV rtv;
			DSV dsv;
			GBuffer():RTHolder<Table::GBuffer>(rtv, dsv){}
		};
	}
}
