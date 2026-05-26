export module HAL:API.PipelineState;
import d3d12;
import wrl;
import Core;
import :Types;
	import <Core_defs.h>;
export namespace HAL
{
	
	
	namespace API
	{

		class TrackedPipeline:public TrackedObject
		{
		public:
			ComPtr<ID3D12PipelineState> m_pipelineState;
			ComPtr<ID3D12StateObject> m_StateObject;

			ComPtr<ID3D12PipelineState> get_native();
			ComPtr<ID3D12StateObject> get_native_state();
		};



		class PipelineStateBase
		{
		public:
			virtual ~PipelineStateBase() = default;
		
			std::string get_cache();

		/*	 std::shared_ptr<TrackedPipeline> get_tracked();*/
				ComPtr<ID3D12PipelineState> get_native();
			ComPtr<ID3D12StateObject> get_native_state();
		};




		class StateObject
		{
		protected:
			ComPtr<ID3D12StateObjectProperties1> stateObjectProperties;

		public:
			D3D12_PROGRAM_IDENTIFIER id;
			uint64 buffer_size;
		};
	}
}