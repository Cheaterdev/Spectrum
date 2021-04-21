#include "pch.h"
static enum_array<Layouts, DX12::RootLayout::ptr> signatures;
void init_signatures()
{
	signatures[Layouts::FrameLayout] = AutoGenSignatureDesc<FrameLayout>().create_signature(Layouts::FrameLayout);
	signatures[Layouts::DefaultLayout] = AutoGenSignatureDesc<DefaultLayout>().create_signature(Layouts::DefaultLayout);
}
Render::RootLayout::ptr get_Signature(Layouts id)
{
	return signatures[id];
}
std::map<UINT, UINT> get_used_slots(std::string slot_name)
{
	std::map<UINT, UINT> result;
	if(slot_name == "TextureRenderer")
	{
		result[1] = 1;
		return result;
	}
	if(slot_name == "BRDF")
	{
		result[2] = 1;
		return result;
	}
	if(slot_name == "DebugInfo")
	{
		result[2] = 1;
		return result;
	}
	if(slot_name == "FontRendering")
	{
		result[1] = 2;
		return result;
	}
	if(slot_name == "FontRenderingConstants")
	{
		result[0] = 2;
		return result;
	}
	if(slot_name == "FontRenderingGlyphs")
	{
		result[1] = 1;
		return result;
	}
	if(slot_name == "FrameInfo")
	{
		result[0] = 32;
		result[1] = 3;
		return result;
	}
	if(slot_name == "MaterialInfo")
	{
		result[0] = 2;
		return result;
	}
	if(slot_name == "MeshInfo")
	{
		result[0] = 2;
		return result;
	}
	if(slot_name == "GatherPipelineGlobal")
	{
		result[1] = 2;
		return result;
	}
	if(slot_name == "GatherPipeline")
	{
		result[0] = 2;
		result[2] = 8;
		return result;
	}
	if(slot_name == "GatherBoxes")
	{
		result[2] = 2;
		return result;
	}
	if(slot_name == "DrawBoxes")
	{
		result[1] = 2;
		result[2] = 1;
		return result;
	}
	if(slot_name == "InitDispatch")
	{
		result[2] = 2;
		return result;
	}
	if(slot_name == "GatherMeshesBoxes")
	{
		result[1] = 2;
		result[2] = 2;
		return result;
	}
	if(slot_name == "MipMapping")
	{
		result[0] = 3;
		result[1] = 1;
		result[2] = 4;
		return result;
	}
	if(slot_name == "CopyTexture")
	{
		result[1] = 1;
		return result;
	}
	if(slot_name == "DownsampleDepth")
	{
		result[1] = 1;
		result[2] = 1;
		return result;
	}
	if(slot_name == "GBuffer")
	{
		result[1] = 5;
		return result;
	}
	if(slot_name == "GBufferDownsample")
	{
		result[1] = 2;
		return result;
	}
	if(slot_name == "GBufferQuality")
	{
		result[1] = 1;
		return result;
	}
	if(slot_name == "PSSMConstants")
	{
		result[0] = 2;
		return result;
	}
	if(slot_name == "PSSMData")
	{
		result[1] = 2;
		return result;
	}
	if(slot_name == "PSSMDataGlobal")
	{
		result[1] = 2;
		return result;
	}
	if(slot_name == "PSSMLighting")
	{
		result[1] = 6;
		return result;
	}
	if(slot_name == "RaytracingRays")
	{
		result[0] = 1;
		result[1] = 5;
		result[2] = 1;
		return result;
	}
	if(slot_name == "Raytracing")
	{
		result[1] = 2;
		return result;
	}
	if(slot_name == "SceneData")
	{
		result[1] = 5;
		return result;
	}
	if(slot_name == "SkyData")
	{
		result[0] = 1;
		result[1] = 4;
		return result;
	}
	if(slot_name == "SkyFace")
	{
		result[0] = 1;
		return result;
	}
	if(slot_name == "EnvFilter")
	{
		result[0] = 3;
		return result;
	}
	if(slot_name == "EnvSource")
	{
		result[1] = 1;
		return result;
	}
	if(slot_name == "SMAA_Global")
	{
		result[0] = 2;
		result[1] = 1;
		return result;
	}
	if(slot_name == "SMAA_Weights")
	{
		result[1] = 3;
		return result;
	}
	if(slot_name == "SMAA_Blend")
	{
		result[1] = 1;
		return result;
	}
	if(slot_name == "Countour")
	{
		result[0] = 1;
		result[1] = 1;
		return result;
	}
	if(slot_name == "DrawStencil")
	{
		result[1] = 1;
		return result;
	}
	if(slot_name == "PickerBuffer")
	{
		result[2] = 1;
		return result;
	}
	if(slot_name == "Instance")
	{
		result[0] = 1;
		return result;
	}
	if(slot_name == "Color")
	{
		result[0] = 1;
		return result;
	}
	if(slot_name == "Test")
	{
		result[0] = 16;
		return result;
	}
	if(slot_name == "NinePatch")
	{
		result[1] = 1;
		return result;
	}
	if(slot_name == "ColorRect")
	{
		result[0] = 3;
		return result;
	}
	if(slot_name == "FlowGraph")
	{
		result[0] = 3;
		return result;
	}
	if(slot_name == "LineRender")
	{
		result[1] = 1;
		return result;
	}
	if(slot_name == "VoxelInfo")
	{
		result[0] = 4;
		return result;
	}
	if(slot_name == "Voxelization")
	{
		result[0] = 4;
		result[2] = 3;
		return result;
	}
	if(slot_name == "VoxelScreen")
	{
		result[1] = 10;
		return result;
	}
	if(slot_name == "VoxelOutput")
	{
		result[2] = 2;
		return result;
	}
	if(slot_name == "VoxelBlur")
	{
		result[1] = 3;
		result[2] = 2;
		return result;
	}
	if(slot_name == "VoxelUpscale")
	{
		result[1] = 3;
		return result;
	}
	if(slot_name == "VoxelVisibility")
	{
		result[1] = 1;
		result[2] = 1;
		return result;
	}
	if(slot_name == "VoxelMipMap")
	{
		result[0] = 1;
		result[1] = 2;
		result[2] = 3;
		return result;
	}
	if(slot_name == "VoxelCopy")
	{
		result[0] = 1;
		result[1] = 3;
		result[2] = 2;
		return result;
	}
	if(slot_name == "VoxelZero")
	{
		result[0] = 1;
		result[1] = 1;
		result[2] = 1;
		return result;
	}
	if(slot_name == "VoxelLighting")
	{
		result[0] = 1;
		result[1] = 7;
		result[2] = 1;
		return result;
	}
	if(slot_name == "VoxelDebug")
	{
		result[1] = 6;
		return result;
	}
	if(slot_name == "DenoiserDownsample")
	{
		result[1] = 2;
		return result;
	}
	if(slot_name == "DenoiserHistoryFix")
	{
		result[1] = 3;
		result[2] = 1;
		return result;
	}
	if(slot_name == "FrameClassification")
	{
		result[1] = 1;
		result[2] = 2;
		return result;
	}
	if(slot_name == "FrameClassificationInitDispatch")
	{
		result[1] = 2;
		result[2] = 2;
		return result;
	}
	return result;
}
