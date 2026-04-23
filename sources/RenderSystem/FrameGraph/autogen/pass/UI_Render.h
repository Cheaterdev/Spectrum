
using namespace FrameGraph;

namespace Passes
{

class UI_Render 
{
public:
	struct Context
	{

		Handlers::Texture H(swapchain);

	};


	static inline const wchar_t* Name = L"UI_Render";

	static constexpr uint32_t MaxCount = 16;
	static inline const wchar_t* Names[MaxCount] = {
		L"UI_Render_0",
		L"UI_Render_1",
		L"UI_Render_2",
		L"UI_Render_3",
		L"UI_Render_4",
		L"UI_Render_5",
		L"UI_Render_6",
		L"UI_Render_7",
		L"UI_Render_8",
		L"UI_Render_9",
		L"UI_Render_10",
		L"UI_Render_11",
		L"UI_Render_12",
		L"UI_Render_13",
		L"UI_Render_14",
		L"UI_Render_15",
	};

//	static constexpr PassID ID = PassID::UI_Render;


	using setup_func_type = std::function<bool(Context&, FrameGraph::TaskBuilder&)>;
	using render_func_type = std::function<void(Context&, FrameGraph::FrameContext&)>;
	
	setup_func_type setup_func;
	render_func_type render_func;
};

}