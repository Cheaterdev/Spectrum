export module HAL:API.CommandAllocator;

import :Types;
import :Utils;
import d3d12;
import Core;

export namespace HAL {

    namespace API {
        class CommandAllocator {
        protected:

        public:
            D3D::CommandAllocator m_commandAllocator;
        };
    }
}