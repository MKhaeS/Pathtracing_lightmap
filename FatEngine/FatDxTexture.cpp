#include <fstream>
#include <assert.h>

#include "d3dcompiler.h"

#include "FatDxTexture.h"
#include "Dds.h"

using namespace std;


FatDx::Texture::Texture (ComPtr<ID3D12Device>   device,     const D3D12_HEAP_TYPE&  heaptype,
                         const wstring&         filename                                    ) {
    m_Device = device;
    m_HeapType = heaptype;
    auto dds_file  = make_unique<ifstream> (filename, std::ios::in | std::ios::binary);
    dds_file->seekg (0, dds_file->end);
    size_t file_size = dds_file->tellg ();
    dds_file->seekg (0, dds_file->beg);

    char magic[4] ={};

    dds_file->read (magic, 4);

    if (!strcmp (magic, "DDS ")) {
        return;
    }

    DDS_HEADER header = {};

    dds_file->read (reinterpret_cast<char*>(&header), sizeof (header));

    if (header.ddspf.dwFlags != 0x41) { //RGB and Alfa
        return;
    }

    int bits    = header.ddspf.dwRGBBitCount;
    int width   = header.dwWidth;
    int height  = header.dwHeight;

    UINT64 data_size = width * height * bits / 8;

    D3DCreateBlob (data_size, &m_Data);
    dds_file->read (reinterpret_cast<char*>(m_Data->GetBufferPointer()), data_size);

    CreateTextureResource( width, height, true, false, 1);
}




FatDx::Texture::Texture (ComPtr<ID3D12Device> device, const D3D12_HEAP_TYPE& heaptype,
                         const int& width, const int& height,
                         const bool& srv, const bool& rtv,
                         const int& samples) {
    if (device == nullptr) {
        return;
    }    
    m_Device = device;
    m_HeapType = heaptype;

    int true_samples = rtv ? samples : 1;
    D3D12_RESOURCE_DESC texResourceDesc     = {};
    {
        texResourceDesc.Alignment           = 0;
        texResourceDesc.DepthOrArraySize    = 1;
        texResourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texResourceDesc.Flags               = rtv   ?   D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET 
                                                    :   D3D12_RESOURCE_FLAG_NONE;
        texResourceDesc.Format              = m_Format;
        texResourceDesc.Width               = width;
        texResourceDesc.Height              = height;
        texResourceDesc.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texResourceDesc.MipLevels           = rtv   ?   1
                                                    :   0;
        texResourceDesc.SampleDesc.Count    = true_samples;
        texResourceDesc.SampleDesc.Quality  = 0;
    }

    D3D12_HEAP_PROPERTIES heapProps         = {};
    {
        heapProps.CPUPageProperty           = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.CreationNodeMask          = 1;
        heapProps.MemoryPoolPreference      = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.Type                      = heaptype;
        heapProps.VisibleNodeMask           = 1;
    }

    D3D12_CLEAR_VALUE optClear              = {};
    {
        optClear.Format                     = m_Format;
    }

    HRESULT res = m_Device->CreateCommittedResource ( &heapProps,
                                                    D3D12_HEAP_FLAG_NONE,
                                                    &texResourceDesc,
                                                    D3D12_RESOURCE_STATE_COMMON,
                                                    rtv ? &optClear
                                                        : nullptr,
                                                    IID_PPV_ARGS (&m_Resource));
    if (FAILED (res)) {
        assert (!"Failed to create texture2d resource");
        return;
    }

    m_HeapType = heaptype;
    m_Description = m_Resource->GetDesc ();

    if (srv) {
        CreateShaderResourceView ();
    }

    if (rtv) {
        CreateRenderTargetView ();
    }
}



void FatDx::Texture::CreateTextureResource (const int & width, const int & height, const bool & srv, const bool & rtv, const int & samples) {
    int true_samples = rtv ? samples : 1;
    D3D12_RESOURCE_DESC texResourceDesc     ={};
    {
        texResourceDesc.Alignment           = 0;
        texResourceDesc.DepthOrArraySize    = 1;
        texResourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texResourceDesc.Flags               = rtv ? D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
                                                  : D3D12_RESOURCE_FLAG_NONE;
        texResourceDesc.Format              = m_Format;
        texResourceDesc.Width               = width;
        texResourceDesc.Height              = height;
        texResourceDesc.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texResourceDesc.MipLevels           = srv ? 0
                                                  : 1;
        texResourceDesc.SampleDesc.Count    = true_samples;
        texResourceDesc.SampleDesc.Quality  = 0;
    }

    D3D12_HEAP_PROPERTIES heapProps         ={};
    {
        heapProps.CPUPageProperty           = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.CreationNodeMask          = 1;
        heapProps.MemoryPoolPreference      = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.Type                      = m_HeapType;
        heapProps.VisibleNodeMask           = 1;
    }

    D3D12_CLEAR_VALUE optClear              ={};
    {
        optClear.Format                     = m_Format;
    }

    HRESULT res = m_Device->CreateCommittedResource (&heapProps,
                                                     D3D12_HEAP_FLAG_NONE,
                                                     &texResourceDesc,
                                                     D3D12_RESOURCE_STATE_COMMON,
                                                     rtv ? &optClear
                                                         : nullptr,
                                                     IID_PPV_ARGS (&m_Resource));
    if (FAILED (res)) {
        assert (!"Failed to create texture2d resource");
        return;
    }

    m_Description = m_Resource->GetDesc ();

    if (srv) {
        CreateShaderResourceView ();
    }

    if (rtv) {
        CreateRenderTargetView ();
    }
}

