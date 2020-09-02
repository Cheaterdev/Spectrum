
[Bind = DefaultLayout::Instance2]
struct RaytracingRays
{
	GBuffer gbuffer;
	RWTexture2D<float4> output;
	float pixelAngle;
}

[nobind]
struct RayCone
{
    float width;
    float angle;
}

[nobind]
struct RayPayload
{
    RayCone cone;
    float4 color;
	uint recursion;
	float dist;

	%{

	RayPayload propagate(float surfaceSpreadAngle = 0, float hitT = 0)
	{
		RayPayload result;

		result.color = 0;
		result.dist = 0;
		result.recursion = recursion + 1;

		result.cone.width = cone.width + cone.angle* hitT;
		result.cone.angle = cone.angle + surfaceSpreadAngle;

		return result;
	}

	}%
}

[nobind]
struct ShadowPayload
{
	bool hit;
}

[nobind]
struct Triangle
{
	mesh_vertex_input v;

	float lod;

	%{
		void init(mesh_vertex_input vertex0, mesh_vertex_input vertex1, mesh_vertex_input vertex2, float3 barycentrics)
		{
			v.normal = (vertex0.normal * barycentrics.x + vertex1.normal * barycentrics.y + vertex2.normal * barycentrics.z);
			v.tc = vertex0.tc * barycentrics.x + vertex1.tc * barycentrics.y + vertex2.tc * barycentrics.z;

			float P_a = length(cross(vertex2.pos - vertex0.pos, vertex1.pos - vertex0.pos));
			float T_a =  length(cross(float3(vertex2.tc - vertex0.tc,0), float3(vertex1.tc - vertex0.tc,0 )));
			lod = 0.5 * log2(T_a / P_a);

		}
	}%
}



[Bind = DefaultLayout::Raytracing]
struct Raytracing
{
	RaytracingAccelerationStructure scene;
	StructuredBuffer<uint> index_buffer;


}
