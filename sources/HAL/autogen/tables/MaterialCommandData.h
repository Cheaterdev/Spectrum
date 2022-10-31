#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct MaterialCommandData
	{
		uint pipeline_id;
		GPUAddress material_cb;
		uint& GetPipeline_id() { return pipeline_id; }
		GPUAddress& GetMaterial_cb() { return material_cb; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(pipeline_id);
			compiler.compile(material_cb);
		}
	};
	#pragma pack(pop)
}
