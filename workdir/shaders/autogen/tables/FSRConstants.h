#pragma once
struct FSRConstants_cb
{
	uint4 Const0;
	uint4 Const1;
	uint4 Const2;
	uint4 Const3;
	uint4 Sample;
};
struct FSRConstants
{
	FSRConstants_cb cb;
	uint4 GetConst0() { return cb.Const0; }
	uint4 GetConst1() { return cb.Const1; }
	uint4 GetConst2() { return cb.Const2; }
	uint4 GetConst3() { return cb.Const3; }
	uint4 GetSample() { return cb.Sample; }

};
 const FSRConstants CreateFSRConstants(FSRConstants_cb cb)
{
	const FSRConstants result = {cb
	};
	return result;
}
