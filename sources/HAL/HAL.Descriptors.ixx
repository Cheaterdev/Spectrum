export module HAL:Descriptors;

import :Types;
import :Format;
import :API.Resource;

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

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
				}
			};

			struct Texture1DArray
			{
				uint MipSlice;
				uint FirstArraySlice;
				uint ArraySize;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
					ar& NVP(FirstArraySlice);
					ar& NVP(ArraySize);
				}
			};

			struct Texture2D
			{
				uint MipSlice;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
				}
			};

			struct Texture2DArray
			{
				uint MipSlice;
				uint FirstArraySlice;
				uint ArraySize;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
					ar& NVP(FirstArraySlice);
					ar& NVP(ArraySize);
				}
			};

			struct Texture2DMS
			{
				uint UnusedField_NothingToDefine;

			private:
				SERIALIZE()
				{

				}
			};

			struct Texture2DMSArray
			{
				uint FirstArraySlice;
				uint ArraySize;

			private:
				SERIALIZE()
				{
					ar& NVP(FirstArraySlice);
					ar& NVP(ArraySize);
				}
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

		private:
			SERIALIZE()
			{
				ar& NVP(Resource);
				ar& NVP(Format);
				ar& NVP(Flags);
				ar& NVP(View);
			}
		};

		struct ShaderResource
		{
			struct Buffer
			{
				uint64 FirstElement;
				uint NumElements;
				uint StructureByteStride;
				bool Raw;

			private:
				SERIALIZE()
				{
					ar& NVP(FirstElement);
					ar& NVP(NumElements);
					ar& NVP(StructureByteStride);
					ar& NVP(Raw);
				}
			};

			struct Texture1D
			{
				uint MostDetailedMip;
				uint MipLevels;
				float ResourceMinLODClamp;

			private:
				SERIALIZE()
				{
					ar& NVP(MostDetailedMip);
					ar& NVP(MipLevels);
					ar& NVP(ResourceMinLODClamp);
				}
			};

			struct Texture1DArray
			{
				uint MostDetailedMip;
				uint MipLevels;
				uint FirstArraySlice;
				uint ArraySize;
				float ResourceMinLODClamp;

			private:
				SERIALIZE()
				{
					ar& NVP(MostDetailedMip);
					ar& NVP(MipLevels);
					ar& NVP(FirstArraySlice);
					ar& NVP(ArraySize);
					ar& NVP(ResourceMinLODClamp);
				}
			};

			struct Texture2D
			{
				uint MostDetailedMip;
				uint MipLevels;
				uint PlaneSlice;
				float ResourceMinLODClamp;

			private:
				SERIALIZE()
				{
					ar& NVP(MostDetailedMip);
					ar& NVP(MipLevels);
					ar& NVP(ResourceMinLODClamp);
				}
			};

			struct Texture2DArray
			{
				uint MostDetailedMip;
				uint MipLevels;
				uint FirstArraySlice;
				uint ArraySize;
				uint PlaneSlice;
				float ResourceMinLODClamp;

			private:
				SERIALIZE()
				{
					ar& NVP(MostDetailedMip);
					ar& NVP(MipLevels);
					ar& NVP(FirstArraySlice);
					ar& NVP(ArraySize);
					ar& NVP(PlaneSlice);
					ar& NVP(ResourceMinLODClamp);
				}
			};

			struct Texture3D
			{
				uint MostDetailedMip;
				uint MipLevels;
				float ResourceMinLODClamp;

			private:
				SERIALIZE()
				{
					ar& NVP(MostDetailedMip);
					ar& NVP(MipLevels);
					ar& NVP(ResourceMinLODClamp);
				}
			};

			struct Cube
			{
				uint MostDetailedMip;
				uint MipLevels;
				float ResourceMinLODClamp;

			private:
				SERIALIZE()
				{
					ar& NVP(MostDetailedMip);
					ar& NVP(MipLevels);
					ar& NVP(ResourceMinLODClamp);
				}
			};

			struct CubeArray
			{
				uint MostDetailedMip;
				uint MipLevels;
				uint First2DArrayFace;
				uint NumCubes;
				float ResourceMinLODClamp;

			private:
				SERIALIZE()
				{
					ar& NVP(MostDetailedMip);
					ar& NVP(MipLevels);
					ar& NVP(First2DArrayFace);
					ar& NVP(NumCubes);
					ar& NVP(ResourceMinLODClamp);
				}
			};

			struct Texture2DMS
			{
				uint UnusedField_NothingToDefine;

			private:
				SERIALIZE()
				{
				}
			};

			struct Texture2DMSArray
			{
				uint FirstArraySlice;
				uint ArraySize;

			private:
				SERIALIZE()
				{
					ar& NVP(FirstArraySlice);
					ar& NVP(ArraySize);
				}
			};

			struct Raytracing
			{
				uint OffsetInBytes;

			private:
				SERIALIZE()
				{
					ar& NVP(OffsetInBytes);
				}
			};

			Resource* Resource;
			Format Format;
			//uint Shader4ComponentMapping;
			std::variant<Buffer, Texture1D, Texture1DArray, Texture2D, Texture2DArray, Texture3D, Texture2DMS, Texture2DMSArray, Cube, CubeArray, Raytracing> View;
		private:
			SERIALIZE()
			{
				ar& NVP(Resource);
				ar& NVP(Format);
				ar& NVP(View);
			}

		};

		struct RenderTarget
		{

			struct Buffer
			{
				uint64 FirstElement;
				uint NumElements;

			private:
				SERIALIZE()
				{
					ar& NVP(FirstElement);
					ar& NVP(NumElements);
				}
			};

			struct Texture1D
			{
				uint MipSlice;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
				}
			};

			struct Texture1DArray
			{
				uint MipSlice;
				uint FirstArraySlice;
				uint ArraySize;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
					ar& NVP(FirstArraySlice);
					ar& NVP(ArraySize);
				}
			};

			struct Texture2D
			{
				uint MipSlice;
				uint PlaneSlice;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
					ar& NVP(PlaneSlice);
				}
			};

			struct Texture2DMS
			{
				uint UnusedField_NothingToDefine;

			private:
				SERIALIZE()
				{
				}
			};

			struct Texture2DArray
			{
				uint MipSlice;
				uint FirstArraySlice;
				uint ArraySize;
				uint PlaneSlice;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
					ar& NVP(FirstArraySlice);
					ar& NVP(ArraySize);
					ar& NVP(PlaneSlice);
				}
			};

			struct Texture2DMSArray
			{
				uint FirstArraySlice;
				uint ArraySize;

			private:
				SERIALIZE()
				{
					ar& NVP(FirstArraySlice);
					ar& NVP(ArraySize);
				}
			};

			struct Texture3D
			{
				uint MipSlice;
				uint FirstWSlice;
				uint WSize;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
					ar& NVP(FirstWSlice);
					ar& NVP(WSize);
				}
			};

			Resource* Resource;
			Format Format;
			std::variant < Buffer, Texture1D, Texture1DArray, Texture2D, Texture2DArray, Texture2DMS, Texture2DMSArray, Texture3D > View;

		private:
			SERIALIZE()
			{
				ar& NVP(Resource);
				ar& NVP(Format);
				ar& NVP(View);
			}
		};

		struct ConstantBuffer
		{
			Resource* Resource;
			uint OffsetInBytes;
			uint SizeInBytes;

		private:
			SERIALIZE()
			{
				ar& NVP(Resource);
				ar& NVP(OffsetInBytes);
				ar& NVP(SizeInBytes);
			}
		};

		struct UnorderedAccess
		{
			struct Buffer
			{
				uint FirstElement;
				uint NumElements;
				uint StructureByteStride;
				bool Raw;
				uint64 CounterOffsetInBytes;
				Resource* CounterResource;

			private:
				SERIALIZE()
				{
					ar& NVP(FirstElement);
					ar& NVP(NumElements);
					ar& NVP(StructureByteStride);
					ar& NVP(Raw);
					ar& NVP(CounterOffsetInBytes);
					ar& NVP(CounterResource);
				}
			};

			struct Texture1D
			{
				uint MipSlice;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
				}
			};

			struct Texture1DArray
			{
				uint MipSlice;
				uint FirstArraySlice;
				uint ArraySize;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
					ar& NVP(FirstArraySlice);
					ar& NVP(ArraySize);
				}
			};

			struct Texture2D
			{
				uint MipSlice;
				uint PlaneSlice;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
					ar& NVP(PlaneSlice);
				}
			};

			struct Texture2DArray
			{
				uint MipSlice;
				uint FirstArraySlice;
				uint ArraySize;
				uint PlaneSlice;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
					ar& NVP(FirstArraySlice);
					ar& NVP(ArraySize);
					ar& NVP(PlaneSlice);
				}
			};

			struct Texture3D
			{
				uint MipSlice;
				uint FirstWSlice;
				uint WSize;

			private:
				SERIALIZE()
				{
					ar& NVP(MipSlice);
					ar& NVP(FirstWSlice);
					ar& NVP(WSize);
				}
			};

			Resource* Resource;
			Format Format;
			std::variant<Buffer, Texture1D, Texture1DArray, Texture2D, Texture2DArray, Texture3D> View;


		private:
			SERIALIZE()
			{
				ar& NVP(Resource);
				ar& NVP(Format);
				ar& NVP(View);
			}

		};

		struct IndexBuffer
		{
			Format Format;
			uint	SizeInBytes =0;
			uint64 OffsetInBytes=0;
			Resource* Resource = nullptr;
		private:
			SERIALIZE()
			{
				ar& NVP(Format);
				ar& NVP(SizeInBytes);
				ar& NVP(OffsetInBytes);

				ar& NVP(Resource);
			}
		};
	
		template <class T> concept ViewTemplate = std::is_same_v<T, UnorderedAccess>
		|| std::is_same_v<T, ConstantBuffer>
			|| std::is_same_v<T, RenderTarget>
			|| std::is_same_v<T, DepthStencil>
			|| std::is_same_v<T, ShaderResource>;

	}

}