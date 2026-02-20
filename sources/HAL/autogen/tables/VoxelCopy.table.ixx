export module HAL:Autogen.Tables.VoxelCopy;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
import <HAL.h>;
import :Autogen.Tables.VoxelTilingParams;
export namespace Table
{
	#pragma pack(push, 1)

	struct VoxelCopy
	{
		static constexpr SlotID ID = SlotID::VoxelCopy;
		HLSL::Texture3D<float4> Source[2];
		HLSL::RWTexture3D<float4> Target[2];
		VoxelTilingParams params;
		HLSL::RWTexture3D<float4>* GetTarget() { return Target; }
		HLSL::Texture3D<float4>* GetSource() { return Source; }
		VoxelTilingParams& GetParams() { return params; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(Source);
			compiler.compile(Target);
			compiler.compile(params);
		}
		struct Compiled
		{
			uint Source[2]; // Texture3D<float4>
			uint Target[2]; // RWTexture3D<float4>
			VoxelTilingParams::Compiled params; // VoxelTilingParams

			
			private:
			SERIALIZE()
			{
				ar& NVP(params);
			}


		};

		static std::string get_typename()
		{
			return "Tables::VoxelCopy";
		}
		private:
		SERIALIZE()
		{
			ar& NVP(params);
		}

	};
	#pragma pack(pop)
}

