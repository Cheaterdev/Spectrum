#pragma once
#include "GPUAddress.h"
struct MaterialCommandData_cb
{
	uint pipeline_id;
	GPUAddress_cb material_cb;
};
struct MaterialCommandData
{
	MaterialCommandData_cb cb;
	uint GetPipeline_id() { return cb.pipeline_id; }
	GPUAddress GetMaterial_cb() { return CreateGPUAddress(cb.material_cb); }
};
 const MaterialCommandData CreateMaterialCommandData(MaterialCommandData_cb cb)
{
	const MaterialCommandData result = {cb
	};
	return result;
}
