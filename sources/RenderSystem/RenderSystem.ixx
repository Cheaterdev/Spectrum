export module RenderSystem;

import HAL;
import Core;

export class RenderSystem : public Singleton<RenderSystem>
{
    friend class Singleton<RenderSystem>;

    std::shared_ptr<HAL::Device> m_device;

    static std::shared_ptr<HAL::Device> select_adapter();

public:
    explicit RenderSystem(std::shared_ptr<HAL::Device> device)
        : m_device(std::move(device)) {}

    ~RenderSystem() override = default;

    static std::shared_ptr<RenderSystem> create_singleton();

    HAL::Device& device() const { return *m_device; }
};
