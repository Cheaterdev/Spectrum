#pragma once
struct BlueNoise
{
	uint frame_index; // uint
	uint sobol_buffer; // Buffer<uint>
	uint ranking_tile_buffer; // Buffer<uint>
	uint scrambling_tile_buffer; // Buffer<uint>
	uint blue_noise_texture; // RWTexture2D<float2>
	uint GetFrame_index() { return frame_index; }
	Buffer<uint> GetSobol_buffer() { return ResourceDescriptorHeap[sobol_buffer]; }
	Buffer<uint> GetRanking_tile_buffer() { return ResourceDescriptorHeap[ranking_tile_buffer]; }
	Buffer<uint> GetScrambling_tile_buffer() { return ResourceDescriptorHeap[scrambling_tile_buffer]; }
	RWTexture2D<float2> GetBlue_noise_texture() { return ResourceDescriptorHeap[blue_noise_texture]; }
};
