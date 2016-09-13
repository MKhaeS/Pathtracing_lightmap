#include "FatDXFramework.h"


bool FatDXFramework::Initialize( const HWND&     hWnd,
								 const int&     width, const int& height,
								 const DXGI_FORMAT& buffer_format ) {
	m_hWnd_			    =	hWnd;
	m_FenceEvent_	    =	CreateEvent ( nullptr, FALSE, FALSE, nullptr );

	m_ClientWidth_	    =	width;
	m_ClientHeight_     =	height;
    bool init_result =   false;

    init_result =    CreateFactory();
	                 NumerateAdapters();
    init_result &=   CreateDeviceFromAdapter( m_SystemAdapters_[0] );
    init_result &=   CreateFence();
	                 AssignDescriptorSizes();
    init_result &=   CreateCommandStructure();
    init_result &=   CreateSwapChain( width, height, buffer_format, 2 );
    init_result &=   CreateDescriptorHeaps();
    init_result &=   CreateRenderTargetView();
    init_result &=   CreateDepthStencilView();
	                 SetViewport( 0.0f, 0.0f, static_cast<float>(m_ClientWidth_), static_cast<float>(m_ClientHeight_) );

	return init_result;
}



bool FatDXFramework::CreateFactory() {
	HRESULT res = CreateDXGIFactory2( DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS  ( &m_Factory_ ) );
	if ( FAILED( res ) ) {
        assert( !"Failed to create DXGI factory" );
        return false;
	}
    return true;
}



void FatDXFramework::NumerateAdapters() {
    if ( m_Factory_ != nullptr ) {
        ComPtr<IDXGIAdapter1> temp_adapter;
        UINT i = 0;

        while ( true ) {
            HRESULT res = m_Factory_->EnumAdapters1( i, &temp_adapter );
            if ( res != DXGI_ERROR_NOT_FOUND ) {
                m_SystemAdapters_.push_back( temp_adapter );
                ++i;
            } else {
                return;
            }
        }
    }
}



bool FatDXFramework::CreateDeviceFromAdapter( ComPtr<IDXGIAdapter1> noAdapter ) {
    if ( noAdapter != nullptr ) {
        HRESULT res = D3D12CreateDevice( noAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS  ( &m_Device ) );

        if ( FAILED ( res ) ) {
            assert( !"Failed to create D3D12 device" );
            return false;
        }

        return true;
    }
    return false;
}



bool FatDXFramework::CreateFence() {
    if ( m_Device != nullptr ) {
        HRESULT res = m_Device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS  ( &m_Fence_ ) );
        if ( FAILED( res ) ) {
            assert( !"Failed to create device fence" );
            return false;
        }
        return true;
    }    
    return false;
}



void FatDXFramework::AssignDescriptorSizes() {
    if ( m_Device != nullptr ) {
        m_uSizeRtv_    = m_Device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
        m_uSizeDsv_    = m_Device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_DSV );
        m_uSizeCbvSrv_ = m_Device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
    }
}



bool FatDXFramework::CreateCommandStructure() {
    if ( m_Device != nullptr ) {

        D3D12_COMMAND_QUEUE_DESC    queueDesc = {};
        queueDesc.Type              = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags             = D3D12_COMMAND_QUEUE_FLAG_NONE;


        HRESULT res = m_Device->CreateCommandQueue( &queueDesc, 
                                                     IID_PPV_ARGS  ( &m_CommandQueue_ ) );
        if ( FAILED ( res ) ) {
            assert( !"Failed to create command queue" );
            return false;
        }
        res = m_Device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                 IID_PPV_ARGS  ( &m_CommandAllocator_ ) );
        if ( FAILED ( res ) ) {
            assert( !"Failed to create command allocator" );
            return false;
        }

        res = m_Device->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                            m_CommandAllocator_.Get(),
                                            nullptr,
                                            IID_PPV_ARGS  ( &m_CommandList_ ) );
        if ( FAILED ( res ) ) {
            assert( !"Failed to create command list" );
            return false;
        }
        m_CommandList_->Close();
        m_CommandAllocator_->Reset ();
        m_CommandList_->Reset ( m_CommandAllocator_.Get (), nullptr );
        return true;
    }
    return false;
}



bool FatDXFramework::CreateSwapChain( const int& width, const int& height,
                                      const DXGI_FORMAT& buffer_format,
                                      const int& nBuffers ) {
    if ( m_Factory_ != nullptr && m_Device != nullptr ) {
        m_SwapChain_.Reset();
        // Maximum samples count here is 1 due to 
        //      https://msdn.microsoft.com/ru-ru/library/windows/desktop/bb173077(v=vs.85).aspx
        // To use multisampling, set the appropriate RT texture
        // This version supports only _FLIP_ swap effects
        // With non _FLIP_ effect in earlier versions you were able to read and write only in buffer(0)
        m_SampleDescription_.Count            = 1;
        m_SampleDescription_.Quality          = 0;

        m_SwapChainDescription_.AlphaMode     = DXGI_ALPHA_MODE_UNSPECIFIED;
        m_SwapChainDescription_.Format        = buffer_format;
        m_SwapChainDescription_.Width         = width;
        m_SwapChainDescription_.Height        = height;
        m_SwapChainDescription_.Scaling       = DXGI_SCALING_NONE;
        m_SwapChainDescription_.Stereo        = FALSE;
        m_SwapChainDescription_.SampleDesc    = m_SampleDescription_;
        m_SwapChainDescription_.BufferUsage   = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        m_SwapChainDescription_.BufferCount   = nBuffers;
        m_SwapChainDescription_.SwapEffect    = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        m_SwapChainDescription_.Flags         = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        ComPtr<IDXGISwapChain1> tempSwapChain;
        HRESULT res = m_Factory_->CreateSwapChainForHwnd( m_CommandQueue_.Get(),
                                                          m_hWnd_,
                                                          &m_SwapChainDescription_,
                                                          nullptr, nullptr,
                                                          tempSwapChain.GetAddressOf() );
        if ( FAILED( res ) ) {
            assert( !"Failed to create swap chain" );
            return false;
        }
        m_SwapChain_ = static_cast<IDXGISwapChain3*>(tempSwapChain.Get());
        m_nSwapChainBuffers_ = nBuffers;
        for ( int i = 0; i < nBuffers; ++i ) {
            ComPtr<ID3D12Resource> temp_buffer;
            m_SwapChain_->GetBuffer( i, IID_PPV_ARGS( &temp_buffer ) );
            m_SwapChainBuffers_.push_back( temp_buffer );
            m_ClientWidth_   = width;
            m_ClientHeight_  = height;
        }
        m_CurrentBuffer_ = m_SwapChain_->GetCurrentBackBufferIndex();
        return true;
    }
    return false;
}



