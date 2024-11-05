
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


#WorkGraph WorkGR
#{
#	shader = workgraph_test;

	 

#		HAL::StateObjectDesc workgraph;
#		workgraph.type = StateObjectType::WorkGraph;
#		workgraph.global_root = HAL::Device::get().get_engine_pso_holder().GetSignature(Layouts::DefaultLayout);
#		HAL::LibraryObject lib;
#		lib.library = HAL::library_shader::get_resource({
#			std::string("shaders\\workgraph_test.hlsl"), "", HAL::ShaderOptions::None, {}
#		});
#		lib.export_shader(std::wstring(L"ClassifyPixels_Node"));
#		lib.export_shader(std::wstring(L"Shadows_Node"));
#		workgraph.libraries.emplace_back(lib);

#		work_graph = std::make_shared<HAL::StateObject>(workgraph);
		
		
#}


#[Bind = WorkGR]
#EntryPoint WorkEP
#{
#	name = ClassifyPixels_Node;
#	input = GraphInput;
#}