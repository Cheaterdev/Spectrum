export module HAL:Impl;

import :Debug;
import d3d12;
import wrl;

export namespace HAL {

    void EnableGPUDebug();
    void EnableShaderModel();
    void init();
}