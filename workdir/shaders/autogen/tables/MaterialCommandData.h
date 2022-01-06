#pragma once
#include "GPUAddress.h"
struct MaterialCommandData
{
	uint pipeline_id; // uint
	GPUAddress material_cb; // GPUAddress
	GPUAddress GetMaterial_cb() { return material_cb; }
	uint GetPipeline_id() { return pipeline_id; }
};
