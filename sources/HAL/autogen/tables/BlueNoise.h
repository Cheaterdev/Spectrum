#pragma once
namespace Table 
{
	#pragma pack(push, 1)
	struct BlueNoise
	{
		uint frame_index;
		HLSL::Buffer<uint> sobol_buffer;
		HLSL::Buffer<uint> ranking_tile_buffer;
		HLSL::Buffer<uint> scrambling_tile_buffer;
		HLSL::RWTexture2D<float2> blue_noise_texture;
		uint& GetFrame_index() { return frame_index; }
		HLSL::Buffer<uint>& GetSobol_buffer() { return sobol_buffer; }
		HLSL::Buffer<uint>& GetRanking_tile_buffer() { return ranking_tile_buffer; }
		HLSL::Buffer<uint>& GetScrambling_tile_buffer() { return scrambling_tile_buffer; }
		HLSL::RWTexture2D<float2>& GetBlue_noise_texture() { return blue_noise_texture; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(frame_index);
			compiler.compile(sobol_buffer);
			compiler.compile(ranking_tile_buffer);
			compiler.compile(scrambling_tile_buffer);
			compiler.compile(blue_noise_texture);
		}
		struct Compiled
		{
			uint frame_index; // uint
			uint sobol_buffer; // Buffer<uint>
			uint ranking_tile_buffer; // Buffer<uint>
			uint scrambling_tile_buffer; // Buffer<uint>
			uint blue_noise_texture; // RWTexture2D<float2>
		};
	};
	#pragma pack(pop)
}
