module HAL:NRD;

import Core;
import nrd;
import :Utils;
import :Device;
import :RootSignature;
import :Types;
import :Sampler;
import :ShaderCompiler;
import d3d12;
import wrl;

namespace nvidia
{
	// PipelineDesc::shaderIdentifier is "fileName|macro1=value1|macro2=value2..."
	// (NRDDescs.h's own documented format for exactly this use case). Splits it
	// into the vendored source file to compile (workdir/shaders/nrd/<fileName>)
	// and the permutation defines NRD's own build used for this pipeline.
	static std::vector<HAL::shader_macro> parse_nrd_shader_identifier(const std::string& identifier, std::string& out_filename)
	{
		size_t bar = identifier.find('|');
		out_filename = identifier.substr(0, bar);

		std::vector<HAL::shader_macro> macros;
		size_t pos = bar;
		while (pos != std::string::npos)
		{
			size_t next = identifier.find('|', pos + 1);
			std::string token = identifier.substr(pos + 1, next == std::string::npos ? std::string::npos : next - pos - 1);
			pos = next;

			if (token.empty()) continue;

			size_t eq = token.find('=');
			if (eq != std::string::npos)
				macros.emplace_back(token.substr(0, eq), token.substr(eq + 1));
			else
				macros.emplace_back(token, "1");
		}
		return macros;
	}

	// nrd::Format's names abbreviate the RGBA components NRD-side
	// (e.g. RGBA16_SFLOAT); HAL::Format spells every component out
	// DXGI-style (R16G16B16A16_FLOAT). Mechanical 1:1 mapping, no format
	// this engine doesn't already have a DXGI equivalent for.
	static HAL::Format to_hal_format(nrd::Format f)
	{
		switch (f)
		{
		case nrd::Format::R8_UNORM: return HAL::Format::R8_UNORM;
		case nrd::Format::R8_SNORM: return HAL::Format::R8_SNORM;
		case nrd::Format::R8_UINT: return HAL::Format::R8_UINT;
		case nrd::Format::R8_SINT: return HAL::Format::R8_SINT;
		case nrd::Format::RG8_UNORM: return HAL::Format::R8G8_UNORM;
		case nrd::Format::RG8_SNORM: return HAL::Format::R8G8_SNORM;
		case nrd::Format::RG8_UINT: return HAL::Format::R8G8_UINT;
		case nrd::Format::RG8_SINT: return HAL::Format::R8G8_SINT;
		case nrd::Format::RGBA8_UNORM: return HAL::Format::R8G8B8A8_UNORM;
		case nrd::Format::RGBA8_SNORM: return HAL::Format::R8G8B8A8_SNORM;
		case nrd::Format::RGBA8_UINT: return HAL::Format::R8G8B8A8_UINT;
		case nrd::Format::RGBA8_SINT: return HAL::Format::R8G8B8A8_SINT;
		case nrd::Format::RGBA8_SRGB: return HAL::Format::R8G8B8A8_UNORM_SRGB;
		case nrd::Format::R16_UNORM: return HAL::Format::R16_UNORM;
		case nrd::Format::R16_SNORM: return HAL::Format::R16_SNORM;
		case nrd::Format::R16_UINT: return HAL::Format::R16_UINT;
		case nrd::Format::R16_SINT: return HAL::Format::R16_SINT;
		case nrd::Format::R16_SFLOAT: return HAL::Format::R16_FLOAT;
		case nrd::Format::RG16_UNORM: return HAL::Format::R16G16_UNORM;
		case nrd::Format::RG16_SNORM: return HAL::Format::R16G16_SNORM;
		case nrd::Format::RG16_UINT: return HAL::Format::R16G16_UINT;
		case nrd::Format::RG16_SINT: return HAL::Format::R16G16_SINT;
		case nrd::Format::RG16_SFLOAT: return HAL::Format::R16G16_FLOAT;
		case nrd::Format::RGBA16_UNORM: return HAL::Format::R16G16B16A16_UNORM;
		case nrd::Format::RGBA16_SNORM: return HAL::Format::R16G16B16A16_SNORM;
		case nrd::Format::RGBA16_UINT: return HAL::Format::R16G16B16A16_UINT;
		case nrd::Format::RGBA16_SINT: return HAL::Format::R16G16B16A16_SINT;
		case nrd::Format::RGBA16_SFLOAT: return HAL::Format::R16G16B16A16_FLOAT;
		case nrd::Format::R32_UINT: return HAL::Format::R32_UINT;
		case nrd::Format::R32_SINT: return HAL::Format::R32_SINT;
		case nrd::Format::R32_SFLOAT: return HAL::Format::R32_FLOAT;
		case nrd::Format::RG32_UINT: return HAL::Format::R32G32_UINT;
		case nrd::Format::RG32_SINT: return HAL::Format::R32G32_SINT;
		case nrd::Format::RG32_SFLOAT: return HAL::Format::R32G32_FLOAT;
		case nrd::Format::RGB32_UINT: return HAL::Format::R32G32B32_UINT;
		case nrd::Format::RGB32_SINT: return HAL::Format::R32G32B32_SINT;
		case nrd::Format::RGB32_SFLOAT: return HAL::Format::R32G32B32_FLOAT;
		case nrd::Format::RGBA32_UINT: return HAL::Format::R32G32B32A32_UINT;
		case nrd::Format::RGBA32_SINT: return HAL::Format::R32G32B32A32_SINT;
		case nrd::Format::RGBA32_SFLOAT: return HAL::Format::R32G32B32A32_FLOAT;
		case nrd::Format::R10_G10_B10_A2_UNORM: return HAL::Format::R10G10B10A2_UNORM;
		case nrd::Format::R10_G10_B10_A2_UINT: return HAL::Format::R10G10B10A2_UINT;
		case nrd::Format::R11_G11_B10_UFLOAT: return HAL::Format::R11G11B10_FLOAT;
		case nrd::Format::R9_G9_B9_E5_UFLOAT: return HAL::Format::R9G9B9E5_SHAREDEXP;
		default: return HAL::Format::R8G8B8A8_UNORM;
		}
	}

