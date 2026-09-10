export module FrameGraph:ViewportContext;

import Core;
import HAL;

export
{
	// Viewport dimensions set once per frame before graph setup.
	//
	// Lives in the FrameGraph module (not Graphics, where it conceptually
	// belongs) because the SIG-generated create_always()/need_always() calls
	// resolving [Size=ViewportContext::frame_size] live in FrameGraph:Passes
	// (compiled as part of this module, from the pass/*.h headers included
	// into passes.ixx) and call get_context<Table::ViewportContext>()
	// directly. ContextTypeFor's specialization below must be visible AT
	// THE POINT that template is instantiated -- which is here, inside
	// FrameGraph's own compiled module interface -- or the primary (identity)
	// template silently wins instead, handing create_always() a disconnected,
	// always-zeroed Table::ViewportContext instead of the live ViewportInfo
	// Graphics-level code populates. A specialization declared up in
	// Graphics (where this used to live) is never seen from here: FrameGraph
	// does not and must not import Graphics.
	struct ViewportInfo : Table::ViewportContext
	{
	};

	template<> struct ContextTypeFor<Table::ViewportContext> { using type = ViewportInfo; };
}
