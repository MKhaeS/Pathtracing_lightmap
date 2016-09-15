#pragma once
#include <memory>
#include <wrl.h>
#include <string>

#include "d3d12.h"

using namespace Microsoft::WRL;
using namespace std;

namespace FatDx
{
    class Resource
    {        
        Resource (const Resource&) = delete;
        Resource& operator = (const Resource&) = delete;
    public:
        ID3D12Resource* Get () const;

    protected:
        Resource ();
        ComPtr<ID3D12Device>    m_Device        =   nullptr;
        ComPtr<ID3D12Resource>  m_Resource      =   nullptr;
        D3D12_RESOURCE_DESC     m_Description   =   {};
        D3D12_HEAP_TYPE         m_HeapType      =   D3D12_HEAP_TYPE_DEFAULT;
        D3D12_RESOURCE_STATES   m_State         =   D3D12_RESOURCE_STATE_COMMON;

        void StateTransition ( ComPtr<ID3D12GraphicsCommandList> cmdlist,
                               D3D12_RESOURCE_STATES     nextstate);
    };
}