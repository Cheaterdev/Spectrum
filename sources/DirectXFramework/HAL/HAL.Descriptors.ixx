export module HAL:Descriptors;

import d3d12;

import :Types;
import :Format;
import :Resource;

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
				None,
				ReadOnlyDepth,
				ReadOnlyStencil,

				GENERATE_OPS
			};

			Resource* Resource;
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
				bool Raw;
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

			Resource* Resource;
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

			Resource* Resource;
			Format Format;
			std::variant < Buffer, Texture1D, Texture1DArray, Texture2D, Texture2DArray, Texture2DMS, Texture2DMSArray, Texture3D > View;
		};

		struct ConstantBuffer
		{
			Resource* Resource;
			uint OffsetInBytes;
			uint SizeInBytes;
		};


		struct UnorderedAccess
		{
			struct Buffer
			{
				uint64 FirstElement;
				uint NumElements;
				uint StructureByteStride;
				uint64 CounterOffsetInBytes;
				bool Raw;
				Resource* CounterResource;
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

			Resource* Resource;
			Format Format;
			std::variant<Texture1D, Texture1DArray, Texture2D, Texture2DArray, Texture3D> View;
		};
	}

}