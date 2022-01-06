#pragma once
struct FSRConstants
{
	uint4 Const0; // uint4
	uint4 Const1; // uint4
	uint4 Const2; // uint4
	uint4 Const3; // uint4
	uint4 Sample; // uint4
	uint4 GetConst0() { return Const0; }
	uint4 GetConst1() { return Const1; }
	uint4 GetConst2() { return Const2; }
	uint4 GetConst3() { return Const3; }
	uint4 GetSample() { return Sample; }
};