void FatDx::Texture::CreateRenderTargetView () {
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc ={};
    {
        rtvHeapDesc.Flags           = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask        = 1;
        rtvHeapDesc.NumDescriptors  = 1;
        rtvHeapDesc.Type            = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    }

    HRESULT res = m_Device->CreateDescriptorHeap (&rtvHeapDesc, IID_PPV_ARGS (&m_RtvHeap));
    if (FAILED (res)) {
        assert (!"Failed to create descriptor heap");
        return;
    }

    m_Rtv = m_RtvHeap->GetCPUDescriptorHandleForHeapStart ();
    m_Device->CreateRenderTargetView (m_Resource.Get(), nullptr, m_Rtv);
}



void FatDx::Texture::CreateShaderResourceView () {
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    {
        srvHeapDesc.Flags           = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        srvHeapDesc.NodeMask        = 1;
        srvHeapDesc.NumDescriptors  = 1;
        srvHeapDesc.Type            = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    }

    HRESULT res = m_Device->CreateDescriptorHeap (&srvHeapDesc, IID_PPV_ARGS (&m_SrvHeap));
    if (FAILED (res)) {
        assert (!"Failed to create descriptor heap");
        return;
    }

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	{
		srvDesc.Format = m_Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	}

    m_Srv = m_SrvHeap->GetCPUDescriptorHandleForHeapStart ();
    m_Device->CreateShaderResourceView (m_Resource.Get (), &srvDesc, m_Srv);
}



void FatDx::Texture::UploadTexture (ComPtr<ID3D12GraphicsCommandList> cmdlist) {
    if (m_HeapType == D3D12_HEAP_TYPE_DEFAULT) {
        UploadToDefaultBuffer (cmdlist);
    } else {
        if (m_HeapType == D3D12_HEAP_TYPE_UPLOAD) {
            UploadToDynamicBuffer (cmdlist);
        }
    }   
}

void FatDx::Texture::SetAsRenderTarget (ComPtr<ID3D12GraphicsCommandList> cmdlist) {
    StateTransition (cmdlist, D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void FatDx::Texture::SetAsPixelShaderResource (ComPtr<ID3D12GraphicsCommandList> cmdlist) {
    StateTransition (cmdlist, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

D3D12_CPU_DESCRIPTOR_HANDLE* FatDx::Texture::Rtv () {
    return &m_Rtv;
}

ID3D12DescriptorHeap** FatDx::Texture::RtvHeapAddress () {
    return m_RtvHeap.GetAddressOf ();
}

D3D12_GPU_DESCRIPTOR_HANDLE FatDx::Texture::SrvGpuHandle () {
    return m_SrvHeap->GetGPUDescriptorHandleForHeapStart ();
}

ID3D12DescriptorHeap ** FatDx::Texture::SrvHeapAddress () {
    return m_SrvHeap.GetAddressOf ();
}


void FatDx::Texture::UploadToDefaultBuffer (ComPtr<ID3D12GraphicsCommandList> cmdlist) {
    UINT64 data_size = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    m_Device->GetCopyableFootprints (&m_Description, 0, 1, 0, &footprint, nullptr, nullptr, &data_size);
	m_UploadBuffer = make_unique<Buffer> (m_Device, D3D12_HEAP_TYPE_UPLOAD, data_size);
    BYTE* data_ptr = nullptr;
	m_UploadBuffer->Map (&data_ptr);
    memcpy (data_ptr, m_Data->GetBufferPointer (), data_size);
	m_UploadBuffer->Unmap ();

    StateTransition (cmdlist, D3D12_RESOURCE_STATE_COPY_DEST);

    D3D12_TEXTURE_COPY_LOCATION dest;
    {
        dest.pResource          = m_Resource.Get ();
        dest.Type               = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dest.SubresourceIndex   = 0;
    }

    D3D12_TEXTURE_COPY_LOCATION src;
    {
        src.pResource           = m_UploadBuffer->Get ();
        src.Type                = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        src.PlacedFootprint     = footprint;
    }
    cmdlist->CopyTextureRegion (&dest, 0, 0, 0, &src, nullptr);
}



void FatDx::Texture::UploadToDynamicBuffer (ComPtr<ID3D12GraphicsCommandList> cmdlist) {
    UINT64 data_size = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    m_Device->GetCopyableFootprints (&m_Description, 0, 1, 0, &footprint, nullptr, nullptr, &data_size);
    void* data_ptr;
    m_Resource->Map (0, nullptr, &data_ptr);
    memcpy (data_ptr, m_Data->GetBufferPointer(), data_size);
    m_Resource->Unmap (0, nullptr);
}

