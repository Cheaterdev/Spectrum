export module HAL:API.CommandAllocator;
import Core;
import d3d12;
import :Types;
import :Utils;

export namespace HAL {

	namespace API
	{
	
		class CommandAllocator
		{
		protected:

		public:
			D3D::CommandAllocator m_commandAllocator;
			
		};
	}



}