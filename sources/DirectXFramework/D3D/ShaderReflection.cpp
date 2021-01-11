#include "pch.h"
#include "ShaderReflection.h"
namespace D3D
{
    D3D_SHADER_DESC::D3D_SHADER_DESC(const D3D12_SHADER_DESC& desc)

    {
        Version = desc.Version;
        Creator = desc.Creator? desc.Creator:"Unknown";
        Flags = desc.Flags;
        ConstantBuffers = desc.ConstantBuffers;
        BoundResources = desc.BoundResources;
        InputParameters = desc.InputParameters;
        OutputParameters = desc.OutputParameters;
        InstructionCount = desc.InstructionCount;
        TempRegisterCount = desc.TempRegisterCount;
        TempArrayCount = desc.TempArrayCount;
        DefCount = desc.DefCount;
        DclCount = desc.DclCount;
        TextureNormalInstructions = desc.TextureNormalInstructions;
        TextureLoadInstructions = desc.TextureLoadInstructions;
        TextureCompInstructions = desc.TextureCompInstructions;
        TextureBiasInstructions = desc.TextureBiasInstructions;
        TextureGradientInstructions = desc.TextureGradientInstructions;
        FloatInstructionCount = desc.FloatInstructionCount;
        IntInstructionCount = desc.IntInstructionCount;
        UintInstructionCount = desc.UintInstructionCount;
        StaticFlowControlCount = desc.StaticFlowControlCount;
        DynamicFlowControlCount = desc.DynamicFlowControlCount;
        MacroInstructionCount = desc.MacroInstructionCount;
        ArrayInstructionCount = desc.ArrayInstructionCount;
        CutInstructionCount = desc.CutInstructionCount;
        EmitInstructionCount = desc.EmitInstructionCount;
        GSOutputTopology = desc.GSOutputTopology;
        GSMaxOutputVertexCount = desc.GSMaxOutputVertexCount;
        InputPrimitive = desc.InputPrimitive;
        PatchConstantParameters = desc.PatchConstantParameters;
        cGSInstanceCount = desc.cGSInstanceCount;
        cControlPoints = desc.cControlPoints;
        HSOutputPrimitive = desc.HSOutputPrimitive;
        HSPartitioning = desc.HSPartitioning;
        TessellatorDomain = desc.TessellatorDomain;
        cBarrierInstructions = desc.cBarrierInstructions;
        cInterlockedInstructions = desc.cInterlockedInstructions;
        cTextureStoreInstructions = desc.cTextureStoreInstructions;
    }



    D3D_SHADER_VARIABLE_DESC::D3D_SHADER_VARIABLE_DESC(const D3D12_SHADER_VARIABLE_DESC& desc)
    {
        Name = desc.Name;
        StartOffset = desc.StartOffset;
        Size = desc.Size;
        uFlags = desc.uFlags;
        DefaultValue = desc.DefaultValue;
        StartTexture = desc.StartTexture;
        TextureSize = desc.TextureSize;
        StartSampler = desc.StartSampler;
        SamplerSize = desc.SamplerSize;
    }


    D3D_SHADER_INPUT_BIND_DESC::D3D_SHADER_INPUT_BIND_DESC(const D3D12_SHADER_INPUT_BIND_DESC& desc)
    {
        Name = desc.Name;
        Type = desc.Type;
        BindPoint = desc.BindPoint;
        BindCount = desc.BindCount;
        uFlags = desc.uFlags;
        ReturnType = desc.ReturnType;
        Dimension = desc.Dimension;
        NumSamples = desc.NumSamples;
        Space = desc.Space;
        uID = desc.uID;
    }

    D3D_SHADER_BUFFER_DESC::D3D_SHADER_BUFFER_DESC(const D3D12_SHADER_BUFFER_DESC& desc)
    {
        Name = desc.Name;
        Type = desc.Type;
        Variables = desc.Variables;
        Size = desc.Size;
        uFlags = desc.uFlags;
    }


    D3D_SIGNATURE_PARAMETER_DESC::D3D_SIGNATURE_PARAMETER_DESC(const D3D12_SIGNATURE_PARAMETER_DESC& desc)
    {
        SemanticName = desc.SemanticName;
        SemanticIndex = desc.SemanticIndex;
        Register = desc.Register;
        SystemValueType = desc.SystemValueType;
        ComponentType = desc.ComponentType;
        Mask = desc.Mask;
        ReadWriteMask = desc.ReadWriteMask;
        Stream = desc.Stream;
        MinPrecision = desc.MinPrecision;
    }

