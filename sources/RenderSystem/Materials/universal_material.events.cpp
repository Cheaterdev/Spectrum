module Graphics:Materials.UniversalMaterial;
import Core;
import HAL;

static IdGenerator ids;
using namespace HAL;

// ---------------------------------------------------------------------------
// Trivial accessors
// ---------------------------------------------------------------------------

size_t materials::universal_material::get_id()
{
	return pipeline->get_id(); ///TODO: change for graph id
}

MaterialGraph::ptr materials::universal_material::get_graph()
{
	return graph.get();
}

void materials::universal_material::test()
{
	graph.test();
}


// ---------------------------------------------------------------- -----------
// Default constructor (serialization / asset-system path)
// ---------------------------------------------------------------------------

materials::universal_material::universal_material() : include_file(this), include_file_raytacing(this)
{
	wshader_name = std::wstring(L"material_") + std::to_wstring(ids.get());
	graph.on_create = [this](MaterialGraph::ptr g)
	{
		g->add_listener(this, false);
		on_graph_changed();
	};
}


// ---------------------------------------------------------------------------
// Change notifications
// ---------------------------------------------------------------------------

void materials::universal_material::on_graph_changed()
{
	need_regenerate_material = true;
}

void materials::universal_material::on_asset_change(std::shared_ptr<Asset> asset)
{
	if (asset == *include_file || asset == *include_file_raytacing)
		on_graph_changed();

	if (asset->get_type() == Asset_Type::TEXTURE)
		on_graph_changed();
}


// ---------------------------------------------------------------------------
// FlowGraph callbacks — all delegate to on_graph_changed()
// ---------------------------------------------------------------------------

void materials::universal_material::on_register(::FlowGraph::window*)
{
	on_graph_changed();
}

void materials::universal_material::on_remove(::FlowGraph::window*)
{
	on_graph_changed();
}

void materials::universal_material::on_add_input(::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_remove_input(::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_add_output(::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_remove_output(::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_link(::FlowGraph::parameter*, ::FlowGraph::parameter*)
{
	on_graph_changed();
}

void materials::universal_material::on_unlink(::FlowGraph::parameter*, ::FlowGraph::parameter*)
{
	on_graph_changed();
}
