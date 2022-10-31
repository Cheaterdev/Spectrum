export module HAL:API.RootSignature;

import d3d12;
import :Utils;

export namespace HAL
{
	namespace API
	{
		class RootSignature
		{
		protected:
			D3D::RootSignature m_rootSignature;
		public:
			virtual~RootSignature() = default;
			D3D::RootSignature get_native();
		};
	}
}

