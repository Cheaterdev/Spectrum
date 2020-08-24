#pragma once
struct Test_cb
{
	float data[16];
};
struct Test
{
	Test_cb cb;
	float GetData(int i) { return cb.data[i]; }
	Texture2D GetTex(int i) { return bindless[i]; }

};
 const Test CreateTest(Test_cb cb)
{
	const Test result = {cb
	};
	return result;
}
