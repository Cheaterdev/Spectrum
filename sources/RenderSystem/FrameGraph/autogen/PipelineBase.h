#pragma once
#include <span>

namespace Pipelines
{

class PipelineBase
{
public:
	virtual ~PipelineBase() = default;
	virtual std::span<const wchar_t* const> GetUsedPassNamesList() const = 0;
};

}
