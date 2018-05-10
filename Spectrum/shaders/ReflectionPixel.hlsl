

struct quad_output
{
	float4 pos : SV_POSITION;
	float2 tc : TEXCOORD0;
};

quad_output VS(uint index : SV_VERTEXID)
{
	static float2 Pos[] =
	{
		float2(-1, 1),
		float2(1, 1),
		float2(-1, -1),
		float2(1, -1)
	};
	static float2 Tex[] =
	{

		float2(0, 0),
		float2(1, 0),
		float2(0, 1),
		float2(1, 1),
	};
	quad_output Output;
	Output.pos = float4(Pos[index], 0.3, 1);
	Output.tc = Tex[index];
	return Output;
}




struct camera_info
{
	matrix view;
	matrix proj;
	matrix view_proj;
	matrix inv_view;
	matrix inv_proj;
	matrix inv_view_proj;
	float3 position;
	float3 direction;
};


cbuffer cbCamera : register(b0)
{
	camera_info camera;
};

cbuffer cbEffect : register(b1)
{
	float time;
};


float3 depth_to_wpos(float d, float2 tc, matrix mat)
{
	float4 P = mul(mat, float4(tc * float2(2, -2) + float2(-1, 1), d, 1));
	return P.xyz / P.w;
}

float3 depth_to_wpos_center(float d, float2 tc, matrix mat)
{
	float4 P = mul(mat, float4(tc, d, 1));
	return P.xyz / P.w;
}

float3 project_tc(float3 pos, matrix mat)
{
	float4 res = mul(mat, float4(pos, 1));
	res /= res.w;
	return float3(res.xy * float2(0.5, -0.5) + float2(0.5, 0.5), res.z);
}

Texture2D<float4> gbuffer[3] : register(t0);
Texture2D<float> depth_buffer : register(t3);
Texture2D<float4> dist_buffer : register(t4);
Texture2D<float4> light_buffer: register(t5);

RWTexture2D<float4> result: register(u0);
//groupshared float4 shared_mem[SIZE][SIZE];
SamplerState LinearSampler : register(s0);
SamplerState PixelSampler : register(s1);

float calc_fresnel(float k0, float3 n, float3 v)
{
	float ndv = saturate(dot(n, -v));
	return k0 + (1 - k0) * pow(1 - ndv, 5);
	return k0 + (1 - k0) * (1 - pow(dot(n, -v), 1));
}

