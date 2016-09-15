#include "FatDxResource.h"

ID3D12Resource* FatDx::Resource::Get () const{
    return m_Resource.Get();
}

FatDx::Resource::Resource () {}

void FatDx::Resource::StateTransition (ComPtr<ID3D12GraphicsCommandList> cmdlist, 
                                       D3D12_RESOURCE_STATES nextstate          ) {
    if (m_State == nextstate) {
        return;
    }

    D3D12_RESOURCE_BARRIER barrier               = {};
    barrier.Type                                 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags                                = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource                 = m_Resource.Get ();
    barrier.Transition.StateBefore               = m_State;
    barrier.Transition.StateAfter                = nextstate;
    barrier.Transition.Subresource               = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    cmdlist->ResourceBarrier (1, &barrier);
    m_State = nextstate;
}
