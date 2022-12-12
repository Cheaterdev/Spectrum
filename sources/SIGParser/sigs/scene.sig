
[Bind = FrameLayout::SceneData]
struct SceneData
{
	StructuredBuffer<node_data> nodes;
	StructuredBuffer<MeshCommandData> meshes;
	StructuredBuffer<MaterialCommandData> materials;

	StructuredBuffer<RaytraceInstanceInfo> raytraceInstanceInfo;

}

