#pragma once
struct PSSMConstants_cb
{
	int level;
	float time;
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
