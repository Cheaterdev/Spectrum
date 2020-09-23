#pragma once
namespace Autogen
{
	struct VoxelDownsample2
	{
		static inline const Layouts layout = Layouts::DefaultLayout;
		static inline const D3D::shader_header compute = { "shaders/voxel_mipmap.hlsl", "CS", 0,{		{		"EntryPoint", "CS"		}		,		{		"COUNT", "3"		}		} }; 
	};
}
