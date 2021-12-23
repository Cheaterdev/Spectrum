#pragma once
struct Test_cb
{
	float data[16]; // float
};
struct Test
{
	Test_cb cb;
	Texture2D<float4> GetTex(int i) { return bindless[i]; }
	float GetData(int i) { return cb.data[i]; }

};
 const Test CreateTest(Test_cb cb)
{
	const Test result = {cb
	};
	return result;
}
