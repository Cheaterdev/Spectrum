#pragma once
struct FontRenderingConstants
{
	float4x4 TransformMatrix; // float4x4
	float4 ClipRect; // float4
	float4x4 GetTransformMatrix() { return TransformMatrix; }
	float4 GetClipRect() { return ClipRect; }
};
