#pragma once
#include "Shaders.h"
namespace D3D
{
    struct D3D_SHADER_DESC
    {
            UINT                    Version;                     // Shader version
            std::string             Creator;                     // Creator string
            UINT                    Flags;                       // Shader compilation/parse flags

            UINT                    ConstantBuffers;             // Number of constant buffers
            UINT                    BoundResources;              // Number of bound resources
            UINT                    InputParameters;             // Number of parameters in the input signature
            UINT                    OutputParameters;            // Number of parameters in the output signature

            UINT                    InstructionCount;            // Number of emitted instructions
            UINT                    TempRegisterCount;           // Number of temporary registers used
            UINT                    TempArrayCount;              // Number of temporary arrays used
            UINT                    DefCount;                    // Number of constant defines
            UINT                    DclCount;                    // Number of declarations (input + output)
            UINT                    TextureNormalInstructions;   // Number of non-categorized texture instructions
            UINT                    TextureLoadInstructions;     // Number of texture load instructions
            UINT                    TextureCompInstructions;     // Number of texture comparison instructions
            UINT                    TextureBiasInstructions;     // Number of texture bias instructions
            UINT                    TextureGradientInstructions; // Number of texture gradient instructions
            UINT                    FloatInstructionCount;       // Number of floating point arithmetic instructions used
            UINT                    IntInstructionCount;         // Number of signed integer arithmetic instructions used
            UINT                    UintInstructionCount;        // Number of unsigned integer arithmetic instructions used
            UINT                    StaticFlowControlCount;      // Number of static flow control instructions used
            UINT                    DynamicFlowControlCount;     // Number of dynamic flow control instructions used
            UINT                    MacroInstructionCount;       // Number of macro instructions used
            UINT                    ArrayInstructionCount;       // Number of array instructions used
            UINT                    CutInstructionCount;         // Number of cut instructions used
            UINT                    EmitInstructionCount;        // Number of emit instructions used
            D3D_PRIMITIVE_TOPOLOGY  GSOutputTopology;            // Geometry shader output topology
            UINT                    GSMaxOutputVertexCount;      // Geometry shader maximum output vertex count
            D3D_PRIMITIVE           InputPrimitive;              // GS/HS input primitive
            UINT                    PatchConstantParameters;     // Number of parameters in the patch constant signature
            UINT                    cGSInstanceCount;            // Number of Geometry shader instances
            UINT                    cControlPoints;              // Number of control points in the HS->DS stage
            D3D_TESSELLATOR_OUTPUT_PRIMITIVE HSOutputPrimitive;  // Primitive output by the tessellator
            D3D_TESSELLATOR_PARTITIONING HSPartitioning;         // Partitioning mode of the tessellator
            D3D_TESSELLATOR_DOMAIN  TessellatorDomain;           // Domain of the tessellator (quad, tri, isoline)
            // instruction counts
            UINT cBarrierInstructions;                           // Number of barrier instructions in a compute shader
            UINT cInterlockedInstructions;                       // Number of interlocked instructions
            UINT cTextureStoreInstructions;                      // Number of texture writes

            D3D_SHADER_DESC() = default;

            D3D_SHADER_DESC(const D3D12_SHADER_DESC&);

