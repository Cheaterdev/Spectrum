
[Bind = DefaultLayout::Instance0]
struct WorkGraphTest
{
	GBuffer gbuffer;
	RWTexture2D<float4> output;
	RWTexture2D<float4> target;
}

[Bind = NoneLayout::None]
struct GraphInput
{
	[DispatchSize]
	uint3 dispatch_grid;
	uint unused; #fff packing CB<->SRV
	int2 WaveOffset; #override
	uint2 unused2;
}


WorkgraphPSO WorkGR
{
	root = DefaultLayout;

	shader = workgraph_test;		
}


#[Bind = WorkGR]
#EntryPoint WorkEP
#{
#	name = ClassifyPixels_Node;
#	input = GraphInput;
#