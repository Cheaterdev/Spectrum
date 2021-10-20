#pragma once
#include <7zip/C/7zTypes.h>

namespace detail
{
  class lzma_alloc
    : public ISzAlloc
  {
    public:
      lzma_alloc()
      {
        this->Alloc = [](ISzAllocPtr, size_t size)   { return malloc(size); };
        this->Free  = [](ISzAllocPtr, void* address) { free(address); };
      }
  };
}
