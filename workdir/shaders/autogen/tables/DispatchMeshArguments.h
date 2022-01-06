#pragma once
struct DispatchMeshArguments
{
	uint3 counts; // uint3
	uint3 GetCounts() { return counts; }
};