	NRD::NRD()
	{
		const nrd::LibraryDesc& lib = *nrd::GetLibraryDesc();
		Log::get() << "[NRD] library v" << (int)lib.versionMajor << "." << (int)lib.versionMinor
		           << "." << (int)lib.versionBuild << ", " << lib.supportedDenoisersNum
		           << " denoisers supported" << Log::endl;

		// SIGMA_SHADOW for RTXShadowNoise (ShadowRTX, voxel.sig). REBLUR_DIFFUSE
		// for indirect GI (RTXIndirectNoise, IndirectRTX, voxel.sig) -- diffuse-
		// only variant since specular/reflections go through a separate
		// REBLUR_SPECULAR instance later, not combined here.
		static const nrd::DenoiserDesc denoisers[] = {
			{ 0, nrd::Denoiser::SIGMA_SHADOW },
			{ 1, nrd::Denoiser::REBLUR_DIFFUSE }
		};

		nrd::InstanceCreationDesc desc{};
		desc.denoisers = denoisers;
		desc.denoisersNum = 2;

		const nrd::Result res = nrd::CreateInstance(desc, instance);
		if (res != nrd::Result::SUCCESS)
		{
			Log::get() << "[NRD] CreateInstance failed (" << (int)res << ")" << Log::endl;
			return;
		}

		resolved = true;
		Log::get() << "[NRD] instance created" << Log::endl;
	}

	NRD::~NRD()
	{
		if (instance)
			nrd::DestroyInstance(*instance);
	}

	void NRD::smoke_test() const
	{
		if (!resolved) return;

		const nrd::InstanceDesc& idesc = *nrd::GetInstanceDesc(*instance);
		Log::get() << "[NRD] instance desc: " << idesc.pipelinesNum << " pipelines, "
		           << idesc.permanentPoolSize << " permanent pool textures, "
		           << idesc.transientPoolSize << " transient pool textures, "
		           << idesc.constantBufferMaxDataSize << " max CB bytes" << Log::endl;
	}

