export module HAL:Autogen.Tables.ReflectionCombine;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.GBuffer;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct ReflectionCombine
	{
		static constexpr SlotID ID = SlotID::ReflectionCombine;
		HLSL::Texture2D<float4> reflection;
		HLSL::RWTexture2D<float4> target;
		GBuffer gbuffer;
		HLSL::Texture2D<float4>& GetReflection() { return reflection; }
		HLSL::RWTexture2D<float4>& GetTarget() { return target; }
		GBuffer& GetGbuffer() { return gbuffer; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(reflection);
			compiler.compile(target);
			compiler.compile(gbuffer);
		}
		struct Compiled
		{
			uint reflection; // Texture2D<float4>
			uint target; // RWTexture2D<float4>
			GBuffer::Compiled gbuffer; // GBuffer
		};
	};
	#pragma pack(pop)
}
