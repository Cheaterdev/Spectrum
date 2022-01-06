#pragma once
#include "FSRConstants.h"
struct FSR
{
	uint source; // Texture2D<float4>
	uint target; // RWTexture2D<float4>
	FSRConstants constants; // FSRConstants
	FSRConstants GetConstants() { return constants; }
	Texture2D<float4> GetSource() { return ResourceDescriptorHeap[source]; }
	RWTexture2D<float4> GetTarget() { return ResourceDescriptorHeap[target]; }
};
