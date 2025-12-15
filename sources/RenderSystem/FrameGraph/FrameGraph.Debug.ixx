export module FrameGraphDebug;

export import "defines.h";
import Core;
import HAL;
import <HAL.h>;

import GUI;			  
import FrameGraph;
			import windows;

using namespace HAL;

export class FrameGraphDebug
{
public:
	static GUI::base::ptr  create_debug_layout(FrameGraph::Graph& graph);

};

