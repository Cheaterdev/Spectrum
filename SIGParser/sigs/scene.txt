
[Bind = FrameLayout::SceneData]
struct SceneData
{

	StructuredBuffer<node_data> nodes;
	StructuredBuffer<mesh_vertex_input> vertexes;

	StructuredBuffer<MeshCommandData> meshes;
	StructuredBuffer<MaterialCommandData> materials;

	Texture2D material_textures[];
}

