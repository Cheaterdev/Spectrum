export module HAL:Autogen.Tables.WorkGraphTest;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
import <HAL.h>;
import :Autogen.Tables.GBuffer;
export namespace Table
{
	#pragma pack(push, 1)

	struct WorkGraphTest
	{
		static constexpr SlotID ID = SlotID::WorkGraphTest;
		HLSL::RWTexture2D<float4> output;
		HLSL::RWTexture2D<float4> target;
		GBuffer gbuffer;
		HLSL::RWTexture2D<float4>& GetOutput() { return output; }
		HLSL::RWTexture2D<float4>& GetTarget() { return target; }
		GBuffer& GetGbuffer() { return gbuffer; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(output);
			compiler.compile(target);
			compiler.compile(gbuffer);
		}
		struct Compiled
		{
			uint output; // RWTexture2D<float4>
			uint target; // RWTexture2D<float4>
			GBuffer::Compiled gbuffer; // GBuffer

			
			private:
			SERIALIZE()
			{
				ar& NVP(gbuffer);
			}


		};


		private:
		SERIALIZE()
		{
			ar& NVP(gbuffer);
		}

	};
	#pragma pack(pop)
}

