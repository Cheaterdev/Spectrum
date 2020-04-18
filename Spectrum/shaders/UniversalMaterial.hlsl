#include "autogen/FrameInfo.h"
#include "autogen/MaterialInfo.h"
#include "autogen/MeshInfo.h"
#include "autogen/MeshData.h"

static const Camera camera = GetFrameInfo().GetCamera();

#define Sampler linearSampler

struct vertex_output   
{ 
float4 pos : SV_POSITION;  
float3 wpos : POSITION;
float3 normal : NORMAL; 
float3 binormal : BINORMAL;
float3 tangent : TANGENT;
float2 tc : TEXCOORD;
    float4 cur_pos : CUR_POSITION; 
  
    float4 prev_pos : PREV_POSITION; 
};
struct vertex_output2
{ 
float4 pos : SV_POSITION;
float3 wpos : POSITION;   
float3 normal : NORMAL; 
float3 binormal : BINORMAL;
float3 tangent : TANGENT;
float2 tc : TEXCOORD;  
    float4 cur_pos : CUR_POSITION; 
 
    float4 prev_pos : PREV_POSITION;
    float dist : DISTANCE; 
};
struct PS_RESULT 
{
float4 albedo : SV_TARGET0; 
float4 normals : SV_TARGET1;
float4 specular : SV_TARGET2; 
    float2 speed : SV_TARGET3;
};
/*
cbuffer cbMaterials : register(b2)
{
    uint texture_offset;
	uint  node_offset;
};*/
#define _voxel_min float3(-50,-50,-50)
#define _voxel_size float3(100,100,100) 
/*
cbuffer Dims : register(b3)
{
    float voxel_min_x;
    float voxel_min_y;
    float voxel_min_z;
    float voxel_size_x;
    float voxel_size_y;
    float voxel_size_z; 

	
	int voxel_mape_x; 
	int voxel_mape_y;
	int voxel_mape_z;*
	int voxel_map;
};
static const float3 voxel_min=float3(voxel_min_x,voxel_min_y,voxel_min_z);
static const float3 voxel_size=float3(voxel_size_x,voxel_size_y,voxel_size_z);*/
//static const int3 voxel_map_size = float3(voxel_mape_x, voxel_mape_y, voxel_mape_z);


struct voxel_info_struct 
{
	float4 voxel_min;
	float4 voxel_size;
	int4 voxel_map_size; 
	int4 voxel_tiles_count;
	int4 voxels_per_tile; 
};/*
cbuffer Dims : register(b3) 
{
	voxel_info_struct st
};
*/
/*
cbuffer cbVoxels : register(b3)
{
	voxel_info_struct voxel_info;
};
*/
//ConstantBuffer<voxel_info_struct> voxel_info: register(b3);

/*
Texture2D textures[] : register(t0); 
RWByteAddressBuffer visibility[] : register(u0);  
RWTexture3D<float4> voxel_tex[2] : register(u0,space1);
RWByteAddressBuffer voxel_visibility: register(u2, space1);

SamplerState LinearSampler : register(s0);
SamplerState PointSampler : register(s1); 
 
SamplerState Sampler : register(s2);
Texture2D best_fit_normals : register(t0,space1);
*/

half3 compress_normals(inout half3 vNormal) 
{
	// renormalize (needed only if any blending or interpolation happened before)
	vNormal.rgb = normalize(vNormal.rgb);
	// get unsigned normal for the cubemap lookup
	half3  vNormalUns = abs(vNormal.rgb);
	// get the main axis for cubemap lookup
	half   maxNAbs = max(vNormalUns.z, max(vNormalUns.x, vNormalUns.y));
	// get texture coordinates in a collapsed cubemap
	float2 vTexCoord = vNormalUns.z<maxNAbs ? (vNormalUns.y<maxNAbs ? vNormalUns.yz : vNormalUns.xz) : vNormalUns.xy;
	vTexCoord = vTexCoord.x < vTexCoord.y ? vTexCoord.yx : vTexCoord.xy;
	vTexCoord.y /= vTexCoord.x;
	// fit normal into the edge of unit cube
	vNormal.rgb /= maxNAbs;
	// look-up fitting length and scale the normal to get the best fit
	half fFittingScale = GetFrameInfo().GetBestFitNormals().SampleLevel(pointClampSampler, vTexCoord,0).a;
	// scale the normal to get the best fit
	vNormal.rgb *= fFittingScale; 
	// wrap to [0;1] unsigned form
	vNormal.rgb = vNormal.rgb * .5h + .5h;

	return vNormal;
}
 