    UINT reflection::get_interface_slots_count()
    {
        return interface_slots;
    }
    reflection::reflection(ComPtr<ID3D12ShaderReflection> pReflector) {
     /*   if (!pReflector) return;
		D3D12_SHADER_DESC _desc;
		pReflector->GetDesc(&_desc);
		desc = D3D::D3D_SHADER_DESC(_desc);
		interface_slots = pReflector->GetNumInterfaceSlots();
		//  inputs.resize(desc.InputParameters);

		for (unsigned int i = 0; i < desc.InputParameters; i++)
		{
			D3D12_SIGNATURE_PARAMETER_DESC sigDesc;
			pReflector->GetInputParameterDesc(i, &sigDesc);
			inputs.emplace_back(sigDesc);
		}

		for (unsigned int i = 0; i < desc.ConstantBuffers; i++)
		{
			D3D12_SHADER_BUFFER_DESC  desc;
			auto buffer = pReflector->GetConstantBufferByIndex(i);
			buffer->GetDesc(&desc);
			const_buffers.emplace_back(desc);

			for (unsigned int i = 0; i < const_buffers.back().Variables; i++)
			{
				ID3D12ShaderReflectionVariable* var = buffer->GetVariableByIndex(i);
				D3D12_SHADER_VARIABLE_DESC desc;
				var->GetDesc(&desc);
				const_buffers.back().variables.emplace_back(desc);
				// TODO!
             
				const_buffers.back().variables.back().interface_slots.emplace_back(var->GetInterfaceSlot(0));
			}
		}

		for (unsigned int i = 0; i < desc.BoundResources; i++)
		{
			D3D12_SHADER_INPUT_BIND_DESC BindDesc;
			pReflector->GetResourceBindingDesc(i, &BindDesc);
			input_bind.emplace_back(BindDesc);
		}*/
    }


    reflection::reflection(D3D_VERSION ver, std::string data)
    {
#ifdef USE_D3D_COMPILER

        if (ver == D3D_VERSION::V11)
        {
            ID3D11ShaderReflection* pReflector;
            HRESULT hr = D3DReflect(data.data(), data.size(), IID_PPV_ARGS(&pReflector));
            D3D11_SHADER_DESC _desc;
            pReflector->GetDesc(&_desc);
            desc = D3D::D3D_SHADER_DESC(_desc);
            interface_slots = pReflector->GetNumInterfaceSlots();
            //  inputs.resize(desc.InputParameters);

            for (unsigned int i = 0; i < desc.InputParameters; i++)
            {
                D3D11_SIGNATURE_PARAMETER_DESC sigDesc;
                pReflector->GetInputParameterDesc(i, &sigDesc);
                inputs.emplace_back(sigDesc);
            }

            for (unsigned int i = 0; i < desc.ConstantBuffers; i++)
            {
                D3D11_SHADER_BUFFER_DESC  desc;
                auto buffer = pReflector->GetConstantBufferByIndex(i);
                buffer->GetDesc(&desc);
                const_buffers.emplace_back(desc);

                for (unsigned int i = 0; i < const_buffers.back().Variables; i++)
                {
                    auto var = buffer->GetVariableByIndex(i);
                    D3D11_SHADER_VARIABLE_DESC desc;
                    var->GetDesc(&desc);
                    const_buffers.back().variables.emplace_back(desc);
                    // TODO!
                    const_buffers.back().variables.back().interface_slots.emplace_back(var->GetInterfaceSlot(0));
                }
            }

            for (unsigned int i = 0; i < desc.BoundResources; i++)
            {
                D3D11_SHADER_INPUT_BIND_DESC BindDesc;
                pReflector->GetResourceBindingDesc(i, &BindDesc);
                input_bind.emplace_back(BindDesc);
            }
        }

        if (ver == D3D_VERSION::V12)
        {
            ID3D12ShaderReflection* pReflector;
            HRESULT hr = D3DReflect(data.data(), data.size(), IID_PPV_ARGS(&pReflector));
            D3D12_SHADER_DESC _desc;
            pReflector->GetDesc(&_desc);
            desc = D3D::D3D_SHADER_DESC(_desc);
            interface_slots = pReflector->GetNumInterfaceSlots();
            //  inputs.resize(desc.InputParameters);

            for (unsigned int i = 0; i < desc.InputParameters; i++)
            {
                D3D12_SIGNATURE_PARAMETER_DESC sigDesc;
                pReflector->GetInputParameterDesc(i, &sigDesc);
                inputs.emplace_back(sigDesc);
            }

            for (unsigned int i = 0; i < desc.ConstantBuffers; i++)
            {
                D3D12_SHADER_BUFFER_DESC  desc;
                auto buffer = pReflector->GetConstantBufferByIndex(i);
                buffer->GetDesc(&desc);
                const_buffers.emplace_back(desc);

                for (unsigned int i = 0; i < const_buffers.back().Variables; i++)
                {
                    auto var = buffer->GetVariableByIndex(i);
                    D3D12_SHADER_VARIABLE_DESC desc;
                    var->GetDesc(&desc);
                    const_buffers.back().variables.emplace_back(desc);
                    // TODO!
                    const_buffers.back().variables.back().interface_slots.emplace_back(var->GetInterfaceSlot(0));
                }
            }

            for (unsigned int i = 0; i < desc.BoundResources; i++)
            {
                D3D12_SHADER_INPUT_BIND_DESC BindDesc;
                pReflector->GetResourceBindingDesc(i, &BindDesc);
                input_bind.emplace_back(BindDesc);
            }
        }

#endif
    }
}