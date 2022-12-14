
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

	%{
	RayCone propagate(float surfaceSpreadAngle = 0, float hitT = 0)
	{
		RayCone result;
		result.width = width + angle* hitT;
		result.angle = angle + surfaceSpreadAngle;
		
		return result;
	}

	}%
}

[nobind]
struct RayPayload
{
    RayCone cone;
    float4 color;
	float3 dir;
	uint recursion;
	float dist;

	%{

	RayPayload propagate(float surfaceSpreadAngle = 0, float hitT = 0)
	{
		RayPayload result;

		result.color = 0;
		result.dist = 0;
		result.recursion = recursion + 1;

		result.cone = cone.propagate(surfaceSpreadAngle, hitT);
		
		return result;
	}


	void init()
	{
		color = 0;
		recursion = 0;
		dist = 0;
		cone.angle = 0;
		cone.width = 0; 
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
}


RaytracePSO MainRTX
{
	root = DefaultLayout;


}

[Bind = MainRTX]
RaytraceRaygen Shadow
{
	[EntryPoint = ShadowRaygenShader]
	raygen = raytracing;
}


[Bind = MainRTX]
RaytraceRaygen Reflection
{
	[EntryPoint = MyRaygenShaderReflection]
	raygen = raytracing;
}


[Bind = MainRTX]
RaytraceRaygen Indirect
{
	[EntryPoint = MyRaygenShader]
	raygen = raytracing;
}

[Bind = MainRTX]
RaytracePass ShadowPass
{
	[EntryPoint = ShadowMissShader]
	miss = raytracing;

	[EntryPoint = ShadowClosestHitShader]
	closest_hit = raytracing;

	payload = ShadowPayload;
}

[Bind = MainRTX]
RaytracePass ColorPass
{
	[EntryPoint = MyMissShader]
	miss = raytracing;

	[EntryPoint = MyClosestHitShader]
	closest_hit = none;

	payload = RayPayload;

	local = MaterialInfo;
	per_material = true;
}