Texture2D get_texture(uint i) 
{
    return GetMaterialInfo().GetTextures(i);
}

void spec_to_metallic(float4 albedo, float3 specular,out float4 mat_albedo, out float metallic)
{
//albedo = pow(albedo, 2.2);
//specular = pow(specular, 2.2);

//	float spec_avg = dot(albedo.rgb / specular, float3(1,1,1))/3;
//	metallic = 1.0 / (albedo.rgb / specular + 1); 



	float3 metal_rgb = (float3(1, 1, 1) / (albedo.rgb / specular + 1));
	metallic = (metal_rgb.x + metal_rgb.y + metal_rgb.z) / 3;


	mat_albedo.rgb = specular / metal_rgb;// albedo.rgb / (1 - metal_rgb);// albedo / (1 - metal_rgb);// specular / metal_rgb;

	//metallic = w;// specular / mat_albedo.rgb;

	mat_albedo.w = albedo.w;  
}


#ifdef BUILD_FUNC_PS


PS_RESULT universal(vertex_output i, float4 albedo, float metallic,float roughness, float4 bump)
{ 
 //  clip(albedo.w-0.5);
   // clip(albedo.a - 0.2);  
    bump.xyz = normalize(bump.xyz * 2.0 - 1.0);
    //float3 bump = xy.x > 0 ? normalize(tex_normal.Sample(point_sampler, i.tc).xyz * 2.0 - 1.0) : float3(0, 0, 1);
	float3 normal =  normalize(-bump.x * i.tangent + bump.y * i.binormal + bump.z * i.normal);

    float3 v = -normalize(camera.GetPosition() - i.wpos);  
    float3 r = reflect(v, normal);         
    PS_RESULT result;   

    result.albedo = float4(albedo.xyz, metallic);
    result.normals = float4(compress_normals(normal), (roughness));
	result.specular = 0;// float4(metallic, roughness); 
      

    float2 cur_p = float2(0.5, 0.5) + float2(0.5, -0.5)*(i.cur_pos.xy / i.cur_pos.w);
    float2 prev_p = float2(0.5,0.5) + float2(0.5, -0.5)*(i.prev_pos.xy / i.prev_pos.w);
	   
    result.speed = 0;// (cur_p - prev_p);// +float2(-1, 1) *(camera.jitter - prev_camera.jitter);
    return result;       
}    

void COMPILED_FUNC(in float3 a, in float2 b, out float4 c, out float d, out float e, out float4 f);
PS_RESULT PS(vertex_output i)
{
	float4 color = 1;
	float metallic = 1;
	float roughness = 1;
	float4 normal = 0;

	COMPILED_FUNC(i.wpos, i.tc, color, metallic, roughness, normal);

	return universal(i, color, metallic, roughness, normal);
}

#endif
   
#ifdef BUILD_FUNC_PS_VOXEL


