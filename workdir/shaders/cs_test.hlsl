
static const uint TOTAL_THREADS = SIZE * SIZE;
Texture2D<float4> orig : register(t0);
RWTexture2D<float4> tex[5] : register(u0);
groupshared float4 shared_mem[SIZE][SIZE];


[numthreads(SIZE, SIZE, 1)]
void CS(uint3 group_id :  SV_GroupID, uint3 thread_id : SV_GroupThreadID)
{
    const uint2 tc = (group_id.xy * SIZE + thread_id.xy);
    float4 c = (orig[tc.xy * 2] + orig[tc.xy * 2 + uint2(1, 0)] + orig[tc.xy * 2 + uint2(0, 1)] + orig[tc.xy * 2 + uint2(1, 1)]) / 4;
    shared_mem[thread_id.x][thread_id.y] = c;
    tex[0][tc] = c;
    uint2 pos = thread_id;
    uint2 pos2 = thread_id * 2;
//   [unroll(TOTAL_THREADS)]

    for (uint s = SIZE / 2, i = 1, t = 1; s > 0; s >>= 1, i <<= 1, t++)
    {
        GroupMemoryBarrierWithGroupSync();
        pos2 = 2 * pos * i;

        if (pos.x < s && pos.y < s)
        {
            float4 color = shared_mem[pos2.x][pos2.y];
            color += shared_mem[pos2.x + i ][pos2.y];
            color += shared_mem[pos2.x][pos2.y + i];
            color += shared_mem[pos2.x + i ][pos2.y + i];
            color /= 4;
            shared_mem[pos2.x][pos2.y] = color;
            tex[t][group_id * SIZE / (i * 2) + pos] = color;
        }
    }
}




