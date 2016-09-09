#include "FatDXFramework.h"
#pragma once




FatDXFramework::FatDXFramework() {
#if defined(DEBUG) || defined(_DEBUG)
    {
        D3D12GetDebugInterface( IID_PPV_ARGS  ( &m_DebugController_ ) );
        m_DebugController_->EnableDebugLayer();
        m_bDebugLayerActive_ = true;
    }
#endif    
}



bool FatDXFramework::CreateFactory() {
    HRESULT res = CreateDXGIFactory2( DXGI_CREATE_FACTORY_DEBUG,  IID_PPV_ARGS  ( &m_Factory_ ) );
    if ( SUCCEEDED( res ) ) {
        return true;
    }
    assert( !"Failed to create DXGI factory" );
    return false;
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
            }
            else {
                return;
            }
        }
    }
}



bool FatDXFramework::CreateDeviceFromAdapter( ComPtr<IDXGIAdapter1> noAdapter ) {
    if ( noAdapter != nullptr ) {
        HRESULT res = D3D12CreateDevice( noAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS  ( &m_Device_ ) );
        if ( SUCCEEDED( res ) ) {

            //
            DXGI_ADAPTER_DESC adapterDesc;
            noAdapter->GetDesc(&adapterDesc);
            std::wstring w_desc = adapterDesc.Description;
            char* c_desc = new char[w_desc.size() + 1];
            size_t sz;
            wcstombs_s(&sz, c_desc, (size_t)(w_desc.size() + 1),  w_desc.c_str(), _TRUNCATE);
            std::string s_desc = c_desc;
            //

            return true;
        }
    }
    assert( !"Failed to create D3D12 device" );
    return false;
}



bool FatDXFramework::CreateFence() {
    if ( m_Device_ != nullptr ) {
        HRESULT res = m_Device_->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS  ( &m_Fence_ ) );
        if ( SUCCEEDED( res ) ) {
            return true;
        }
    }
    assert( !"Failed to create device fence");
    return false;
}



void FatDXFramework::AssignDescriptorSizes() {
    if ( m_Device_ != nullptr ) {
        m_uSizeRtv_    = m_Device_->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
        m_uSizeDsv_    = m_Device_->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_DSV );
        m_uSizeCbvSrv_ = m_Device_->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
    }
}



bool FatDXFramework::CreateCommandStructure()
{
    if ( m_Device_ != nullptr ) {

        D3D12_COMMAND_QUEUE_DESC    queueDesc = { };
        queueDesc.Type              = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Flags             = D3D12_COMMAND_QUEUE_FLAG_NONE;
        

        HRESULT res = m_Device_->CreateCommandQueue( &queueDesc, IID_PPV_ARGS  ( &m_CommandQueue_ ) );
        if ( SUCCEEDED( res ) ) {
            res = m_Device_->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                                     IID_PPV_ARGS  ( &m_CommandAllocator_ ) );
            if ( SUCCEEDED( res ) ) {
                res = m_Device_->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                                    m_CommandAllocator_.Get(), 
                                                    nullptr, 
                                                    IID_PPV_ARGS  ( &m_CommandList_ ) );
                if ( SUCCEEDED( res ) ) {
                    m_CommandList_->Close();
                    return true;
                }
            }
        }
    }
    assert( !"Failed to create command structure" );
    return false;
}