	void NRD::create_pipelines(HAL::Device& device)
	{
		if (!resolved) return;

		const nrd::InstanceDesc& idesc = *nrd::GetInstanceDesc(*instance);

		// Shared pipeline layout, per InstanceDesc's own recommendation: a
		// root CBV for constants, static NEAREST/LINEAR_CLAMP samplers, and
		// one SRV + one UAV descriptor table sized to the largest single
		// dispatch (perSetTexturesMaxNum/perSetStorageTexturesMaxNum) rather
		// than tight per-pipeline layouts.
		HAL::RootSignatureDesc rs_desc;
		rs_desc[0] = HAL::DescriptorConstBuffer(idesc.constantBufferRegisterIndex,
			HAL::ShaderVisibility::ALL, idesc.constantBufferAndSamplersSpaceIndex);
		rs_desc[1] = HAL::DescriptorTable(HAL::DescriptorRange::SRV, HAL::ShaderVisibility::ALL,
			idesc.resourcesBaseRegisterIndex, idesc.descriptorPoolDesc.perSetTexturesMaxNum, idesc.resourcesSpaceIndex);
		rs_desc[2] = HAL::DescriptorTable(HAL::DescriptorRange::UAV, HAL::ShaderVisibility::ALL,
			idesc.resourcesBaseRegisterIndex, idesc.descriptorPoolDesc.perSetStorageTexturesMaxNum, idesc.resourcesSpaceIndex);

		for (uint32_t i = 0; i < idesc.samplersNum; ++i)
		{
			const HAL::SamplerDesc& sampler = (idesc.samplers[i] == nrd::Sampler::NEAREST_CLAMP)
				? HAL::Samplers::SamplerPointClampDesc
				: HAL::Samplers::SamplerLinearClampDesc;
			rs_desc.set_sampler(idesc.samplersBaseRegisterIndex + i, idesc.constantBufferAndSamplersSpaceIndex,
				HAL::ShaderVisibility::ALL, sampler);
		}

		root_signature = std::make_shared<HAL::RootSignature>(device, rs_desc);

		pipelines.clear();
		pipelines.reserve(idesc.pipelinesNum);

		// NRD's own embedded DXIL (p.computeShaderDXIL) is NOT used here --
		// instead we recompile NRD's real, vendored shader source
		// (workdir/shaders/nrd/, see custom-overlay/nrd's usage notes) through
		// this engine's own DXC pipeline. This works unmodified because
		// NRD.hlsli's DXC branch emits plain register(tN/uN/sN/bN, spaceN)
		// bindings using NRD_CONSTANT_BUFFER_REGISTER_INDEX/
		// NRD_RESOURCES_SPACE_INDEX/NRD_CONSTANT_BUFFER_AND_SAMPLERS_SPACE_INDEX
		// -- the exact same values InstanceDesc reports and that the root
		// signature above was built from -- so the root signature is
		// identical whether the bytecode comes from nrd.lib or from here.
		uint32_t created = 0;
		for (uint32_t i = 0; i < idesc.pipelinesNum; ++i)
		{
			const nrd::PipelineDesc& p = idesc.pipelines[i];

			std::string filename;
			std::vector<HAL::shader_macro> macros = parse_nrd_shader_identifier(p.shaderIdentifier, filename);

			resource_file_depender depender;
			HAL::shader_include includer("shaders/nrd/", depender);

			auto compiled = HAL::ShaderCompiler::get().Compile_Shader_File(filename, macros, "cs_6_8", "main", HAL::ShaderOptions::None, &includer);
			if (!compiled)
			{
				Log::get() << "[NRD] pipeline " << i << " (" << p.shaderIdentifier << ") shader compile failed" << Log::endl;
				pipelines.push_back(nullptr);
				continue;
			}

			D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc{};
			pso_desc.pRootSignature = root_signature->get_native().Get();
			pso_desc.CS.pShaderBytecode = compiled->blob.data();
			pso_desc.CS.BytecodeLength = compiled->blob.size();

			D3D::PipelineState pso;
			HRESULT hr = device.get_native_device()->CreateComputePipelineState(&pso_desc, IID_PPV_ARGS(&pso));
			if (hr != S_OK)
			{
				Log::get() << "[NRD] pipeline " << i << " (" << p.shaderIdentifier << ") CreateComputePipelineState failed" << Log::endl;
				pipelines.push_back(nullptr);
				continue;
			}

			pipelines.push_back(pso);
			++created;
		}

		Log::get() << "[NRD] " << created << "/" << idesc.pipelinesNum << " pipelines created" << Log::endl;
	}

	void NRD::ensure_pools(HAL::Device& device, uint2 render_size)
	{
		if (!resolved) return;
		if (pools_render_size == render_size && pools_ready()) return;

		const nrd::InstanceDesc& idesc = *nrd::GetInstanceDesc(*instance);

		auto make_pool = [&](const nrd::TextureDesc* descs, uint32_t count, std::vector<HAL::TextureResource::ptr>& out)
		{
			out.clear();
			out.reserve(count);
			for (uint32_t i = 0; i < count; ++i)
			{
				const nrd::TextureDesc& td = descs[i];

				// downsampleFactor is a linear divisor (e.g. 16 for
				// SIGMA_ClassifyTiles' 16x16 tile-classification output, not
				// a bit-shift exponent), and ceiling-divided -- confirmed
				// against NVIDIA's own reference integration,
				// NRDIntegration.hpp: "DivideUp(m_Desc.resourceWidth,
				// nrdTextureDesc.downsampleFactor)".
				uint2 size(
					(render_size.x + td.downsampleFactor - 1) / td.downsampleFactor,
					(render_size.y + td.downsampleFactor - 1) / td.downsampleFactor);

				out.push_back(std::make_shared<HAL::TextureResource>(device,
					HAL::ResourceDesc::Tex2D(to_hal_format(td.format), size, 1, 1,
						HAL::ResFlags::UnorderedAccess | HAL::ResFlags::ShaderResource),
					HAL::HeapType::DEFAULT));
			}
		};

		make_pool(idesc.permanentPool, idesc.permanentPoolSize, permanent_pool);
		make_pool(idesc.transientPool, idesc.transientPoolSize, transient_pool);

		pools_render_size = render_size;

		Log::get() << "[NRD] pools allocated: " << permanent_pool.size() << " permanent, "
		           << transient_pool.size() << " transient, at " << render_size.x << "x" << render_size.y << Log::endl;
	}
}
