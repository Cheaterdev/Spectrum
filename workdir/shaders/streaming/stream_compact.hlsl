// Compacts a streamer's feedback (see streaming.prism). One thread per page id.
//
// `result` layout, in dwords; must match Graphics:Streaming's reader (QuantaResidency.cpp):
//   [0, 16)            number of requests found per priority bucket (may exceed the bucket's capacity)
//   [16]               requests that did not fit in their bucket
//   [17]               number of eviction candidates found
//   [18]               candidates that did not fit
//   [32, ...)          request entries, BUCKET_CAPACITY per bucket: (page id, priority)
//   [... , ...)        eviction entries, EVICT_CAPACITY of them: (page id, last epoch the page was reached or shown)

#include "../autogen/StreamCompact.h"

static const uint REQUEST_BUCKETS = 16;
static const uint BUCKET_CAPACITY = 32;
static const uint EVICT_CAPACITY = 128;

static const uint OUT_REQUEST_OVERFLOW = 16;
static const uint OUT_EVICT_COUNT = 17;
static const uint OUT_EVICT_OVERFLOW = 18;
static const uint OUT_REQUESTS = 32;
static const uint OUT_EVICTS = OUT_REQUESTS + REQUEST_BUCKETS * BUCKET_CAPACITY * 2;

// Streaming::FlagLoading / FlagEvictable.
static const uint FLAG_LOADING = 1;
static const uint FLAG_EVICTABLE = 2;

static const uint INVALID = 0xFFFFFFFF;

// The priority is half-float error bits, which order like their values for positive halves: the exponent is a log2
// scale. 2^-5 and below share the lowest bucket, 2^10 and above the highest.
uint PriorityBucket(uint priority)
{
    int exponent = int((priority >> 10) & 31);
    return uint(clamp(exponent - 10, 0, int(REQUEST_BUCKETS) - 1));
}

#ifdef BUILD_FUNC_CS
[numthreads(64, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
    StreamCompact data = GetStreamCompact();
    const uint page = dispatchID.x;
    if (page >= data.GetPage_count())
        return;

    RWStructuredBuffer<uint> result = data.GetResult();
    const uint flags = data.GetPage_flags()[page * 2];

    const uint want = data.GetRequests()[page * 2];
    if (want != 0 && (want >> 16) == (data.GetEpoch() & 0xFFFF)
        && data.GetPage_slots()[page] == INVALID && (flags & FLAG_LOADING) == 0)
    {
        const uint priority = want & 0xFFFF;
        const uint bucket = PriorityBucket(priority);

        uint index;
        InterlockedAdd(result[bucket], 1, index);
        if (index < BUCKET_CAPACITY)
        {
            const uint entry = OUT_REQUESTS + (bucket * BUCKET_CAPACITY + index) * 2;
            result[entry] = page;
            result[entry + 1] = priority;
        }
        else
        {
            uint ignored;
            InterlockedAdd(result[OUT_REQUEST_OVERFLOW], 1, ignored);
        }
    }

    if ((flags & FLAG_EVICTABLE) != 0)
    {
        const uint last = max(data.GetPage_flags()[page * 2 + 1], data.GetRequests()[page * 2 + 1]);
        if (last + data.GetEviction_age() < data.GetEpoch())
        {
            uint index;
            InterlockedAdd(result[OUT_EVICT_COUNT], 1, index);
            if (index < EVICT_CAPACITY)
            {
                const uint entry = OUT_EVICTS + index * 2;
                result[entry] = page;
                result[entry + 1] = last;
            }
            else
            {
                uint ignored;
                InterlockedAdd(result[OUT_EVICT_OVERFLOW], 1, ignored);
            }
        }
    }
}
#endif
