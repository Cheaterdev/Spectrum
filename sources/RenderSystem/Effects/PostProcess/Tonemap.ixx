export module Graphics:Tonemap;

import FrameGraph;

// Mirrors Tonemap.cpp's Variable<>s into Table::Post::TonemapSelectors for the
// generated setup (tonemap.prism). Must run before graph.setup() -- called
// from main.cpp next to ddgi_update_selectors().
export void tonemap_update_selectors(FrameGraph::Graph& graph);

// The Variable<>s' current values, for the pass body (TonemapPass.cpp). That body is a
// `module FrameGraph;` unit -- the module of the class whose members it defines -- and so
// cannot see this module's file-local settings.
export struct TonemapSettings
{
	int   tonemap_operator;   // Post::TonemapOperator
	bool  auto_exposure;
	float compensation;
	float manual_ev;
	float speed_up;
	float speed_down;
	float low_percent;
	float high_percent;
	float min_ev;
	float max_ev;
	bool  hdr_output;
	float paper_white_nits;
	float peak_override_nits;   // 0 = the display's reported peak
};

export TonemapSettings tonemap_settings();

// True once after a request to snap exposure to the metered value (first frame, auto exposure toggled on).
export bool tonemap_take_reset();