bool FatDXFramework::CreateSwapChain( const int& width, const int& height,
                                      const DXGI_FORMAT& buffer_format,
                                      const int& nBuffers ) {
    if ( m_Factory_ != nullptr && m_Device_ != nullptr ) {
        m_SwapChain_.Reset();
        // Maximum samples count here is 1 due to 
        //      https://msdn.microsoft.com/ru-ru/library/windows/desktop/bb173077(v=vs.85).aspx
        // To use multisampling, set the appropriate RTV
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

        if ( SUCCEEDED( res ) ) {
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

    }
    assert( !"Failed to create swap chain" );
    return false;
}



bool FatDXFramework::CreateDescriptorHeaps() {
    if ( m_Device_ != nullptr ) {
        D3D12_DESCRIPTOR_HEAP_DESC      rtvHeapDesc;
        rtvHeapDesc.Type                = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDesc.Flags               = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvHeapDesc.NumDescriptors      = m_nSwapChainBuffers_;
        rtvHeapDesc.NodeMask            = 0;

        HRESULT res = m_Device_->CreateDescriptorHeap( &rtvHeapDesc, IID_PPV_ARGS  ( &m_RtvHeap_ ) );
        if ( FAILED( res ) ) {
            assert( !"Failed to create RTV descriptor heap" );
            return false;
        }

        D3D12_DESCRIPTOR_HEAP_DESC      dsvHeapDesc;
        dsvHeapDesc.Type                = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsvHeapDesc.Flags               = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dsvHeapDesc.NumDescriptors      = 1;
        dsvHeapDesc.NodeMask            = 0;

        res = m_Device_->CreateDescriptorHeap( &dsvHeapDesc, IID_PPV_ARGS  ( &m_DsvHeap_ ) );
        if ( SUCCEEDED( res ) ) {            
            return true;
        }
    }
    assert( !"Failed to create DSV descriptor heap" );
    return false;
}



bool FatDXFramework::CreateRenderTargetView() {
    if ( m_Device_ != nullptr ) {
        m_RtvHeapHandle_ = m_RtvHeap_->GetCPUDescriptorHandleForHeapStart();
        D3D12_CPU_DESCRIPTOR_HANDLE temp_handle = m_RtvHeapHandle_;
        for ( auto buf : m_SwapChainBuffers_ )
        {
            m_Device_->CreateRenderTargetView( buf.Get(), nullptr, temp_handle );
            temp_handle.ptr += m_uSizeRtv_;
        }
        return true;
    }
    assert( !"Failed to create render target view" );
    return false;
}


bool FatDXFramework::CreateDepthStencilView() {
    if ( m_Device_ != nullptr ) {
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

        HRESULT res = m_Device_->CreateCommittedResource( &heapProps,
                                                          D3D12_HEAP_FLAG_NONE,
                                                          &depthStencilDesc,
                                                          D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                          &optClear,
                                                          IID_PPV_ARGS  ( &m_DepthStencilBuffer_ ) );
        if ( SUCCEEDED( res ) ) {
            m_Device_->CreateDepthStencilView( m_DepthStencilBuffer_.Get(),
                                               nullptr,
                                               m_DsvHeapHandle_ );
            return true;
        }
    }
    assert( !"Failed to create depth stencil view" );
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
}



void FatDXFramework::NumerateOutputsForDevice(const int& noDevice)
{
	UINT i = 0;
	ComPtr<IDXGIOutput> temp_output;

}	



bool FatDXFramework::Initialize(const HWND&     hWnd,
                                const int&     width, const int& height,
	                            const DXGI_FORMAT& buffer_format)
{
    m_hWnd_ = hWnd;
    m_FenceEvent_ = CreateEvent (nullptr, FALSE, FALSE, nullptr);

    m_ClientWidth_ = width;
    m_ClientHeight_ = height;

	CreateFactory();
	NumerateAdapters();
	CreateDeviceFromAdapter(m_SystemAdapters_[0]);
	CreateFence();
	AssignDescriptorSizes();
	CreateCommandStructure();
	CreateSwapChain(width, height, buffer_format, 2);
	CreateDescriptorHeaps();
	CreateRenderTargetView();
    CreateDepthStencilView();
    SetViewport( 0.0f, 0.0f, static_cast<float>(m_ClientWidth_), static_cast<float>(m_ClientHeight_));


    
	XMFLOAT3 v[] = { { -1.0f, 1.0f, 0.1f }, { -1.0f, -1.0f, 0.1f }, { 1.0f, 1.0f, 0.1f }};

    UINT64 size = 3 * sizeof (XMFLOAT3);
    
    m_CommandAllocator_->Reset ();
    m_CommandList_->Reset (m_CommandAllocator_.Get (), nullptr);
    ComPtr<ID3D12Resource> uploaBuffer = nullptr;
    m_VertBuffer_ = LoadDataToGpu (v, size, uploaBuffer);
    m_VBview_.BufferLocation = m_VertBuffer_->GetGPUVirtualAddress ();
    m_VBview_.SizeInBytes = size;
    m_VBview_.StrideInBytes = sizeof (XMFLOAT3);
    m_CommandList_->Close ();
    ComPtr<ID3D12CommandList> cmdsList[] = { m_CommandList_.Get () };
    m_CommandQueue_->ExecuteCommandLists (_countof (cmdsList), cmdsList->GetAddressOf ());
    HRESULT hr = m_CommandQueue_->Signal (m_Fence_.Get (), m_FenceValue_);
    WaitSignal ();
    
	return true;
}




ComPtr<ID3D12Resource> FatDXFramework::LoadDataToGpu ( const void * pVertexData, const UINT64& dataSize,
                                                           ComPtr<ID3D12Resource>& uploadBuffer ) {
    // Important to keep pointer to uploadBuffer after the function returns
    // untill GPU stops executing after m_CommandQueue_->ExecuteCommandLists call

    ComPtr<ID3D12Resource> gpuBuffer = CreateGpuBuffer ( dataSize );
                        uploadBuffer = CreateUploadBuffer ( dataSize );
    ChangeResourceState ( gpuBuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST );      
    CopyDataToBuffer ( gpuBuffer, uploadBuffer, pVertexData, dataSize );
    ChangeResourceState ( gpuBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ );

    return gpuBuffer;
}



bool FatDXFramework::CopyDataToBuffer ( const ComPtr<ID3D12Resource>& destination,
                                      const ComPtr<ID3D12Resource>& upload, 
                                      const void * pDataSource, 
                                      const UINT64 & dataSize ) const {
    D3D12_RESOURCE_DESC uploadDesc          = upload->GetDesc();
    D3D12_RESOURCE_DESC destinationDesc     = destination->GetDesc();
    if ( uploadDesc.Dimension       != D3D12_RESOURCE_DIMENSION_BUFFER   ||
         destinationDesc.Dimension  != D3D12_RESOURCE_DIMENSION_BUFFER   ||
         uploadDesc.Width           <  dataSize)
    {
        assert ( !"Resources are not buffers or have invalid size" );
        return false;
    }
    //Map the upload resource memory to host memory and copy the appropriate data
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



ComPtr<ID3D12Resource> FatDXFramework::CreateGpuBuffer ( const UINT64 & bufferSize ) {
    // This buffer represents real allocated memory on GPU
    ComPtr<ID3D12Resource> gpuBuffer               = nullptr;
    D3D12_HEAP_PROPERTIES                          defHeapProperties;
    defHeapProperties.Type                         = D3D12_HEAP_TYPE_DEFAULT;
    defHeapProperties.CPUPageProperty              = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    defHeapProperties.MemoryPoolPreference         = D3D12_MEMORY_POOL_UNKNOWN;
    defHeapProperties.CreationNodeMask             = 1;
    defHeapProperties.VisibleNodeMask              = 1;

    D3D12_RESOURCE_DESC                             defResourceDescription;
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

    HRESULT res = m_Device_->CreateCommittedResource ( &defHeapProperties, D3D12_HEAP_FLAG_NONE,
                                                       &defResourceDescription, D3D12_RESOURCE_STATE_COMMON,
                                                       nullptr, IID_PPV_ARGS ( &gpuBuffer ) );
    if ( FAILED ( res ) ) {
        assert ( !"Failed to create GPU buffer resource" );
    }
    return gpuBuffer;
}



ComPtr<ID3D12Resource> FatDXFramework::CreateUploadBuffer ( const UINT64 & bufferSize ) {
    // This buffer represents CPU memory which is mediator for uploading data into GPU memory
    ComPtr<ID3D12Resource>                            uploadBuffer;
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

    HRESULT res  = m_Device_->CreateCommittedResource ( &uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
                                                &uploadResourceDescription, D3D12_RESOURCE_STATE_GENERIC_READ,
                                                nullptr, IID_PPV_ARGS ( &uploadBuffer ) );
    if ( FAILED ( res ) ) {
        assert ( !"Failed to create upload buffer resource" );
    }

    return uploadBuffer;
}



void FatDXFramework::ChangeResourceState ( const ComPtr<ID3D12Resource>& resource, 
                                                   D3D12_RESOURCE_STATES prev_state,
                                                   D3D12_RESOURCE_STATES next_state ) {
    D3D12_RESOURCE_BARRIER                           copyBarrier = { };
    copyBarrier.Type                                 = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    copyBarrier.Flags                                = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    copyBarrier.Transition.pResource                 = resource.Get ( );
    copyBarrier.Transition.StateBefore               = prev_state;
    copyBarrier.Transition.StateAfter                = next_state;
    copyBarrier.Transition.Subresource               = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_CommandList_->ResourceBarrier ( 1, &copyBarrier );
}


void FatDXFramework::BuildPSO () {
    D3D12_INPUT_ELEMENT_DESC vertDesc = {};
    {
        vertDesc.SemanticName = "POSITION";
        vertDesc.SemanticIndex = 0;
        vertDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        vertDesc.AlignedByteOffset = 0;
        vertDesc.InputSlot = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        vertDesc.InstanceDataStepRate = 0;
    }
    D3D12_INPUT_LAYOUT_DESC layoutDesc = {};
    {
        layoutDesc.pInputElementDescs = &vertDesc;
        layoutDesc.NumElements = 1;
    }
    D3D12_RASTERIZER_DESC rastDesc = {};
    {
        rastDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rastDesc.CullMode = D3D12_CULL_MODE_NONE;
        rastDesc.FrontCounterClockwise = FALSE;
        rastDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rastDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rastDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rastDesc.DepthClipEnable = TRUE;
        rastDesc.AntialiasedLineEnable = FALSE;
        rastDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        rastDesc.MultisampleEnable = FALSE;
        rastDesc.ForcedSampleCount = 1;
    }

    ComPtr<ID3DBlob> vsBinary = 
        LoadBinary (L"C:\\Users\\khaes\\Documents\\Visual Studio 2015\\Projects\\FatEngine\\x64\\Debug\\VS.cso");
    ComPtr<ID3DBlob> psBinary = 
        LoadBinary (L"C:\\Users\\khaes\\Documents\\Visual Studio 2015\\Projects\\FatEngine\\x64\\Debug\\PS.cso");

    D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
    {
        rsDesc.NumParameters = 0;
        rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    }

    ComPtr<ID3DBlob> serializedRS;
    ComPtr<ID3D10Blob> errorBlob;

    HRESULT res = D3D12SerializeRootSignature (&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRS, &errorBlob);    
    m_Device_->CreateRootSignature (0, serializedRS->GetBufferPointer (), serializedRS->GetBufferSize (), IID_PPV_ARGS (&m_RootSignature_));

    D3D12_RENDER_TARGET_BLEND_DESC rtblendDesc{
        FALSE, FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    D3D12_BLEND_DESC blendDesc = {};
    {
        blendDesc.RenderTarget[0] = rtblendDesc;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_RootSignature_.Get ();
    psoDesc.VS = { reinterpret_cast<BYTE*>(vsBinary->GetBufferPointer ()), vsBinary->GetBufferSize () };
    psoDesc.PS = { reinterpret_cast<BYTE*>(psBinary->GetBufferPointer ()), psBinary->GetBufferSize () };
    psoDesc.BlendState = blendDesc;
    psoDesc.RasterizerState = rastDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.InputLayout = layoutDesc;

    m_Device_->CreateGraphicsPipelineState (&psoDesc, IID_PPV_ARGS (&m_Pso_));
}

ComPtr<ID3DBlob> FatDXFramework::LoadBinary ( const std::wstring & filename ) {
    std::ifstream fin ( filename, std::ios::binary );
    fin.seekg ( 0, fin.end );
    int size = fin.tellg ( );
    fin.seekg ( 0, fin.beg );
    ComPtr<ID3DBlob> blob;
    HRESULT res = D3DCreateBlob ( size, blob.GetAddressOf ( ) );

    fin.read ( reinterpret_cast<char*>(blob->GetBufferPointer ( )), size );
    fin.close ( );
    return blob;
}

void FatDXFramework::Update ()
{
    //int delta_time = t0.Tick ();
    Render ();
}

int FatDXFramework::CreateRootSignature () {
    D3D12_ROOT_SIGNATURE_DESC rsDesc ={};
    {
        rsDesc.NumParameters = 0;
        rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    }

    ComPtr<ID3DBlob> serializedRS;
    ComPtr<ID3D10Blob> errorBlob;
    ComPtr<ID3D12RootSignature> newSignature;
    HRESULT res = D3D12SerializeRootSignature (&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                               &serializedRS, &errorBlob);
    if (FAILED (res)) {
        return -1;
    }
    res = m_Device_->CreateRootSignature (0, serializedRS->GetBufferPointer (),
                                          serializedRS->GetBufferSize (), IID_PPV_ARGS (&newSignature));
    if (FAILED (res)) {
        return -1;
    }

    m_RootSignatures_.push_back (newSignature);
    ++m_nRootSignatures_;

    return (m_nRootSignatures_ - 1);
}

int FatDXFramework::CreateVertexShaderFromCso (const std::wstring & filename) {
    std::ifstream fin (filename, std::ios::binary);
    fin.seekg (0, fin.end);
    int size = fin.tellg ();
    fin.seekg (0, fin.beg);
    ComPtr<ID3DBlob> blob;
    HRESULT res = D3DCreateBlob (size, blob.GetAddressOf ());

    fin.read (reinterpret_cast<char*>(blob->GetBufferPointer ()), size);
    fin.close ();
    m_VertexShaders_.push_back (blob);
    ++m_nVertexShaders_;
    return (m_nVertexShaders_ - 1);
}

int FatDXFramework::CreatePixelShaderFromCso (const std::wstring & filename) {
    std::ifstream fin (filename, std::ios::binary);
    fin.seekg (0, fin.end);
    int size = fin.tellg ();
    fin.seekg (0, fin.beg);
    ComPtr<ID3DBlob> blob;
    HRESULT res = D3DCreateBlob (size, blob.GetAddressOf ());

    fin.read (reinterpret_cast<char*>(blob->GetBufferPointer ()), size);
    fin.close ();
    m_PixelShaders_.push_back (blob);
    ++m_nPixelShaders_;
    return (m_nPixelShaders_ - 1);
}

int FatDXFramework::CreateRasterizerDescription (const D3D12_FILL_MODE& fillmode,
                                                 const D3D12_CULL_MODE& cullmode,
                                                 const bool& multisample,
                                                 const int& nsamples) {
    D3D12_RASTERIZER_DESC rastDesc ={};
    {
        rastDesc.FillMode = fillmode;
        rastDesc.CullMode = cullmode;
        rastDesc.FrontCounterClockwise = FALSE;
        rastDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        rastDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        rastDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rastDesc.DepthClipEnable = TRUE;
        rastDesc.AntialiasedLineEnable = FALSE;
        rastDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        rastDesc.MultisampleEnable = multisample;
        rastDesc.ForcedSampleCount = nsamples;
    }
    m_RasterizerDescs_.push_back (rastDesc);
    ++m_nRasterizerDescs;

    return (m_nRasterizerDescs - 1);
}

int FatDXFramework::CreatePipelineStateObject (const int & rootsignature, 
                                               const int & vshader,
                                               const int & pshader,
                                               const int & rasterizer, 
                                               const int & layout,
                                               const D3D12_PRIMITIVE_TOPOLOGY_TYPE& topology) {
    D3D12_RENDER_TARGET_BLEND_DESC rtblendDesc{
        FALSE, FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL,
    };

    D3D12_BLEND_DESC blendDesc ={};
    {
        blendDesc.RenderTarget[0] = rtblendDesc;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc ={};
    psoDesc.pRootSignature = m_RootSignatures_[rootsignature].Get();
    psoDesc.VS = { 
        reinterpret_cast<BYTE*>(m_VertexShaders_[vshader]->GetBufferPointer ()), 
                                m_VertexShaders_[vshader]->GetBufferSize () };
    psoDesc.PS = { 
        reinterpret_cast<BYTE*>(m_PixelShaders_[pshader]->GetBufferPointer ()),
                                m_PixelShaders_[pshader]->GetBufferSize () };
    psoDesc.BlendState = blendDesc;
    psoDesc.RasterizerState = m_RasterizerDescs_[rasterizer];
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = topology;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.InputLayout = m_LayoutDescs_[layout];
    ComPtr<ID3D12PipelineState> pso;
    HRESULT res = m_Device_->CreateGraphicsPipelineState (&psoDesc, IID_PPV_ARGS (&pso));
    m_PSOs_.push_back (pso);
    ++m_nPSOs_;
    return (m_nPSOs_ - 1);
}

int FatDXFramework::CreatePositionColorLayout () {
    D3D12_INPUT_ELEMENT_DESC vertDesc[2] ={};
    {
        vertDesc[0].SemanticName = "POSITION";
        vertDesc[0].SemanticIndex = 0;
        vertDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        vertDesc[0].AlignedByteOffset = 0;
        vertDesc[0].InputSlot = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        vertDesc[0].InstanceDataStepRate = 0;
        vertDesc[1].SemanticName = "COLOR";
        vertDesc[1].SemanticIndex = 0;
        vertDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        vertDesc[1].AlignedByteOffset = 12;
        vertDesc[1].InputSlot = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        vertDesc[1].InstanceDataStepRate = 0;
    }

    m_ElementsDescs_.push_back (vertDesc[0]);
    m_ElementsDescs_.push_back (vertDesc[1]);


    D3D12_INPUT_LAYOUT_DESC layoutDesc ={};
    {
        layoutDesc.pInputElementDescs = &*(m_ElementsDescs_.end() - 2);
        layoutDesc.NumElements = 2;
    }
    m_LayoutDescs_.push_back (layoutDesc);
    ++m_nLayoutDescs_;
    return (m_nLayoutDescs_ - 1);
}

int FatDXFramework::CreateGeometry () {
    m_Geometries_.push_back (std::make_shared<std::vector<float>> ());
    ++m_nGeometries_;
    return (m_nGeometries_ - 1);
}

void FatDXFramework::AddGeometry (const int & gidx, const std::vector<float>& gdata) {
    auto geometry = m_Geometries_[gidx];
    geometry->insert (geometry->end (), gdata.begin (), gdata.end ());
}

int FatDXFramework::CreateBufferFromGeometry (const int & gidx, int* viewidx) {
    int buffer_size = 4 * m_Geometries_[gidx]->size ();   

    ComPtr<ID3D12Resource> uploadbuffer;
    m_CommandAllocator_->Reset ();
    m_CommandList_->Reset (m_CommandAllocator_.Get (), nullptr);
    ComPtr<ID3D12Resource> buffer = LoadDataToGpu (m_Geometries_[gidx]->data (), buffer_size, uploadbuffer);
    m_CommandList_->Close ();
    ComPtr<ID3D12CommandList> cmdsList[] ={ m_CommandList_.Get () };
    m_CommandQueue_->ExecuteCommandLists (_countof (cmdsList), cmdsList->GetAddressOf ());
    HRESULT hr = m_CommandQueue_->Signal (m_Fence_.Get (), m_FenceValue_);
    WaitSignal ();

    m_Resources_.push_back (buffer);
    ++m_nResourcses_;

    D3D12_VERTEX_BUFFER_VIEW vbView;
    {
        vbView.BufferLocation = buffer->GetGPUVirtualAddress ();
        vbView.SizeInBytes = buffer_size;
        vbView.StrideInBytes = 28;
    }

    m_VertexBufferViews_.push_back (vbView);
    *viewidx = m_nVertexBuffersViews_;
    ++m_nVertexBuffersViews_;
    return (m_nResourcses_ - 1);
}



void FatDXFramework::Render ()
{
    m_CommandAllocator_->Reset ();
    m_CommandList_->Reset (m_CommandAllocator_.Get (), m_Pso_.Get ());
    ChangeResourceState (m_SwapChainBuffers_[m_CurrentBuffer_], D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    
    m_CommandList_->RSSetViewports (1, &m_Viewport_);
    D3D12_RECT scissor{ 0.0f, 0.0f, m_ClientWidth_, m_ClientHeight_ };
    m_CommandList_->RSSetScissorRects (1, &scissor);    
    float c[] = { 0.1f, 0.3f, 0.5f, 1.0f };
    D3D12_CPU_DESCRIPTOR_HANDLE temp_desc = m_RtvHeap_->GetCPUDescriptorHandleForHeapStart ();
    temp_desc.ptr += m_CurrentBuffer_ * m_uSizeRtv_;
    m_CommandList_->OMSetRenderTargets (1, &temp_desc, true, &m_DsvHeapHandle_);
    m_CommandList_->ClearRenderTargetView (temp_desc, c, 0, nullptr);
    m_CommandList_->ClearDepthStencilView (m_DsvHeapHandle_, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    
    m_CommandList_->SetPipelineState ( m_PSOs_[0].Get ( ) );
    m_CommandList_->SetGraphicsRootSignature (m_RootSignatures_[0].Get ());

    m_CommandList_->IASetPrimitiveTopology (D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    int i = 0;
    for (auto g : m_Geometries_) {
        int num_of_verts = 4*g->size () / m_VertexBufferViews_[i].StrideInBytes;
        m_CommandList_->IASetVertexBuffers (0, 1, &m_VertexBufferViews_[i]);
        m_CommandList_->DrawInstanced (num_of_verts, 1, 0, 0);
        ++i;
    }

    ChangeResourceState (m_SwapChainBuffers_[m_CurrentBuffer_], D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

    m_CommandList_->Close ();
    ComPtr<ID3D12CommandList> new_cmdsList[] = { m_CommandList_.Get () };
    m_CommandQueue_->ExecuteCommandLists (_countof (new_cmdsList), new_cmdsList->GetAddressOf ());
    m_CommandQueue_->Signal (m_Fence_.Get (), m_FenceValue_);

    m_SwapChain_->Present (0, 0);
        
    m_CurrentBuffer_ = (m_CurrentBuffer_ + 1) % 2;
    WaitSignal ();
}

void FatDXFramework::WaitSignal ()
{
    if (m_Fence_->GetCompletedValue () < m_FenceValue_) {
        // we have the fence create an event which is signaled once the fence's current value is "fenceValue"
        m_Fence_->SetEventOnCompletion (m_FenceValue_, m_FenceEvent_);

        // We will wait until the fence has triggered the event that it's current value has reached "fenceValue". once it's value
        // has reached "fenceValue", we know the command queue has finished executing
        WaitForSingleObject (m_FenceEvent_, INFINITE);
    }
    ++m_FenceValue_;
}
