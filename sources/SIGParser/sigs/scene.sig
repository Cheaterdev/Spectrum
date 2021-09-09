
[Bind = FrameLayout::SceneData]
struct SceneData
{

	StructuredBuffer<node_data> nodes;
	StructuredBuffer<mesh_vertex_input> vertexes;

	StructuredBuffer<MeshCommandData> meshes;
	StructuredBuffer<MaterialCommandData> materials;


	StructuredBuffer<MeshInstance> meshInstances;

	StructuredBuffer<Meshlet> meshlets;

	StructuredBuffer<uint> indices;

	Texture2D<float4> material_textures[];
}

