export module Graphics:Bloom;

import FrameGraph;

// Mirrors Bloom.cpp's Variable<>s into Table::Post::BloomSelectors for the
// generated setup (bloom.prism). Must run before graph.setup() -- called from
// main.cpp next to tonemap_update_selectors().
export void bloom_update_selectors(FrameGraph::Graph& graph);

// The Variable<>s' current values, for the pass bodies (BloomPass.cpp). Those are `module FrameGraph;` units --
// the module of the classes whose members they define -- and cannot see this module's file-local settings.
export struct BloomSettings
{
	int   mode;   // Post::BloomMode
	float intensity;
	float threshold;
	float knee;
	float scatter;
	float radius;
	int   levels;
	bool  temporal;
	float temporal_weight;
	bool  karis;
};

export BloomSettings bloom_settings();
