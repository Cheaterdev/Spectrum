#pragma once
struct BRDF_uav
{
	RWTexture3D<float4> output;
};
struct BRDF
{
	BRDF_uav uav;
	RWTexture3D<float4> GetOutput() { return uav.output; }

};
 const BRDF CreateBRDF(BRDF_uav uav)
{
	const BRDF result = {uav
	};
	return result;
}
