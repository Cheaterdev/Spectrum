export module HAL:Descriptors;

import d3d12;

import :Types;
import :Format;

export namespace HAL
{
	// TODO: probably shoud not be used at all -> use hlsl direct mappings instead
	namespace Views
	{
		struct DepthStencil
		{
			struct Texture1D
			{
				uint MipSlice;
			} 	TEX1D_DSV;

			struct Texture1DArray
			{
				uint MipSlice;
				uint FirstArraySlice;
				uint ArraySize;
			};

			struct Texture2D
			{
				uint MipSlice;
			};

			struct Texture2DArray
			{
				uint MipSlice;
				uint FirstArraySlice;
				uint ArraySize;
			};

			struct Texture2DMS
			{
				uint UnusedField_NothingToDefine;
			};

			struct Texture2DMSArray
			{
				uint FirstArraySlice;
				uint ArraySize;
			};

			enum class Flags : uint
			{
				None = 0,
				ReadDepth = 0x1,
				Readstencil = 0x2
			};

			Format Format;
			Flags Flags;

			std::variant<Texture1D, Texture1DArray, Texture2D, Texture2DArray, Texture2DMS, Texture2DMSArray> View;
		};

		struct ShaderResource
		{
			struct Buffer
			{
				uint64 FirstElement;
				uint NumElements;
				uint StructureByteStride;
				bool raw;
			};

			struct Texture1D
			{
				uint MostDetailedMip;
				uint MipLevels;
				float ResourceMinLODClamp;
			};

			struct Texture1DArray
			{
				uint MostDetailedMip;
				uint MipLevels;
				uint FirstArraySlice;
				uint ArraySize;
				float ResourceMinLODClamp;
			};

			struct Texture2D
			{
				uint MostDetailedMip;
				uint MipLevels;
				uint PlaneSlice;
				float ResourceMinLODClamp;
			};

			struct Texture2DArray
			{
				uint MostDetailedMip;
				uint MipLevels;
				uint FirstArraySlice;
				uint ArraySize;
				uint PlaneSlice;
				float ResourceMinLODClamp;
			};

			struct Texture3D
			{
				uint MostDetailedMip;
				uint MipLevels;
				float ResourceMinLODClamp;
			};

			struct Cube
			{
				uint MostDetailedMip;
				uint MipLevels;
				float ResourceMinLODClamp;
			};

			struct CubeArray
			{
				uint MostDetailedMip;
				uint MipLevels;
				uint First2DArrayFace;
				uint NumCubes;
				float ResourceMinLODClamp;
			};

			struct Texture2DMS
			{
				uint UnusedField_NothingToDefine;
			};

			struct Texture2DMSArray
			{
				uint FirstArraySlice;
				uint ArraySize;
			};

			struct Raytracing
			{
				GPUAddressPtr Location;
			};

			Format Format;
			uint Shader4ComponentMapping;
			std::variant<Buffer, Texture1D, Texture1DArray, Texture2D, Texture2DArray, Texture3D, Texture2DMS, Texture2DMSArray, Cube, CubeArray, Raytracing> View;
		};

		struct RenderTarget
		{

			struct Buffer
			{
				uint64 FirstElement;
				uint NumElements;
			};

			struct Texture1D
			{
				uint MipSlice;
			};

			struct Texture1DArray
			{
				uint MipSlice;
				uint FirstArraySlice;
				uint ArraySize;
			};

			struct Texture2D
			{
				uint MipSlice;
				uint PlaneSlice;
			};

			struct Texture2DMS
			{
				uint UnusedField_NothingToDefine;
			};

			struct Texture2DArray
			{
				uint MipSlice;
				uint FirstArraySlice;
				uint ArraySize;
				uint PlaneSlice;
			};

			struct Texture2DMSArray
			{
				uint FirstArraySlice;
				uint ArraySize;
			};

			struct Texture3D
			{
				uint MipSlice;
				uint FirstWSlice;
				uint WSize;
			};

			Format Format;
			std::variant < Buffer, Texture1D, Texture1DArray, Texture2D, Texture2DArray, Texture2DMS, Texture2DMSArray, Texture3D > View;
		};

		struct ResourceAddress
		{
			GPUAddressPtr address;
		};

		struct ConstantBuffer
		{
			GPUAddressPtr BufferLocation;
			uint SizeInBytes;
		};


		struct UnorderedAccess
		{
			struct Texture1D
			{
				uint MipSlice;
			};

			struct Texture1DArray
			{
				uint MipSlice;
				uint FirstArraySlice;
				uint ArraySize;
			};

			struct Texture2D
			{
				uint MipSlice;
				uint PlaneSlice;
			};

			struct Texture2DArray
			{
				uint MipSlice;
				uint FirstArraySlice;
				uint ArraySize;
				uint PlaneSlice;
			};

			struct Texture3D
			{
				uint MipSlice;
				uint FirstWSlice;
				uint WSize;
			};

			Format Format;
			std::variant<Texture1D, Texture1DArray, Texture2D, Texture2DArray,  Texture3D> View;
		};
	}

}/*

				DEPTH_STENCIL_VIEW_DESC dsv;

				struct
				{
					Resource* counter;
					UNORDERED_ACCESS_VIEW_DESC desc;
				} uav;

				SHADER_RESOURCE_VIEW_DESC srv;
				RENDER_TARGET_VIEW_DESC rtv;
				CONSTANT_BUFFER_VIEW_DESC cbv;*/