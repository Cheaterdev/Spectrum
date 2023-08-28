export module HAL:Autogen.Tables.PSSMData;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.Camera;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct PSSMData
	{
		static constexpr SlotID ID = SlotID::PSSMData;
		HLSL::Texture2DArray<float> light_buffer;
		HLSL::StructuredBuffer<Camera> light_cameras;
		HLSL::Texture2DArray<float>& GetLight_buffer() { return light_buffer; }
		HLSL::StructuredBuffer<Camera>& GetLight_cameras() { return light_cameras; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(light_buffer);
			compiler.compile(light_cameras);
		}
		struct Compiled
		{
			uint light_buffer; // Texture2DArray<float>
			uint light_cameras; // StructuredBuffer<Camera>
		};
	};
	#pragma pack(pop)
}
