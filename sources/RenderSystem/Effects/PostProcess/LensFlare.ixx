export module Graphics:LensFlare;

import FrameGraph;

// Mirrors LensFlare.cpp's Variable<>s into Table::Post::LensFlareSelectors for
// the generated setup (lens_flare.prism). Must run before graph.setup() --
// called from main.cpp next to bloom_update_selectors().
export void lens_flare_update_selectors(FrameGraph::Graph& graph);
