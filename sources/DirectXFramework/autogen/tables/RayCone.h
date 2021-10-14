#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct RayCone
	{
		struct CB
		{
			float width;
			float angle;
		} &cb;
		float& GetWidth() { return cb.width; }
		float& GetAngle() { return cb.angle; }
		RayCone(CB&cb) :cb(cb){}
	};
	#pragma pack(pop)
}
