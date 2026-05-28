module GUI:FlowSystem;

::FlowGraph::window::ptr FlowGraph::FlowSystem::desc::create()
{
    return create_func();
}

FlowGraph::FlowSystem::FlowSystem()
{
    register_node<::FlowGraph::window>("comment");
}

std::vector<FlowGraph::FlowSystem::desc>& FlowGraph::FlowSystem::get_all()
{
    return creators;
}
