#pragma once
namespace RT
{
	namespace Table
	{
		struct GBuffer
		{
			struct RTV
			{
				Graphics::Handle albedo;
				Graphics::Handle normals;
				Graphics::Handle specular;
				Graphics::Handle motion;
			} &rtv;
			struct DSV
			{
				Graphics::Handle depth;
			}&dsv;
			Graphics::Handle& GetAlbedo() { return rtv.albedo; }
			Graphics::Handle& GetNormals() { return rtv.normals; }
			Graphics::Handle& GetSpecular() { return rtv.specular; }
			Graphics::Handle& GetMotion() { return rtv.motion; }
			Graphics::Handle& GetDepth() { return dsv.depth; }
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
