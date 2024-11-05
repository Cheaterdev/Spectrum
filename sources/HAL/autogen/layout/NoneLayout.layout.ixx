export module HAL:Autogen.Layouts.NoneLayout;
import Core;
import :Types;
import :Sampler;

export struct NoneLayout
{
	struct None
	{
		static const uint ID = 0;
		static const uint CB = 4;
		static const uint CB_ID = 0;
		static inline const std::vector<uint> tables = { 0 };
	};

	template<class Processor> static void for_each(Processor& processor)
	{
		processor.template process<None>({  });
	}
};