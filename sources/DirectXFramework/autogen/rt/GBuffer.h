#pragma once
namespace RT
{
	namespace Table
	{
		struct GBuffer
		{
			struct RTV
			{
				HAL::Handle albedo;
				HAL::Handle normals;
				HAL::Handle specular;
				HAL::Handle motion;
			} &rtv;
			struct DSV
			{
				HAL::Handle depth;
			}&dsv;
			HAL::Handle& GetAlbedo() { return rtv.albedo; }
			HAL::Handle& GetNormals() { return rtv.normals; }
			HAL::Handle& GetSpecular() { return rtv.specular; }
			HAL::Handle& GetMotion() { return rtv.motion; }
			HAL::Handle& GetDepth() { return dsv.depth; }
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
