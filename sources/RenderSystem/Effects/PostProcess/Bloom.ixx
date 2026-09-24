export module Graphics:Bloom;

import FrameGraph;

// Mirrors Bloom.cpp's Variable<>s into Table::Post::BloomSelectors for the
// generated setup (bloom.prism). Must run before graph.setup() -- called from
// main.cpp next to tonemap_update_selectors().
export void bloom_update_selectors(FrameGraph::Graph& graph);
