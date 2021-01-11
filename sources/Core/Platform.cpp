#include  "pch.h"
#include "Platform.h"

SystemInfo::SystemInfo()
{
    // Calculate core count
    nCoreCount = 0;
    #pragma omp parallel
    {
        nCoreCount++;
    }
}

SystemInfo::~SystemInfo()
{}