bool FatDXFramework::CreateDescriptorHeaps() {
    if ( m_Device != nullptr ) {
        D3D12_DESCRIPTOR_HEAP_DESC      rtvHeapDesc;
        rtvHeapDesc.Type                = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags               = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NumDescriptors      = m_nSwapChainBuffers_;
        rtvHeapDesc.NodeMask            = 0;

        HRESULT res = m_Device->CreateDescriptorHeap( &rtvHeapDesc, IID_PPV_ARGS  ( &m_RtvHeap_ ) );
        if ( FAILED( res ) ) {
            assert( !"Failed to create RTV descriptor heap" );
            return false;
        }

        D3D12_DESCRIPTOR_HEAP_DESC      dsvHeapDesc;
        dsvHeapDesc.Type                = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags               = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NumDescriptors      = 1;
        dsvHeapDesc.NodeMask            = 0;

        res = m_Device->CreateDescriptorHeap( &dsvHeapDesc, IID_PPV_ARGS  ( &m_DsvHeap_ ) );
        if ( FAILED( res ) ) {
            assert( !"Failed to create DSV descriptor heap" );
            return false;
        }
        return true;
    }
    return false;
}



bool FatDXFramework::CreateRenderTargetView() {
    if ( m_Device != nullptr ) {
        m_RtvHeapHandle_ = m_RtvHeap_->GetCPUDescriptorHandleForHeapStart();
        D3D12_CPU_DESCRIPTOR_HANDLE temp_handle = m_RtvHeapHandle_;
        for ( auto buf : m_SwapChainBuffers_ ) {
            m_Device->CreateRenderTargetView( buf.Get(), nullptr, temp_handle );
            temp_handle.ptr += m_uSizeRtv_;
        }
        return true;
    }
    assert( !"Failed to create RTV. Device is NULL" );
    return false;
}



bool FatDXFramework::CreateDepthStencilView() {
    if ( m_Device != nullptr ) {
        m_DsvHeapHandle_ = m_DsvHeap_->GetCPUDescriptorHandleForHeapStart();
        D3D12_RESOURCE_DESC                 depthStencilDesc;
        depthStencilDesc.Alignment          = 0;
        depthStencilDesc.DepthOrArraySize   = 1;
        depthStencilDesc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        depthStencilDesc.Format             = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.Width              = m_ClientWidth_;
        depthStencilDesc.Height             = m_ClientHeight_;
        depthStencilDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        depthStencilDesc.MipLevels          = 1;
        depthStencilDesc.SampleDesc         = m_SampleDescription_;

        D3D12_CLEAR_VALUE                   optClear;
        optClear.DepthStencil.Depth         = 1.0f;
        optClear.DepthStencil.Stencil       = 0;
        optClear.Format                     = DXGI_FORMAT_D24_UNORM_S8_UINT;

        D3D12_HEAP_PROPERTIES               heapProps;
        heapProps.CPUPageProperty           = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.CreationNodeMask          = 1;
        heapProps.MemoryPoolPreference      = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.Type                      = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.VisibleNodeMask           = 1;

        HRESULT res = m_Device->CreateCommittedResource( &heapProps,
                                                          D3D12_HEAP_FLAG_NONE,
                                                          &depthStencilDesc,
                                                          D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                          &optClear,
                                                          IID_PPV_ARGS  ( &m_DepthStencilBuffer_ ) );
        if ( FAILED( res ) ) {
            assert( !"Failed to create depth stencil view" );
            return false;
        }
        m_Device->CreateDepthStencilView( m_DepthStencilBuffer_.Get(),
                                           nullptr,
                                           m_DsvHeapHandle_ );
        return true;
    }
    assert( !"Failed to create DSV. Device is NULL" );
    return false;
}



void FatDXFramework::SetViewport( const float & top_left_x, const float & top_left_y,
                                  const float & width,      const float & height ) {
    m_Viewport_.TopLeftX    = top_left_x;
    m_Viewport_.TopLeftY    = top_left_y;
    m_Viewport_.Width       = width;
    m_Viewport_.Height      = height;
    m_Viewport_.MinDepth    = 0.0f;
    m_Viewport_.MaxDepth    = 1.0f;
    m_CommandList_->Close ();
    ComPtr<ID3D12CommandList> new_cmdsList[] = { m_CommandList_.Get () };
    m_CommandQueue_->ExecuteCommandLists ( _countof ( new_cmdsList ), new_cmdsList->GetAddressOf () );
    m_CommandQueue_->Signal ( m_Fence_.Get (), m_FenceValue_ );
    WaitSignal();
}