void universal_voxel(vertex_output i, float4 albedo, float metallic, float roughness, float4 bump)
{   
 //   clip(albedo.w - 0.5);
  
    uint3 index = floor(voxel_info.voxel_map_size.xyz * ((i.wpos.xyz - (voxel_info.voxel_min.xyz)) / voxel_info.voxel_size.xyz)+0.5);
//	uint3 index = 512 * ((i.wpos.xyz - (-150)) /300); 
	  
    voxel_tex[0][index] = float4(albedo.xyz*(1-metallic), 1);
    voxel_tex[1][index] = float4(i.normal*0.5+0.5, 1); 
	
#ifndef VOXEL_DYNAMIC 
	index /= voxel_info.voxels_per_tile.xyz;// uint3(32, 16, 16);
	 

	int3 cell_size = voxel_info.voxel_tiles_count.xyz;// voxel_info.voxel_map_size.xyz / int3(32, 16, 16);
	int offset = 4 * (index.x+ cell_size.x*(index.y+ cell_size.y*index.z)); 
	voxel_visibility.Store4(offset, 0);
#endif
}
void COMPILED_FUNC(in float3 a, in float2 b, out float4 c, out float d, out float e, out float4 f);
void PS_VOXEL(vertex_output i)
{
	float4 color = 1;
	float metallic = 1;
	float roughness = 1;
	float4 normal = 0;

	COMPILED_FUNC(i.wpos, i.tc, color, metallic, roughness, normal);

	universal_voxel(i, color, metallic, roughness, normal);
}
#endif

float4 tile_sample(Texture2D<float4> tex, SamplerState s, float2 tc, Texture2D<float4> residency, RWByteAddressBuffer visibility)
{
  /*  float calculated_level = tex.CalculateLevelOfDetail(s, tc);
    float2 dims; 
    residency.GetDimensions(dims.x, dims.y);   
    float4 res_data = residency.Gather(PointSampler, tc) * 255;
    float min_level = max(res_data.x, res_data.y);
    min_level = max(min_level, res_data.z);
    min_level = max(min_level, res_data.w);
    int offset = 4 * (floor(tc.x * dims.x) + floor(tc.y * dims.y + 0.5) * dims.x);
    visibility.InterlockedMin(offset, int(calculated_level));
    return tex.SampleLevel(s, tc, max(calculated_level, min_level)); */

    return float4(1, 0, 0, 0);
}



struct HS_CONSTANT_DATA_OUTPUT
{
float Edges[3] : SV_TessFactor;
float Inside : SV_InsideTessFactor;

};


HS_CONSTANT_DATA_OUTPUT ConstantsHS(InputPatch<vertex_output2, 3> p, uint PatchID : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT output = (HS_CONSTANT_DATA_OUTPUT) 0;
    float4 vEdgeTessellationFactors;
    vEdgeTessellationFactors = 8; //* p[0].dist;
    //vEdgeTessellationFactors.w = vEdgeTessellationFactors.x;
    //  vEdgeTessellationFactors.y = vEdgeTessellationFactors.x;
    //  vEdgeTessellationFactors.z = vEdgeTessellationFactors.x;
    // Calculate edge scale factor from vertex scale factor: simply compute
    // average tess factor between the two vertices making up an edge
    float3 fScaleFactor = 1;
    fScaleFactor.x = 0.5 * (p[1].dist + p[2].dist);
    fScaleFactor.y = 0.5 * (p[2].dist + p[0].dist);
    fScaleFactor.z = 0.5 * (p[0].dist + p[1].dist);
    // Scale edge factors 
    vEdgeTessellationFactors *= fScaleFactor.xyzx; 
    // Assign tessellation levels
    output.Edges[0] = max(1, vEdgeTessellationFactors.x);
    output.Edges[1] = max(1, vEdgeTessellationFactors.y);
    output.Edges[2] = max(1, vEdgeTessellationFactors.z);
    output.Inside = max(1, vEdgeTessellationFactors.w);
    return output; 
}




[domain("tri")] 
[partitioning("fractional_even")]  
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]  
[patchconstantfunc("ConstantsHS")]
[maxtessfactor(8.0)]
vertex_output2 HS(InputPatch<vertex_output2, 3> inputPatch, 
                  uint uCPID : SV_OutputControlPointID)
{
    vertex_output2 output = (vertex_output2) 0;
    //  output.pos = inputPatch[uCPID].pos;
    output.wpos = inputPatch[uCPID].wpos; 
    output.normal = inputPatch[uCPID].normal;
    output.binormal = inputPatch[uCPID].binormal;
    output.tangent = inputPatch[uCPID].tangent;
    output.tc = inputPatch[uCPID].tc;
    output.cur_pos = inputPatch[uCPID].cur_pos;
    output.prev_pos = inputPatch[uCPID].prev_pos;

    return output;
}


