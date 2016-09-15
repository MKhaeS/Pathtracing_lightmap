#pragma once

#include "FatDxResource.h"

namespace FatDx
{
    class Buffer : public Resource
    {
    public:
        Buffer (ComPtr<ID3D12Device>   device, const D3D12_HEAP_TYPE&  heaptype,
                 const UINT64& size);
        void Map (BYTE** ptr);
        void Unmap ();

    private:
        bool m_isMapped = false;
    };
}
