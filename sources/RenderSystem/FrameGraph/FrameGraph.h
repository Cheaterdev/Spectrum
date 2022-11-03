#pragma once

import Graphics;
import Math;

class main_renderer;
class Scene;


struct TimeInfo
{
	
	float time;
	float totalTime = 0;
	
	
	
};

struct SkyInfo
{
	float3 sunDir;
};
struct SceneInfo
{
	main_renderer* renderer;
	Scene* scene;
};

struct CameraInfo
{
	camera* cam;
};

struct ViewportInfo
{
	ivec2 frame_size;
	ivec2 upscale_size;
};
#define H(x) x = #x