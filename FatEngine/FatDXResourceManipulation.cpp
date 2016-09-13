#include "FatDXFramework.h"


ComPtr<ID3D12Resource> FatDXFramework::CreateGpuBuffer ( const UINT64 &     bufferSize ) {
    // This buffer represents real allocated memory on GPU
    ComPtr<ID3D12Resource> gpuBuffer               = nullptr;
    D3D12_HEAP_PROPERTIES                          defHeapProperties;
    defHeapProperties.Type                         = D3D12_HEAP_TYPE_DEFAULT;
    defHeapProperties.CPUPageProperty              = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    defHeapProperties.MemoryPoolPreference         = D3D12_MEMORY_POOL_UNKNOWN;
    defHeapProperties.CreationNodeMask             = 1;
    defHeapProperties.VisibleNodeMask              = 1;

    D3D12_RESOURCE_DESC                            defResourceDescription;
    defResourceDescription.Dimension               = D3D12_RESOURCE_DIMENSION_BUFFER;
    defResourceDescription.Format                  = DXGI_FORMAT_UNKNOWN;
    defResourceDescription.Width                   = bufferSize;
    defResourceDescription.SampleDesc.Count        = 1;
    defResourceDescription.SampleDesc.Quality      = 0;
    defResourceDescription.Alignment               = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    defResourceDescription.DepthOrArraySize        = 1;
    defResourceDescription.Height                  = 1;
    defResourceDescription.MipLevels               = 1;
    defResourceDescription.Flags                   = D3D12_RESOURCE_FLAG_NONE;
    defResourceDescription.Layout                  = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    HRESULT res = m_Device->CreateCommittedResource ( &defHeapProperties, D3D12_HEAP_FLAG_NONE,
                                                       &defResourceDescription, D3D12_RESOURCE_STATE_COMMON,
                                                       nullptr, IID_PPV_ARGS ( &gpuBuffer ) );
    if ( FAILED ( res ) ) {
        assert ( !"Failed to create GPU buffer resource" );
    }
    return gpuBuffer;
}



ComPtr<ID3D12Resource> FatDXFramework::CreateUploadBuffer ( const UINT64 &  bufferSize ) {
    // This buffer represents CPU memory which is mediator for uploading data into GPU memory or
    // used to store dynamic data
    ComPtr<ID3D12Resource>                            uploadBuffer = nullptr;
    D3D12_HEAP_PROPERTIES                             uploadHeapProperties;
    uploadHeapProperties.Type                         = D3D12_HEAP_TYPE_UPLOAD;
    uploadHeapProperties.CPUPageProperty              = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    uploadHeapProperties.MemoryPoolPreference         = D3D12_MEMORY_POOL_UNKNOWN;
    uploadHeapProperties.CreationNodeMask             = 1;
    uploadHeapProperties.VisibleNodeMask              = 1;

    D3D12_RESOURCE_DESC                               uploadResourceDescription;
    uploadResourceDescription.Dimension               = D3D12_RESOURCE_DIMENSION_BUFFER;
    uploadResourceDescription.Format                  = DXGI_FORMAT_UNKNOWN;
    uploadResourceDescription.Width                   = bufferSize;
    uploadResourceDescription.SampleDesc.Count        = 1;
    uploadResourceDescription.SampleDesc.Quality      = 0;
    uploadResourceDescription.Alignment               = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    uploadResourceDescription.DepthOrArraySize        = 1;
    uploadResourceDescription.Height                  = 1;
    uploadResourceDescription.MipLevels               = 1;
    uploadResourceDescription.Flags                   = D3D12_RESOURCE_FLAG_NONE;
    uploadResourceDescription.Layout                  = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    HRESULT res  = m_Device->CreateCommittedResource ( &uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
                                                        &uploadResourceDescription, D3D12_RESOURCE_STATE_GENERIC_READ,
                                                        nullptr, IID_PPV_ARGS ( &uploadBuffer ) );
    if ( FAILED ( res ) ) {
        assert ( !"Failed to create upload buffer resource" );
    }

    return uploadBuffer;
}


ComPtr<ID3D12Resource> FatDXFramework::LoadDataToGpu ( const void * pVertexData, const UINT64& dataSize,
                                                       ComPtr<ID3D12Resource>& uploadBuffer ) {
    // Important to keep pointer to uploadBuffer after the function returns
    // untill GPU stops executing after m_CommandQueue_->ExecuteCommandLists call
    ComPtr<ID3D12Resource> gpuBuffer = CreateGpuBuffer ( dataSize );
    uploadBuffer = CreateUploadBuffer ( dataSize );
    ResourceStateTransition ( gpuBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST );
    CopyDataToBuffer ( gpuBuffer, uploadBuffer, pVertexData, dataSize );
    ResourceStateTransition ( gpuBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ );

    return gpuBuffer;
}



bool FatDXFramework::CopyDataToBuffer ( const ComPtr<ID3D12Resource>&   destination,
                                        const ComPtr<ID3D12Resource>&   upload,
                                        const void *                    pDataSource,
                                        const UINT64 &                  dataSize )  const {
    D3D12_RESOURCE_DESC uploadDesc          = upload->GetDesc();
    D3D12_RESOURCE_DESC destinationDesc     = destination->GetDesc();
    if ( uploadDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
         destinationDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
         uploadDesc.Width           <  dataSize ) {
        assert ( !"Resources are not buffers or have invalid size" );
        return false;
    }

    BYTE* pDataUpload;
    HRESULT res = upload->Map( 0, nullptr, reinterpret_cast<void**> (&pDataUpload) );
    if ( FAILED( res ) ) {
        return false;
    }
    memcpy( pDataUpload, pDataSource, dataSize );
    upload->Unmap ( 0, nullptr );

    m_CommandList_->CopyResource( destination.Get(), upload.Get() );

    return true;
}



void FatDXFramework::ResourceStateTransition ( const ComPtr<ID3D12Resource>&    resource,
                                               D3D12_RESOURCE_STATES            prev_state,
                                               D3D12_RESOURCE_STATES            next_state ) {
    if ( prev_state == next_state ) {
        return;
    }

    D3D12_RESOURCE_BARRIER                           copyBarrier = {};
    copyBarrier.Type                                 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    copyBarrier.Flags                                = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    copyBarrier.Transition.pResource                 = resource.Get ();
    copyBarrier.Transition.StateBefore               = prev_state;
    copyBarrier.Transition.StateAfter                = next_state;
    copyBarrier.Transition.Subresource               = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_CommandList_->ResourceBarrier ( 1, &copyBarrier );
}