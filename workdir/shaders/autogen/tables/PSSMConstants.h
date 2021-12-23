#pragma once
struct PSSMConstants_cb
{
	int level; // int
	float time; // float
};
struct PSSMConstants
{
	PSSMConstants_cb cb;
	int GetLevel() { return cb.level; }
	float GetTime() { return cb.time; }

};
 const PSSMConstants CreatePSSMConstants(PSSMConstants_cb cb)
{
	const PSSMConstants result = {cb
	};
	return result;
}
