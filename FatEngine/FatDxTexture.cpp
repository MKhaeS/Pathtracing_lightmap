#include "FatDXFramework.h"



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



