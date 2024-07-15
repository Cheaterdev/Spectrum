#pragma once
#include "sig_hlsl.hlsl"
struct MaterialCommandData
{
	uint material_cb; // uint
	uint pipeline_id; // uint
	uint GetMaterial_cb() { return material_cb; }
	uint GetPipeline_id() { return pipeline_id; }
};