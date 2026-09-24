export module Graphics:Tonemap;

import FrameGraph;

// Mirrors Tonemap.cpp's Variable<>s into Table::Post::TonemapSelectors for the
// generated setup (tonemap.prism). Must run before graph.setup() -- called
// from main.cpp next to ddgi_update_selectors().
export void tonemap_update_selectors(FrameGraph::Graph& graph);
