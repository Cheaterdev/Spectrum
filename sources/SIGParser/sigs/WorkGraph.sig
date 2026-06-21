
[Bind = DefaultLayout::Instance0]
struct WorkGraphTest
{
	GBuffer gbuffer;
	RWTexture2D<float4> output;
	RWTexture2D<float4> target;
}

[nobind]
struct GraphInput
{
	[DispatchSize]
	uint3 dispatch_grid;
	uint unused; #fff packing CB<->SRV
	int2 WaveOffset; #override
	uint2 unused2;
}


[nobind]
struct TileRecord
{
	uint2 tileXY;
}

[Bind = DefaultLayout::WorkGR_ClassifyPixels_NodeEmulation]
struct WorkGR_ClassifyPixels_NodeEmulation
{
	GraphInput graphInput;
	uint YZBase;
	AppendStructuredBuffer<TileRecord> Shadows_Node;
}

[Bind = DefaultLayout::WorkGR_Shadows_NodeEmulation]
struct WorkGR_Shadows_NodeEmulation
{
	ConsumeStructuredBuffer<TileRecord> input;
}

[ExcludeVulkan] WorkgraphPSO WorkGR
{
	root = DefaultLayout;

	shader = workgraph_test;

	Node ClassifyPixels_Node
	{
		launch = broadcasting;
		entry = true;
		num_threads = { 64, 1, 1 };
		max_dispatch_grid = { 256, 64, 64 };
		input = GraphInput;
		[MaxRecords = 64]
		NodeOutput TileRecord Shadows_Node;
	}

	Node Shadows_Node
	{
		launch = thread;
		input = TileRecord;
	}
}