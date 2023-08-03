export module HAL:Autogen.Tables.MaterialCommandData;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct MaterialCommandData
	{
		static constexpr SlotID ID = SlotID::MaterialCommandData;
		uint material_cb;
		uint pipeline_id;
		uint& GetMaterial_cb() { return material_cb; }
		uint& GetPipeline_id() { return pipeline_id; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(material_cb);
			compiler.compile(pipeline_id);
		}
		using Compiled = MaterialCommandData;
		};
		#pragma pack(pop)
	}
