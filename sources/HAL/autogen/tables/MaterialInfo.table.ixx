export module HAL:Autogen.Tables.MaterialInfo;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct MaterialInfo
	{
		static constexpr SlotID ID = SlotID::MaterialInfo;
		std::vector<HLSL::Texture2D<float4>> textures;
		DynamicData data;
		std::vector<HLSL::Texture2D<float4>>& GetTextures() { return textures; }
		DynamicData& GetData() { return data; }
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(data);
			compiler.compile(textures);
		}
		};
		#pragma pack(pop)
	}
