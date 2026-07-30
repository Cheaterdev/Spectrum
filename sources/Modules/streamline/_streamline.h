#pragma once
// sl_dlss.h/sl_dlss_d.h aren't self-contained (assume sl.h came first), so
// this aggregating header establishes the required order — same fix as cereal's _cereal.h.
#include <streamline/sl.h>

#include <streamline/sl_dlss.h>     // Super Resolution   (sl::kFeatureDLSS)
#include <streamline/sl_dlss_d.h>   // Ray Reconstruction (sl::kFeatureDLSS_RR)
