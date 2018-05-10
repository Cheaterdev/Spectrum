#ifdef BUILD_FUNC_CS
StructuredBuffer<float> Exposure : register( t0 );
//Texture2D<float3> Bloom : register( t1 );
RWTexture2D<float3> ColorRW : register( u0 );
RWTexture2D<uint> OutLuma : register( u1 );
SamplerState LinearSampler : register( s0 );

cbuffer cb : register( b0 )
{
	float2 g_RcpBufferDim;
	float g_BloomStrength;
	float g_ToeStrength;
};


// Encodes a smooth logarithmic gradient for even distribution of precision natural to vision
float LinearToLogLuminance( float x, float gamma = 4.0 )
{
	return log2(lerp(1, exp2(gamma), x)) / gamma;
}

// This assumes the default color gamut found in sRGB and REC709.  The color primaries determine these
// coefficients.  Note that this operates on linear values, not gamma space.
float RGBToLuminance( float3 x )
{
	return dot( x, float3(0.212671, 0.715160, 0.072169) );		// Defined by sRGB/Rec.709 gamut
}
float ToneMapLuma( float luma )
{
	return 1 - exp2(-luma);
}
[numthreads( 8, 8, 1 )]
void CS( uint3 DTid : SV_DispatchThreadID )
{
	float2 TexCoord = (DTid.xy + 0.5) * g_RcpBufferDim;

	// Load HDR and bloom
	float3 hdrColor = ColorRW[DTid.xy];


	//hdrColor += g_BloomStrength * Bloom.SampleLevel(LinearSampler, TexCoord, 0);
	hdrColor *= Exposure[0];


	ColorRW[DTid.xy] = hdrColor;//float(OutLuma[DTid.xy])/255;//hdrColor;

	//OutLuma[DTid.xy] = 2;//LinearToLogLuminance(ToneMapLuma(RGBToLuminance(hdrColor)));

}
#else

ByteAddressBuffer Histogram : register( t0 );
StructuredBuffer<float> Exposure : register( t1 );
RWTexture2D<float3> ColorBuffer : register( u0 );

groupshared uint gs_hist[256];

[numthreads( 256, 1, 1 )]
void HISTO_CS( uint GI : SV_GroupIndex, uint3 DTid : SV_DispatchThreadID )
{
	uint histValue = Histogram.Load(GI * 4);

	// Compute the maximum histogram value, but don't include the black pixel
	gs_hist[GI] = GI == 0 ? 0 : histValue;
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 128) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 64) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 32) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 16) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 8) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 4) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 2) % 256]);
	GroupMemoryBarrierWithGroupSync();
	gs_hist[GI] = max(gs_hist[GI], gs_hist[(GI + 1) % 256]);
	GroupMemoryBarrierWithGroupSync();

	uint maxHistValue = gs_hist[GI];

	uint2 BufferDim;
	ColorBuffer.GetDimensions(BufferDim.x, BufferDim.y);

	const uint2 RectCorner = uint2(BufferDim.x / 2 - 512, BufferDim.y - 256);
	const uint2 GroupCorner = RectCorner + DTid.xy * 4;

	uint height = 127 - DTid.y * 4;
	uint threshold = histValue * 128 / max(1, maxHistValue);

	float3 OutColor = (GI == (uint)Exposure[3]) ? float3(1.0, 1.0, 0.0) : float3(0.5, 0.5, 0.5);

	for (uint i = 0; i < 4; ++i)
	{
		float3 MaskedColor = (height - i) < threshold ? OutColor : float3(0, 0, 0);

		// 4-wide column with 2 pixels for the histogram bar and 2 for black spacing
		ColorBuffer[GroupCorner + uint2(0, i)] = MaskedColor;
		ColorBuffer[GroupCorner + uint2(1, i)] = MaskedColor;
		ColorBuffer[GroupCorner + uint2(2, i)] = float3(0, 0, 0);
		ColorBuffer[GroupCorner + uint2(3, i)] = float3(0, 0, 0);
	}
}
#endif