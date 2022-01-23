module;


export module d3d12_types;

export import d3d12;
import Utils;
import serialization;

import HAL.Types;

export
{


	namespace Graphics
	{





		enum class ResourceType : char
		{

			BUFFER,
			TEXTURE1D,
			//	TEXTURE1DARRAY,
			TEXTURE2D,
			//TEXTURE2DARRAY,
			TEXTURE3D,
			CUBE
		};



		struct ResourceViewDesc
		{
			ResourceType type;
			DXGI_FORMAT format;

			union
			{
				struct
				{

					UINT PlaneSlice;
					UINT MipSlice;
					UINT FirstArraySlice;
					UINT MipLevels;
					UINT ArraySize;
				} Texture2D;

				struct
				{
					UINT64 Size;
					UINT64 Offset;
					UINT64 Stride;
					bool counted;
				} Buffer;

			};
		};


	}

}


module:private;

