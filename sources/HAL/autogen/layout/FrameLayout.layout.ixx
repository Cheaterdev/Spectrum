export module HAL:Autogen.Layouts.FrameLayout;
import Core;
import :Types;
import :Sampler;
export struct FrameLayout
{
	struct CameraData
	{
		static const uint ID = 0;
		static const uint CB = 34;
		static const uint CB_ID = 0;
		static const uint SRV = 3;
		static const uint SRV_ID = 2;
		static inline const std::vector<uint> tables = {0, 2};
	};
	struct SceneData
	{
		static const uint ID = 1;
		static const uint CB = 2;
		static const uint CB_ID = 3;
		static const uint SRV = 4;
		static const uint SRV_ID = 5;
		static inline const std::vector<uint> tables = {3, 5};
	};
	struct PassData
	{
		static const uint ID = 2;
		static const uint CB = 2;
		static const uint CB_ID = 6;
		static const uint SRV = 1;
		static const uint SRV_ID = 8;
		static inline const std::vector<uint> tables = {6, 8};
	};
	struct DebugInfo
	{
		static const uint ID = 3;
		static const uint CB = 2;
		static const uint CB_ID = 9;
		static const uint UAV = 1;
		static const uint UAV_ID = 11;
		static inline const std::vector<uint> tables = {9, 11};
	};
	template<class Processor> static void for_each(Processor& processor) {
		processor.template process<CameraData,SceneData,PassData,DebugInfo>({HAL::Samplers::SamplerLinearWrapDesc,HAL::Samplers::SamplerPointClampDesc,HAL::Samplers::SamplerLinearClampDesc,HAL::Samplers::SamplerAnisoBorderDesc,HAL::Samplers::SamplerPointBorderDesc});
	}
};
