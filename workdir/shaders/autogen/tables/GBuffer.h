#pragma once
struct GBuffer_srv
{
	uint albedo; // Texture2D<float4>
	uint normals; // Texture2D<float4>
	uint specular; // Texture2D<float4>
	uint depth; // Texture2D<float>
	uint motion; // Texture2D<float2>
};
struct GBuffer
{
	GBuffer_srv srv;
	Texture2D<float4> GetAlbedo() { return ResourceDescriptorHeap[srv.albedo]; }
	Texture2D<float4> GetNormals() { return ResourceDescriptorHeap[srv.normals]; }
	Texture2D<float4> GetSpecular() { return ResourceDescriptorHeap[srv.specular]; }
	Texture2D<float> GetDepth() { return ResourceDescriptorHeap[srv.depth]; }
	Texture2D<float2> GetMotion() { return ResourceDescriptorHeap[srv.motion]; }

};
 const GBuffer CreateGBuffer(GBuffer_srv srv)
{
	const GBuffer result = {srv
	};
	return result;
}
