export module HAL:Autogen.Tables.FrameGraph_Debug_Common;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
import <HAL.h>;
export namespace Table
{
	#pragma pack(push, 1)

	struct FrameGraph_Debug_Common
	{
		static constexpr SlotID ID = SlotID::FrameGraph_Debug_Common;
		uint2 targetSize;
		float3 minColor;
		float3 maxColor;
		uint selectedMip;
		uint selectedArrayIndex;
		HLSL::RWTexture2D<float4> target;
		HLSL::RWTexture2D<float4>& GetTarget() { return target; }
		uint2& GetTargetSize() { return targetSize; }
		float3& GetMinColor() { return minColor; }
		float3& GetMaxColor() { return maxColor; }
		uint& GetSelectedMip() { return selectedMip; }
		uint& GetSelectedArrayIndex() { return selectedArrayIndex; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(targetSize);
			compiler.compile(minColor);
			compiler.compile(maxColor);
			compiler.compile(selectedMip);
			compiler.compile(selectedArrayIndex);
			compiler.compile(target);
		}
		struct Compiled
		{
			uint2 targetSize; // uint2
			float3 minColor; // float3
			float3 maxColor; // float3
			uint selectedMip; // uint
			uint selectedArrayIndex; // uint
			uint target; // RWTexture2D<float4>

			
			private:
			SERIALIZE()
			{
				ar& NVP(targetSize);
				ar& NVP(minColor);
				ar& NVP(maxColor);
				ar& NVP(selectedMip);
				ar& NVP(selectedArrayIndex);
			}


		};

		static std::string get_typename()
		{
			return "Tables::FrameGraph_Debug_Common";
		}
		private:
		SERIALIZE()
		{
			ar& NVP(targetSize);
			ar& NVP(minColor);
			ar& NVP(maxColor);
			ar& NVP(selectedMip);
			ar& NVP(selectedArrayIndex);
		}

	};
	#pragma pack(pop)
}