#ifdef BUILD_FUNC_PS
#define sample(tex, s,  tc) tex.SampleLevel(s, tc, 0);

//tex.Sample(s, tc);

#else
#define sample(tex, s,  tc) tex.SampleLevel(s, tc, 3);

#endif 


#ifdef BUILD_FUNC_DS


//--------------------------------------------------------------------------------------
// Domain Shader
//--------------------------------------------------------------------------------------
float TESS(vertex_output2 i);

[domain("tri")]
vertex_output2 DS(HS_CONSTANT_DATA_OUTPUT input, float3 BarycentricCoordinates : SV_DomainLocation,
                 const OutputPatch<vertex_output2, 3> TrianglePatch)
{
    vertex_output2 output = (vertex_output2) 0;
    // Interpolate world space position with barycentric coordinates
    float3 vWorldPos = BarycentricCoordinates.x * TrianglePatch[0].wpos +
                       BarycentricCoordinates.y * TrianglePatch[1].wpos +
                       BarycentricCoordinates.z * TrianglePatch[2].wpos;
    // Interpolate world space normal and renormalize it
    float3 vNormal = BarycentricCoordinates.x * TrianglePatch[0].normal + 
                     BarycentricCoordinates.y * TrianglePatch[1].normal +
                     BarycentricCoordinates.z * TrianglePatch[2].normal;
    float3 vBiNormal = BarycentricCoordinates.x * TrianglePatch[0].binormal +
                       BarycentricCoordinates.y * TrianglePatch[1].binormal +
                       BarycentricCoordinates.z * TrianglePatch[2].binormal;
    float3 vTangent = BarycentricCoordinates.x * TrianglePatch[0].tangent +
                      BarycentricCoordinates.y * TrianglePatch[1].tangent +
                      BarycentricCoordinates.z * TrianglePatch[2].tangent;
    vNormal = normalize(vNormal);
    vTangent = normalize(vTangent);
    vBiNormal = normalize(vBiNormal);
    // Interpolate other inputs with barycentric coordinates
    output.tc = BarycentricCoordinates.x * TrianglePatch[0].tc +
                BarycentricCoordinates.y * TrianglePatch[1].tc +
                BarycentricCoordinates.z * TrianglePatch[2].tc;


    output.cur_pos = BarycentricCoordinates.x * TrianglePatch[0].cur_pos +
                BarycentricCoordinates.y * TrianglePatch[1].cur_pos +
                BarycentricCoordinates.z * TrianglePatch[2].cur_pos;

    output.dist = BarycentricCoordinates.x * TrianglePatch[0].dist +
                BarycentricCoordinates.y * TrianglePatch[1].dist +
                BarycentricCoordinates.z * TrianglePatch[2].dist;
	output.prev_pos = BarycentricCoordinates.x * TrianglePatch[0].prev_pos +
		BarycentricCoordinates.y * TrianglePatch[1].prev_pos +
		BarycentricCoordinates.z * TrianglePatch[2].prev_pos;

    output.normal = vNormal;
    output.binormal = vBiNormal;
    output.tangent = vTangent;
    float vNormalHeight = TESS(output);
    vWorldPos += vNormalHeight * vNormal / 1;
    output.wpos = vWorldPos;
    output.pos = mul(camera.GetViewProj(), float4(output.wpos.xyz, 1));

    return output;
}   
 

void COMPILED_FUNC(in float3 a, in float2 b, out float c);


float TESS(vertex_output2 i)
{
	float displacement = 0;
	COMPILED_FUNC(i.wpos, i.tc, displacement);

	return displacement;
}

#endif
