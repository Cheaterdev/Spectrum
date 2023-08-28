export module HAL:Autogen.Layouts.DefaultLayout;
import Core;
import :Autogen.Layouts.FrameLayout;
import :Types;
import :Sampler;
export struct DefaultLayout: public FrameLayout
{
	struct Instance0
	{
		static const uint ID = 4;
		static const uint CB = 18;
		static const uint CB_ID = 12;
		static const uint SRV = 7;
		static const uint SRV_ID = 14;
		static const uint UAV = 4;
		static const uint UAV_ID = 15;
		static inline const std::vector<uint> tables = {12, 14, 15};
	};
	struct Instance1
	{
		static const uint ID = 5;
		static const uint CB = 4;
		static const uint CB_ID = 16;
		static const uint SRV = 12;
		static const uint SRV_ID = 18;
		static const uint UAV = 8;
		static const uint UAV_ID = 19;
		static inline const std::vector<uint> tables = {16, 18, 19};
	};
	struct Instance2
	{
		static const uint ID = 6;
		static const uint CB = 2;
		static const uint CB_ID = 20;
		static const uint SRV = 6;
		static const uint SRV_ID = 22;
		static const uint UAV = 3;
		static const uint UAV_ID = 23;
		static inline const std::vector<uint> tables = {20, 22, 23};
	};
	struct Instance3
	{
		static const uint ID = 7;
		static const uint CB = 4;
		static const uint CB_ID = 24;
		static const uint UAV = 3;
		static const uint UAV_ID = 26;
		static inline const std::vector<uint> tables = {24, 26};
	};
	struct Instance4
	{
		static const uint ID = 8;
		static const uint CB = 2;
		static const uint CB_ID = 27;
		static inline const std::vector<uint> tables = {27};
	};
	struct Instance5
	{
		static const uint ID = 9;
		static const uint CB = 2;
		static const uint CB_ID = 29;
		static inline const std::vector<uint> tables = {29};
	};
	struct Raytracing
	{
		static const uint ID = 10;
		static const uint CB = 2;
		static const uint CB_ID = 31;
		static const uint SRV = 1;
		static const uint SRV_ID = 33;
		static inline const std::vector<uint> tables = {31, 33};
	};
	struct MaterialData
	{
		static const uint ID = 11;
		static const uint CB = 2;
		static const uint CB_ID = 34;
		static const uint SRV = 1;
		static const uint SRV_ID = 36;
		static inline const std::vector<uint> tables = {34, 36};
	};
	template<class Processor> static void for_each(Processor& processor) {
		processor.template process<CameraData,SceneData,PassData,DebugInfo,Instance0,Instance1,Instance2,Instance3,Instance4,Instance5,Raytracing,MaterialData>({HAL::Samplers::SamplerLinearWrapDesc,HAL::Samplers::SamplerPointClampDesc,HAL::Samplers::SamplerLinearClampDesc,HAL::Samplers::SamplerAnisoBorderDesc,HAL::Samplers::SamplerPointBorderDesc});
	}
};
