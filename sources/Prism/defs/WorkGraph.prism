
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

# counter_pad(8) + tile_data(256*256 * sizeof(TileRecord)). Shared between
# RTXShadow's own WorkGraphBuffer [Size=...] below and PassDefaults.cpp's
# emulation FlowGraph nodes (ClassifyFlowNode's tile-buffer view), so the
# two can't drift -- was two separately hand-maintained copies of this same
# formula before.
const WG_TileSection = `8u + 256u * 256u * sizeof(Table::TileRecord)`;

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
	# Field must generate a GetInput() accessor -- workgraph_nodes.jinja's
	# thread-launch wrapper (WGEmul_ThreadInput_T::Get()) hardcodes
	# .GetInput(), unlike the broadcasting-node case which derives its
	# getter from the input type name instead. "input" (lowercase) is a SIG
	# grammar keyword (Node's own `input = Type;` param), so this has to be
	# capitalized. Never actually compiled before this session (this
	# emulation path only builds when native D3D12 work graphs are
	# unavailable, which every dev machine so far has had) -- caught when
	# native support was removed and the emulation became the only path.
	ConsumeStructuredBuffer<TileRecord> Input;
}

[ExcludeVulkan] WorkgraphPSO WorkGR
{
	root = DefaultLayout;

	shader = "dev/workgraph_test.hlsl";

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