
struct node_data
{
	matrix node_global_matrix;
	matrix node_inverse_matrix;
};

struct draw_info
{
	float3 mn;
	float3 mx;
	unsigned int instance_id;
	unsigned int pipeline_id;
	node_data node;
};


struct dispatch_info
{
	unsigned int count;
	uint3 dispatch_count;
};


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
//#define OCCLUSION_TEST_ENABLED
StructuredBuffer<draw_info> all_list:register(t0);
//AppendStructuredBuffer<draw_info> render_list:register(u0);
RWStructuredBuffer<uint> visible_list:register(u0);
RWStructuredBuffer<uint> view_buffer: register(u1);
RWStructuredBuffer<dispatch_info> dispach_buffer:register(u2);
AppendStructuredBuffer<draw_info> occluder_test_list: register(u3);

cbuffer info : register(b0)
{
	uint count;
	uint per_thread;
};

cbuffer camera_info: register(b1)
{
	float4 planes[6];
};


bool is_visible(float3 pos, float l)
{

	for (int i = 0; i < 6; i++)
	{
		float d = dot(planes[i], float4(pos, 1));


		if (d > l) return false;
	}
	return true;
}


cbuffer cbCamera : register(b2)
{
	camera_info camera;
};

bool is_camera_inside(draw_info aabb)
{
	float4 cam_pos = mul(aabb.node.node_inverse_matrix, float4(camera.position, 1));
	cam_pos /= cam_pos.w;
	//cam_pos.xyz = camera.position;

	return all((cam_pos.xyz>aabb.mn)&&(cam_pos.xyz<aabb.mx));
}
bool is_visible(draw_info aabb)
{
	float4 p[8];
	p[0] = float4(aabb.mx.x, aabb.mx.y, aabb.mx.z,1);
	p[1] = float4(aabb.mx.x, aabb.mx.y, aabb.mn.z, 1);
	p[2] = float4(aabb.mx.x, aabb.mn.y, aabb.mx.z, 1);
	p[3] = float4(aabb.mx.x, aabb.mn.y, aabb.mn.z, 1);
	p[4] = float4(aabb.mn.x, aabb.mx.y, aabb.mx.z, 1);
	p[5] = float4(aabb.mn.x, aabb.mx.y, aabb.mn.z, 1);
	p[6] = float4(aabb.mn.x, aabb.mn.y, aabb.mx.z, 1);
	p[7] = float4(aabb.mn.x, aabb.mn.y, aabb.mn.z, 1);


	for (int i = 0; i < 6; i++)
	{
		bool bFullyOut = true;

		float4 my_plane =  mul(planes[i], aabb.node.node_global_matrix);
	//	float4 my_plane = mul(planes[i],aabb.node.node_inverse_matrix);


		for (int j = 0; j < 8; ++j)
		{
			float d = dot(my_plane, p[j]);

			if (d > 0)
				bFullyOut = false;
		}


		if (bFullyOut)
			return false;
	}


	return true;
}

groupshared uint max_count = 0;

[numthreads(64, 1, 1)]
void CS(
	uint3 groupID       : SV_GroupID,
	uint3 dispatchID : SV_DispatchThreadID,
	uint3 groupThreadID : SV_GroupThreadID,
	uint  groupIndex : SV_GroupIndex
)
{

	uint tid = groupThreadID.x + 64 * groupID.x;
	uint start_index = per_thread*(tid);
	uint end_index = min(count, start_index + per_thread);

	uint my_count = 0;
	for (uint index = start_index; index < end_index; index++)
	{

		draw_info info = all_list[index];


		bool inside =  is_camera_inside(info);

		if (inside || is_visible(info))
		{
			uint counter = visible_list.IncrementCounter();
			visible_list[counter] = info.instance_id;

#ifdef OCCLUSION_TEST_ENABLED

			if (!inside)
				occluder_test_list.Append(info);
			else
			{
				view_buffer[info.instance_id] = info.pipeline_id;
			}
			//else
			//	occluder_test_list[counter].instance_id = -1;
#else
			view_buffer[info.instance_id] = info.pipeline_id;
#endif
			my_count++;
		}
		else
			view_buffer[info.instance_id] = -1;
	}
}