        private:
            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                ar& NVP(ArrayInstructionCount);
                ar& NVP(BoundResources);
                ar& NVP(cBarrierInstructions);
                ar& NVP(cControlPoints);
                ar& NVP(cGSInstanceCount);
                ar& NVP(cInterlockedInstructions);
                ar& NVP(ConstantBuffers);
                ar& NVP(Creator);
                ar& NVP(cTextureStoreInstructions);
                ar& NVP(CutInstructionCount);
                ar& NVP(DclCount);
                ar& NVP(DefCount);
                ar& NVP(DynamicFlowControlCount);
                ar& NVP(EmitInstructionCount);
                ar& NVP(Flags);
                ar& NVP(FloatInstructionCount);
                ar& NVP(GSMaxOutputVertexCount);
                ar& NVP(GSOutputTopology);
                ar& NVP(HSOutputPrimitive);
                ar& NVP(HSPartitioning);
                ar& NVP(InputParameters);
                ar& NVP(InputPrimitive);
                ar& NVP(InstructionCount);
                ar& NVP(IntInstructionCount);
                ar& NVP(MacroInstructionCount);
                ar& NVP(OutputParameters);
                ar& NVP(PatchConstantParameters);
                ar& NVP(TempArrayCount);
                ar& NVP(TempRegisterCount);
                ar& NVP(TessellatorDomain);
                ar& NVP(TextureBiasInstructions);
                ar& NVP(TextureCompInstructions);
                ar& NVP(TextureGradientInstructions);
                ar& NVP(TextureLoadInstructions);
                ar& NVP(TextureNormalInstructions);
                ar& NVP(UintInstructionCount);
                ar& NVP(Version);
            }


    };


    struct D3D_SHADER_VARIABLE_DESC
    {
            std::string             Name;           // Name of the variable
            UINT                    StartOffset;    // Offset in constant buffer's backing store
            UINT                    Size;           // Size of variable (in bytes)
            UINT                    uFlags;         // Variable flags
            LPVOID                  DefaultValue;   // Raw pointer to default value
            UINT                    StartTexture;   // First texture index (or -1 if no textures used)
            UINT                    TextureSize;    // Number of texture slots possibly used.
            UINT                    StartSampler;   // First sampler index (or -1 if no textures used)
            UINT                    SamplerSize;    // Number of sampler slots possibly used.
            std::vector<UINT> interface_slots;

            D3D_SHADER_VARIABLE_DESC() = default;

            D3D_SHADER_VARIABLE_DESC(const D3D12_SHADER_VARIABLE_DESC&);

        private:
            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                //	ar & g.DefaultValue;
                ar& NVP(SamplerSize);
                ar& NVP(Size);
                ar& NVP(StartOffset);
                ar& NVP(StartSampler);
                ar& NVP(StartTexture);
                ar& NVP(Name);
                ar& NVP(TextureSize);
                ar& NVP(uFlags);
                ar& NVP(interface_slots);
            }
    };




    struct D3D_SHADER_INPUT_BIND_DESC
    {
            std::string                 Name;           // Name of the resource
            D3D_SHADER_INPUT_TYPE       Type;           // Type of resource (e.g. texture, cbuffer, etc.)
            UINT                        BindPoint;      // Starting bind point
            UINT                        BindCount;      // Number of contiguous bind points (for arrays)

            UINT                        uFlags;         // Input binding flags
            D3D_RESOURCE_RETURN_TYPE    ReturnType;     // Return type (if texture)
            D3D_SRV_DIMENSION           Dimension;      // Dimension (if texture)
            UINT                        NumSamples;     // Number of samples (0 if not MS texture)
            UINT                        Space;          // Register space
            UINT uID;                                   // Range ID in the bytecode


            D3D_SHADER_INPUT_BIND_DESC() = default;

            D3D_SHADER_INPUT_BIND_DESC(const D3D12_SHADER_INPUT_BIND_DESC&);

        private:
            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                ar& NVP(Name);
                ar& NVP(Type);
                ar& NVP(BindPoint);
                ar& NVP(BindCount);
                ar& NVP(uFlags);
                ar& NVP(ReturnType);
                ar& NVP(Dimension);
                ar& NVP(NumSamples);
                ar& NVP(Space);
            }
    };


    struct D3D_SHADER_BUFFER_DESC
    {
            std::string             Name;           // Name of the constant buffer
            D3D_CBUFFER_TYPE        Type;           // Indicates type of buffer content
            UINT                    Variables;      // Number of member variables
            UINT                    Size;           // Size of CB (in bytes)
            UINT                    uFlags;         // Buffer description flags
            std::vector<D3D_SHADER_VARIABLE_DESC> variables;

            D3D_SHADER_BUFFER_DESC() = default;

            D3D_SHADER_BUFFER_DESC(const D3D12_SHADER_BUFFER_DESC&);


        private:
            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                ar& NVP(Name);
                ar& NVP(Type);
                ar& NVP(Variables);
                ar& NVP(Size);
                ar& NVP(uFlags);
                ar& NVP(variables);
            }
    };


    struct D3D_SIGNATURE_PARAMETER_DESC
    {
            std::string                 SemanticName;   // Name of the semantic
            UINT                        SemanticIndex;  // Index of the semantic
            UINT                        Register;       // Number of member variables
            D3D_NAME                    SystemValueType;// A predefined system value, or D3D_NAME_UNDEFINED if not applicable
            D3D_REGISTER_COMPONENT_TYPE ComponentType;  // Scalar type (e.g. uint, float, etc.)
            BYTE                        Mask;           // Mask to indicate which components of the register
            // are used (combination of D3D10_COMPONENT_MASK values)
            BYTE                        ReadWriteMask;  // Mask to indicate whether a given component is
            // never written (if this is an output signature) or
            // always read (if this is an input signature).
            // (combination of D3D_MASK_* values)
            UINT                        Stream;         // Stream index
            D3D_MIN_PRECISION           MinPrecision;   // Minimum desired interpolation precision

            D3D_SIGNATURE_PARAMETER_DESC() = default;

    //        D3D_SIGNATURE_PARAMETER_DESC(const D3D11_SIGNATURE_PARAMETER_DESC&);
            D3D_SIGNATURE_PARAMETER_DESC(const D3D12_SIGNATURE_PARAMETER_DESC&);

        private:
            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                ar& NVP(SemanticName);
                ar& NVP(SemanticIndex);
                ar& NVP(Register);
                ar& NVP(SystemValueType);
                ar& NVP(ComponentType);
                ar& NVP(Mask);
                ar& NVP(ReadWriteMask);
                ar& NVP(Stream);
                ar& NVP(MinPrecision);
            }


    } ;



    class reflection
    {

            D3D_SHADER_DESC desc;
            UINT interface_slots;
            std::vector<D3D_SIGNATURE_PARAMETER_DESC> inputs;
            std::vector<D3D_SHADER_BUFFER_DESC> const_buffers;
            std::vector<D3D_SHADER_INPUT_BIND_DESC> input_bind;
            //	ShaderWrapper t;


        public:

            UINT get_interface_slots_count();

            std::vector<D3D_SIGNATURE_PARAMETER_DESC>& get_inputs()
            {
                return inputs;
            }

            /*
            D3D12_SIGNATURE_PARAMETER_DESC &GetInputParameterDesc(int i);
            ShaderReflectionConstantBuffer &GetShaderReflectionConstantBuffer(int i);
            ShaderReflectionInputBindDesc &GetInputBindDesc(int i);
            D3D12_SHADER_INPUT_BIND_DESC* GetInputBindDescByName(string name);
            ShaderReflectionVariable* GetVariableByName(string name);


            CShaderDesc GetDesc();*/
            reflection() = default;

            reflection(D3D_VERSION ver, std::string data);

            reflection(ComPtr<ID3D12ShaderReflection> refl);

        private:

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                ar& NVP(desc);
                ar& NVP(interface_slots);
                ar& NVP(inputs);
                ar& NVP(const_buffers);
                ar& NVP(input_bind);
            }
    };




}
