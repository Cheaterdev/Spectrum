    

[Bind = DefaultLayout::Instance0]
struct DenoiserShadow_Prepare
{
	int2 BufferDimensions;
	Texture2D<uint> t2d_hitMaskResults;
	RWStructuredBuffer<uint> rwsb_shadowMask;
}

ComputePSO DenoiserShadow_Prepare
{
	root = DefaultLayout;

	[EntryPoint = main]
	compute = denoiser/prepare_shadow_mask_d3d12;
}




[Bind = DefaultLayout::Instance0]
struct DenoiserShadow_TileClassification
{
	float3   Eye;
	int      FirstFrame;
	int2     BufferDimensions;
	float2   InvBufferDimensions;
	float4x4 ProjectionInverse;
	float4x4 ReprojectionMatrix;
	float4x4 ViewProjectionInverse;
	float    DepthSimilaritySigma;


	Texture2D<float>            t2d_depth;
	Texture2D<float2>           t2d_velocity;
	Texture2D<float3>           t2d_normal;
	Texture2D<float2>           t2d_history;
	Texture2D<float>            t2d_previousDepth;
	StructuredBuffer<uint>      sb_raytracerResult;


	RWStructuredBuffer<uint>    rwsb_tileMetaData;
	RWTexture2D<float2>         rwt2d_reprojectionResults; 

	Texture2D<float3>           t2d_previousMoments;
	RWTexture2D<float3>         rwt2d_momentsBuffer; 
}


ComputePSO DenoiserShadow_TileClassification
{
	root = DefaultLayout;

	[EntryPoint = main]
	compute = denoiser/tile_classification_d3d12;
}


[Bind = DefaultLayout::Instance0]
struct DenoiserShadow_Filter
{
	float4x4 ProjectionInverse;
	int2     BufferDimensions;
	float2   InvBufferDimensions;
	float    DepthSimilaritySigma;

	Texture2D<float>       t2d_DepthBuffer;
	Texture2D<float16_t3>  t2d_NormalBuffer;
	StructuredBuffer<uint>  sb_tileMetaData;

}


[Bind = DefaultLayout::Instance1]
struct DenoiserShadow_FilterLocal
{
	Texture2D<float16_t2>  rqt2d_input;
	RWTexture2D<float2>  rwt2d_output;
}



[Bind = DefaultLayout::Instance1]
struct DenoiserShadow_FilterLast
{
	Texture2D<float16_t2>  rqt2d_input;
	RWTexture2D<unorm float4>  rwt2d_output;
}
ComputePSO DenoiserShadow_Filter
{
	root = DefaultLayout;

	[EntryPoint = main]
	[Enable16bits]
	compute = denoiser/filter_soft_shadows_pass_d3d12;

	[CS]
	define Pass = {0,1,2};
}
