#pragma once
import stl.core;

namespace Passes
{

class PassNodeBase
{
public:
	virtual ~PassNodeBase() = default;
	virtual std::span<const FrameGraph::ResourceAccess> GetUsedResourcesList() const = 0;
};

}
