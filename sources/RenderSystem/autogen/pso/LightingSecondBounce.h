#pragma once
namespace Autogen
{
	struct LightingSecondBounce: public Lighting
	{
		static inline const D3D::shader_header compute = { "shaders/voxel_lighting.hlsl", "CS", 0,{		{		"EntryPoint", "CS"		}		,		{		"SECOND_BOUNCE", "1"		}		} }; 
	};
}
