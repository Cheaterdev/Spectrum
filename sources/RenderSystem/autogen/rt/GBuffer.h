#pragma once
namespace RT
{
	namespace Table
	{
		struct GBuffer
		{
			struct RTV
			{
				Render::Handle albedo;
				Render::Handle normals;
				Render::Handle specular;
				Render::Handle motion;
			} &rtv;
			struct DSV
			{
				Render::Handle depth;
			}&dsv;
			Render::Handle& GetAlbedo() { return rtv.albedo; }
			Render::Handle& GetNormals() { return rtv.normals; }
			Render::Handle& GetSpecular() { return rtv.specular; }
			Render::Handle& GetMotion() { return rtv.motion; }
			Render::Handle& GetDepth() { return dsv.depth; }
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
