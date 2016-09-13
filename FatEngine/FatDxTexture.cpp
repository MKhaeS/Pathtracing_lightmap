#include <fstream>


#include "FatDXFramework.h"
#include "Dds.h"


using namespace std;


int FatDXFramework::CreateTexture2D( const int & width, const int & height, const int & samples, const DXGI_FORMAT & format ) {

    D3D12_RESOURCE_DESC texResourceDesc = {};
    {
        texResourceDesc.Alignment = 0;
        texResourceDesc.DepthOrArraySize = 1;
        texResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        texResourceDesc.Format = format;
        texResourceDesc.Width = width;
        texResourceDesc.Height = height;
        texResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texResourceDesc.MipLevels = 1;
        texResourceDesc.SampleDesc.Count = samples;
        texResourceDesc.SampleDesc.Quality = 0;
    }

    D3D12_HEAP_PROPERTIES               heapProps = {};
    {
        heapProps.CPUPageProperty           = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.CreationNodeMask          = 1;
        heapProps.MemoryPoolPreference      = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.Type                      = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.VisibleNodeMask           = 1;
    }

    ComPtr<ID3D12Resource> texture_resource;

    HRESULT res = m_Device->CreateCommittedResource( &heapProps,
                                                     D3D12_HEAP_FLAG_NONE,
                                                     &texResourceDesc,
                                                     D3D12_RESOURCE_STATE_COMMON,
                                                     nullptr,
                                                     IID_PPV_ARGS  ( &texture_resource ) );
    if ( FAILED( res ) ) {
        assert( !"Failed to create texture2d resource" );
        return -1;
    }

    auto newTexture = std::make_unique<Texture2D>();
    {
        newTexture->Resource = texture_resource;
    }
    m_Textures.push_back( std::move( newTexture ) );
    ++m_nTextures;

    return (m_nTextures - 1);
}

bool FatDXFramework::CreateRtvForTexture( const int & texture ) {
    if ( m_Textures.size() < texture + 1 )
        return false;
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    {
        rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NodeMask = 1;
        rtvHeapDesc.NumDescriptors = 1;
        rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    }
    ComPtr<ID3D12DescriptorHeap> rtvDescHeap;
    HRESULT res = m_Device->CreateDescriptorHeap( &rtvHeapDesc, IID_PPV_ARGS( &rtvDescHeap ) );
    if ( FAILED( res ) ) {
        assert( !"Failed to create descriptor heap" );
        return false;
    }

    m_Textures[texture]->RtvDescriptorHeap = rtvDescHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    m_Device->CreateRenderTargetView( m_Textures[texture]->Resource.Get(), nullptr, rtvHandle );
    m_Textures[texture]->RTV = rtvHandle;
    return true;
}

bool FatDXFramework::CreateSrvForTexture( const int & texture ) {
	if ( m_Textures.size() < texture + 1 )
		return false;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	{
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		srvHeapDesc.NodeMask = 1;
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;        
	}
	ComPtr<ID3D12DescriptorHeap> srvDescHeap;
	HRESULT res = m_Device->CreateDescriptorHeap( &srvHeapDesc, IID_PPV_ARGS( &srvDescHeap ) );
	if ( FAILED( res ) ) {
		assert( !"Failed to create descriptor heap" );
		return false;
	}

	m_Textures[texture]->SrvDescriptorHeap = srvDescHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = srvDescHeap->GetCPUDescriptorHandleForHeapStart();

	m_Device->CreateShaderResourceView( m_Textures[texture]->Resource.Get(), nullptr, srvHandle );
	m_Textures[texture]->SRV = srvHandle;
	return true;
}

void FatDXFramework::LoadTextureFromDds (int texture, const std::wstring & filename) {
	ifstream dds_file (filename, std::ios::in | std::ios::binary);

	dds_file.seekg (0, dds_file.end);
	int size = dds_file.tellg ();
	dds_file.seekg (0, dds_file.beg);

	char magic[4] ={};

	dds_file.read (magic, 4);

	if (!strcmp (magic, "DDS ")) { 
		return;
	}

	DDS_HEADER header ={};

	dds_file.read (reinterpret_cast<char*>(&header), sizeof (header));

	if (header.ddspf.dwFlags != 0x41) { //RGB and Alfa
		return;
	}

	int bits = header.ddspf.dwRGBBitCount;
	int width = header.dwWidth;
	int height = header.dwHeight;

	UINT64 blobsize = width*height * bits / 8;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    m_Device->GetCopyableFootprints (&(m_Textures[texture]->Resource->GetDesc ()), 0, 1, 0, &footprint, nullptr, nullptr, &blobsize);

    ComPtr<ID3D12Resource> dds_buffer = CreateUploadBuffer (blobsize);
    D3D12_RESOURCE_DESC desc = dds_buffer->GetDesc ();
    BYTE* dds_data;
    //
	

    
    HRESULT res = dds_buffer->Map (0, nullptr, reinterpret_cast<void**> (&dds_data));

    dds_file.read (reinterpret_cast<char*>(dds_data), blobsize);
    dds_buffer->Unmap (0, nullptr );
	dds_file.close ();

    ResourceStateTransition (m_Textures[texture]->Resource, m_Textures[texture]->State, D3D12_RESOURCE_STATE_COPY_DEST);
    m_Textures[texture]->State = D3D12_RESOURCE_STATE_COPY_DEST;

    D3D12_TEXTURE_COPY_LOCATION dest;
    {
        dest.pResource = m_Textures[texture]->Resource.Get();
        dest.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dest.SubresourceIndex = 0;
        
    }

    D3D12_TEXTURE_COPY_LOCATION src;
    {
        src.pResource = dds_buffer.Get();
        src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        src.PlacedFootprint = footprint;
    }

    //    m_CommandList_->CopyResource (m_Textures[texture]->Resource.Get (), dds_buffer.Get());
    m_CommandList_->CopyTextureRegion (&dest, 0, 0, 0, &src, nullptr);

    ResourceStateTransition (m_Textures[texture]->Resource, m_Textures[texture]->State, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    m_Textures[texture]->State = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    m_CommandList_->Close ();
    ComPtr<ID3D12CommandList> new_cmdsList[] ={ m_CommandList_.Get () };
    m_CommandQueue_->ExecuteCommandLists (_countof (new_cmdsList), new_cmdsList->GetAddressOf ());
    m_CommandQueue_->Signal (m_Fence_.Get (), m_FenceValue_);
    WaitSignal ();

    return;
}





