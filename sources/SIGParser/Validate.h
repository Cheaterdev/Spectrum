#pragma once
#include "Parsed.h"

// Semantic checks over the merged model. Reports into diagnostics(); the
// caller decides whether to generate.
void validate(Parsed& parsed);
