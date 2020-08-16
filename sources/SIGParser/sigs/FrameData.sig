struct Frustum
{
	float4 planes[6];
}

struct Camera
{
	float4x4 view;
	float4x4 proj;
	float4x4 viewProj;
	float4x4 invView;
	float4x4 invProj;
	float4x4 invViewProj;
	float4 position;
	float4 direction;
	float4 jitter;
	
	Frustum frustum;
}

[Bind = FrameLayout::CameraData]
struct FrameInfo
{
	float4 time;
	
    Camera camera;
    Camera prevCamera;

    Texture2D bestFitNormals;
	Texture3D<float4> brdf;

}
