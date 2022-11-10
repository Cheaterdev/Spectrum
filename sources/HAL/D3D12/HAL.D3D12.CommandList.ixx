export module HAL:API.CommandList;
import Core;
import :Types;


import :Private.CommandListCompiler;
import :ResourceStates;
import :Resource;
import :DescriptorHeap;
import :Fence;
import :FrameManager;
import :PipelineState;
import :API.IndirectCommand;

export namespace HAL {

	namespace API
	{

		class TransitionCommandList
		{
		protected:
			ComPtr<ID3D12GraphicsCommandList4> m_commandList;
			ComPtr<ID3D12CommandAllocator> m_commandAllocator;
			
		public:
			ComPtr<ID3D12GraphicsCommandList4> get_native();
		};
	}



}