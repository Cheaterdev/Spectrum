#pragma once
struct BRDF_uav
{
	uint output; // RWTexture3D<float4>
};
struct BRDF
{
	BRDF_uav uav;
	RWTexture3D<float4> GetOutput() { return ResourceDescriptorHeap[uav.output]; }

};
 const BRDF CreateBRDF(BRDF_uav uav)
{
	const BRDF result = {uav
	};
	return result;
}
