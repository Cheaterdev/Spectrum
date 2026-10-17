// Test.HAL.Rendering RenderCube: 6 faces × 2 tris × 3 verts = 36, each face a distinct color.
// VS builds clip-space position from SV_VertexID using hardcoded positions +
// a fixed rotation + reversed-depth perspective (near=1, far=20, fovY=60°).
static const float3 kPos[36] = {
    // Front (+Z)
    float3(-0.5,-0.5, 0.5), float3( 0.5,-0.5, 0.5), float3( 0.5, 0.5, 0.5),
    float3(-0.5,-0.5, 0.5), float3( 0.5, 0.5, 0.5), float3(-0.5, 0.5, 0.5),
    // Back (-Z)
    float3( 0.5,-0.5,-0.5), float3(-0.5,-0.5,-0.5), float3(-0.5, 0.5,-0.5),
    float3( 0.5,-0.5,-0.5), float3(-0.5, 0.5,-0.5), float3( 0.5, 0.5,-0.5),
    // Left (-X)
    float3(-0.5,-0.5,-0.5), float3(-0.5,-0.5, 0.5), float3(-0.5, 0.5, 0.5),
    float3(-0.5,-0.5,-0.5), float3(-0.5, 0.5, 0.5), float3(-0.5, 0.5,-0.5),
    // Right (+X)
    float3( 0.5,-0.5, 0.5), float3( 0.5,-0.5,-0.5), float3( 0.5, 0.5,-0.5),
    float3( 0.5,-0.5, 0.5), float3( 0.5, 0.5,-0.5), float3( 0.5, 0.5, 0.5),
    // Top (+Y)
    float3(-0.5, 0.5, 0.5), float3( 0.5, 0.5, 0.5), float3( 0.5, 0.5,-0.5),
    float3(-0.5, 0.5, 0.5), float3( 0.5, 0.5,-0.5), float3(-0.5, 0.5,-0.5),
    // Bottom (-Y)
    float3(-0.5,-0.5,-0.5), float3( 0.5,-0.5,-0.5), float3( 0.5,-0.5, 0.5),
    float3(-0.5,-0.5,-0.5), float3( 0.5,-0.5, 0.5), float3(-0.5,-0.5, 0.5),
};
static const float4 kColor[6] = {
    float4(1,0,0,1), float4(0,1,0,1), float4(0,0,1,1),
    float4(1,1,0,1), float4(1,0,1,1), float4(0,1,1,1),
};
struct VSOut { float4 pos : SV_Position; float4 col : COLOR0; };
VSOut VS(uint vid : SV_VertexID)
{
    float3 p = kPos[vid];
    // Rotate 45 deg around Y  (cos=0.7071, sin=0.7071)
    float3 q;
    q.x =  p.x * 0.7071 + p.z * 0.7071;
    q.y =  p.y;
    q.z = -p.x * 0.7071 + p.z * 0.7071;
    p = q;
    // Rotate 30 deg around X  (cos=0.8660, sin=0.5)
    q.x =  p.x;
    q.y =  p.y * 0.8660 - p.z * 0.5;
    q.z =  p.y * 0.5    + p.z * 0.8660;
    p = q;
    // Place cube at z=2.5 (camera at origin, looking +Z)
    p.z += 2.5;
    // Reversed-depth perspective: near=1, far=20, fovY=60 deg (f=cot(30)=1.7321)
    // A = -near/(far-near) = -1/19, B = far/(far-near) = 20/19
    float f = 1.7321;
    float A = -0.052632;
    float B =  1.052632;
    // 1080/1920: the target is kReferenceWidth x kReferenceHeight.
    float aspect_x = 0.5625;
    VSOut o;
    o.pos = float4(p.x * f * aspect_x, p.y * f, p.z * A + B, p.z);
    o.col = kColor[vid / 6];
    return o;
}
float4 PS(VSOut i) : SV_Target { return i.col; }
