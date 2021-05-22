#pragma once
#include "TilingParams.h"
struct TilingPostprocess_srv
{
	TilingParams_srv tiling;
};
struct TilingPostprocess
{
	TilingPostprocess_srv srv;
	TilingParams GetTiling() { return CreateTilingParams(srv.tiling); }

};
 const TilingPostprocess CreateTilingPostprocess(TilingPostprocess_srv srv)
{
	const TilingPostprocess result = {srv
	};
	return result;
}
