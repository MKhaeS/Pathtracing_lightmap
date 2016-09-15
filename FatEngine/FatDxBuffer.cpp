#include <assert.h>

#include "FatDxBuffer.h"

FatDx::Buffer::Buffer (ComPtr<ID3D12Device> device, const D3D12_HEAP_TYPE & heaptype, const UINT64 & size) { 
    m_Device = device;
    m_HeapType = heaptype;

    D3D12_HEAP_PROPERTIES                           heapProperties;
    heapProperties.Type                             = heaptype;
    heapProperties.CPUPageProperty                  = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference             = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask                 = 1;
    heapProperties.VisibleNodeMask                  = 1;

    D3D12_RESOURCE_DESC                             defResourceDescription;
    defResourceDescription.Dimension                = D3D12_RESOURCE_DIMENSION_BUFFER;
    defResourceDescription.Format                   = DXGI_FORMAT_UNKNOWN;
    defResourceDescription.Width                    = size;
    defResourceDescription.SampleDesc.Count         = 1;
    defResourceDescription.SampleDesc.Quality       = 0;
    defResourceDescription.Alignment                = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    defResourceDescription.DepthOrArraySize         = 1;
    defResourceDescription.Height                   = 1;
    defResourceDescription.MipLevels                = 1;
    defResourceDescription.Flags                    = D3D12_RESOURCE_FLAG_NONE;
    defResourceDescription.Layout                   = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    m_State = (heaptype == D3D12_HEAP_TYPE_DEFAULT) ? D3D12_RESOURCE_STATE_COMMON
                                                    : D3D12_RESOURCE_STATE_GENERIC_READ;

    HRESULT res = m_Device->CreateCommittedResource (&heapProperties, D3D12_HEAP_FLAG_NONE,
                                                     &defResourceDescription, m_State,
                                                     nullptr, IID_PPV_ARGS (&m_Resource));
    if (FAILED (res)) {
        assert (!"Failed to create GPU buffer resource");
    }
    m_Description = m_Resource->GetDesc ();
}

void FatDx::Buffer::Map (BYTE ** ptr) {
    if (m_HeapType == D3D12_HEAP_TYPE_UPLOAD) {
        m_Resource->Map (0, nullptr, reinterpret_cast<void**>(ptr));
        m_isMapped = true;
    }
}

void FatDx::Buffer::Unmap () {
    if (m_isMapped) {
        m_Resource->Unmap (0, nullptr);
    }
}
