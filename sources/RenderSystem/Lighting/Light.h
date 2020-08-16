#pragma once
#include <random>


class Light : public scene_object
{
public:
	vec3 color;
	vec3 position;
	float radius;


};

struct LightSRV
{
	float4 color_radius;
	float4 position_dist;
};
class LightSystem:public Events::prop_handler
{
	Render::ComputePipelineState::ptr state;

public:
	std::vector<Light> lights;
	PSSM pssm;
	LightSystem();







	void process(MeshRenderContext::ptr& context, G_Buffer& buffer, main_renderer::ptr r, scene_object::ptr scene);



};