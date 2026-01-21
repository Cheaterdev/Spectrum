export module HAL:API.PipelineState;
import d3d12;
import wrl;
import Core;
import :Types;

export namespace HAL
{
	
	
	namespace API
	{

		class TrackedPipeline:public TrackedObject
		{
		public:
			ComPtr<ID3D12PipelineState> m_pipelineState;
			ComPtr<ID3D12StateObject> m_StateObject;
		};



		class PipelineStateBase
		{

		public:
			virtual ~PipelineStateBase() = default;
			ComPtr<ID3D12PipelineState> get_native();
			ComPtr<ID3D12StateObject> get_native_state();

			std::string get_cache();
		};




		class StateObject
		{
		protected:
			ComPtr<ID3D12StateObjectProperties1> stateObjectProperties;

		public:
			D3D12_PROGRAM_IDENTIFIER id;
			uint buffer_size;
		};
	}
}