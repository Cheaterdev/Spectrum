class PSSM:public FrameGraphGenerator
{
	Render::PipelineState::ptr draw_mask_state;
	Render::PipelineState::ptr draw_result_state;

	ShaderMaterial::ptr mat;


	ivec2 size = { 1024, 1024 };
	float2 pixel_size = float2(1, 1) / float2(1024, 1024);


	float scaler = 1;
	
	static const int renders_size = 6;

	std::mutex pos_mutex;
	float3 position;
	size_t counter = 0;
public:

	virtual void generate(FrameGraph& graph) override;

	float3 get_position();

	void set_position(float3 p);

	PSSM();

};