float calc_vignette(float2 inTex)
{
	float2 T = saturate(inTex);
	// float vignette = 16 * (T.x * T.y * ((1 - T.x) * (1 - T.y)));
	float vignette = 4 * (T.y * ((1 - T.y)));
	return vignette;
}
float rnd(float2 uv)
{
	return frac(sin(dot(uv, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
}


float4 PS(quad_output i) : SV_TARGET0
{
	float2 dims;
	gbuffer[0].GetDimensions(dims.x, dims.y);
	//i.tc-=float2(0.5,0.5)/dims;
		float level = 0;

		// float4 albedo = gbuffer[0].SampleLevel(PixelSampler, i.tc, 0);
		 float3 normal = normalize(gbuffer[1].SampleLevel(PixelSampler, i.tc, level).xyz * 2 - 1);
		 //	float4 specular = gbuffer[1].SampleLevel(PixelSampler, i.tc, 0).w;
			// specular.w = 1;


			 float raw_z = depth_buffer.SampleLevel(PixelSampler, i.tc, level);
			 if (raw_z == 1) return 0;
			 float3 pos = depth_to_wpos(raw_z, i.tc,   camera.inv_view_proj);
			 float3 v = -normalize(camera.position - pos);
			 float len = length(camera.position - pos);
			 //   float fresnel = calc_fresnel(specular.w, normal, v);

				float3 r = reflect(v, normal);

				//r+=0.005*normal*pow(fresnel,0.25);//*(1-specular.w);
					//r=normalize(r);
					float3 refl_pos = pos;
					// float sini = sin(time * 220 + float(i.tc.x));
					// float cosi = cos(time * 220 + float(i.tc.y));
					 //float rand = rnd(float2(sini, cosi));
					 //float rand2 = rnd(float2(cosi, sini));

					 //float rcos = cos(6.14*rand);
					 //float rsin = sin(6.14*rand);

					 //float3 right = normalize(cross(r, float3(0, 1, 0.1)));
					 //float3 tangent = normalize(cross(right, r));


					 //r += rand2*specular.w*(tangent*rsin + right*rcos);

					 r = normalize(r);
					 float errorer = distance(camera.position, pos) / 10;

					 float orig_dist = distance(camera.position, pos);


					 float start_offset = orig_dist / 100;

					// float error = orig_dist/
					 float dist = start_offset/1; //lerp(10,20,1-fresnel);

					 float raw_z_reflected = 0;
					 float3 reflect_tc = float3(i.tc, raw_z);


					 float delta = 0.1;
					 bool direction = true;
					 bool hit = false;
					 for (int i = 0; i < 120; i++)
					 {

						 float3 cur_pos = pos + dist * r;
						 reflect_tc = project_tc(cur_pos, camera.view_proj);
						 raw_z_reflected = depth_buffer.SampleLevel(PixelSampler, reflect_tc.xy, level).x;
						 refl_pos = depth_to_wpos(raw_z_reflected, float2(reflect_tc.xy), camera.inv_view_proj);



						 if (raw_z_reflected < reflect_tc.z&&length(refl_pos - cur_pos) < start_offset)
						 {

							 
							hit = true;

							if (!direction) delta /= 2;
							dist -= delta;
							direction = true;
							 
							//break;
							 //delta /= 2;
						 }
						 else
						 {
							 if (!hit) delta = delta * 1.042;// min(delta * 2, 1);
							 if (direction) delta /= 2;
							 direction = false;

							 dist += delta;
						 }
						 //else
						 //	dist += 0.5;


						 // distance(pos, refl_pos);


					 //	if(raw_z_reflected)
					 //	 level = clamp(dist/8, 0, 5);
						 /*if(raw_z_reflected<raw_z)
						 {/
						 dist*=3;
						 level*=2;
						 }	//level=12;*/
					 }

					 /**/

					 //if (dot(v, r) < 0.5)
					 {
						 //dist = orig_dist /2;
					 }
					 for (int i = 0; i < -18; i++)
					 {
						 reflect_tc = project_tc(pos + dist * r, camera.view_proj);
						 raw_z_reflected = depth_buffer.SampleLevel(LinearSampler, reflect_tc.xy, level).x;
						 refl_pos = depth_to_wpos(raw_z_reflected, float2(reflect_tc.xy), camera.inv_view_proj);
						 dist = distance(pos, refl_pos);
						 // level = clamp(8 * dist * (1 - fresnel) / orig_dist, 0, 5);

					  }

					  reflect_tc = project_tc(pos + dist * r, camera.view_proj);
					  float3 delta_position = pos - refl_pos;
					  float reflection_distance = distance(pos + dist * r, refl_pos);
					  float  dist2 = distance(pos, refl_pos);



					  float vignette = calc_vignette(reflect_tc);
					  float bad = 1;// pow(saturate((dot(r, -delta_position) / dist2)), 264);
					    bad *= saturate(1 - 2 * reflection_distance / errorer);
					// bad *= saturate(1-dot(v,r));
					 bad *= (raw_z_reflected < 1);


					 //dist += (1-bad) * 100;
				 //	bad *= saturate(1-8*abs(dist-dist2));
					// level +=  8 * saturate(1 - 1 * bad);


				   ///  float4 reflect_color = light_buffer.SampleLevel(LinearSampler, reflect_tc, 6*dist);
					 //float3 reflect_normal = normalize(gbuffer[1].SampleLevel(LinearSampler, reflect_tc, 0).xyz*2-1);



					 //float4 res = 0 ;

					 //return reflect_color;
					// float lerper = 1 - reflect_color.w * pow(0.9, (1 - bad) * 32);

					 //float3 direct = vignette * reflect_color.xyz *( bad);// pow(0.9, (1 - bad) * 32);


					 //bad *= saturate(0.5+0.5*dot(-reflect_normal.xyz,normal.xyz));
					 //float3 indirect = 0;// cube_probe.SampleLevel(LinearSampler, r, (1 - fresnel) * 8.0f);
					 //res.xyz = fresnel*direct;// lerp(direct, indirect, lerper);

				  //   res.xyz += ambient_texture.SampleLevel(LinearSampler, reflect_tc, level);
					   // res =1;
					 return float4(dist, raw_z_reflected, raw_z, bad);// float4(direct.xyz, bad);
}



float4 PS_LAST(quad_output i) : SV_TARGET0
{

	float4 dist = dist_buffer.SampleLevel(LinearSampler, i.tc, 0);


	float3 normal = normalize(gbuffer[1].SampleLevel(PixelSampler, i.tc, 0).rgb * 2 - 1);
	float4 specular = gbuffer[2].SampleLevel(PixelSampler, i.tc, 0);


	float raw_z = depth_buffer.SampleLevel(PixelSampler, i.tc, 0);
	//	if (raw_z == 1) return 0;
		float3 pos = depth_to_wpos(raw_z, i.tc, camera.inv_view_proj);



		float3 v = -normalize(camera.position - pos);
		float len = length(camera.position - pos);

		float3 r = reflect(v, normal);



		float2 tc = project_tc(pos + dist.x * r, camera.view_proj);

		float vignette = calc_vignette(tc);

		float4 albedo = light_buffer.SampleLevel(LinearSampler,tc,clamp(dist.x,0,8));


		float w = 1;// saturate(1 - dist.x / 8);

		//return float4(dist.xxx/18, 1);
		return  float4(w*vignette*albedo.xyz*dist.w,0.1);
}


