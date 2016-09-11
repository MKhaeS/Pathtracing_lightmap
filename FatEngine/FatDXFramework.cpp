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

int FatDXFramework::CreateRootSignature () {
    D3D12_ROOT_SIGNATURE_DESC rsDesc ={};
    {
        rsDesc.NumParameters = 0;
        rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    }

    ComPtr<ID3DBlob> serializedRS;
    ComPtr<ID3DBlob> errorBlob;
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

    m_RootSignatures.push_back (newSignature);
    ++m_nRootSignatures;

    return (m_nRootSignatures - 1);
}

int FatDXFramework::CreateTextureRootSignature () {
	D3D12_DESCRIPTOR_RANGE descRange ={};
	{
		descRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descRange.RegisterSpace = 0;
		descRange.BaseShaderRegister = 0;
		descRange.NumDescriptors = 1;
		descRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	// create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE descTable = {};
	{
		descTable.NumDescriptorRanges = 1; // we only have one range
		descTable.pDescriptorRanges = &descRange; // the pointer to the beginning of our ranges array
	}

	D3D12_ROOT_PARAMETER  rootParameter = {};
	{
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter.DescriptorTable = descTable;
		rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	{
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	}

	D3D12_ROOT_SIGNATURE_DESC rsDesc = {};
	{
		rsDesc.NumParameters = 1;
		rsDesc.NumStaticSamplers = 1;
		rsDesc.pParameters = &rootParameter;
		rsDesc.pStaticSamplers = &sampler;
		rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	}

	ComPtr<ID3DBlob> serializedRS;
	ComPtr<ID3DBlob> errorBlob;
	ComPtr<ID3D12RootSignature> newSignature;
	HRESULT res = D3D12SerializeRootSignature ( &rsDesc, D3D_ROOT_SIGNATURE_VERSION_1,
												&serializedRS, &errorBlob );
	if ( FAILED ( res ) ) {
		return -1;
	}
	res = m_Device_->CreateRootSignature ( 0, serializedRS->GetBufferPointer (),
										   serializedRS->GetBufferSize (), IID_PPV_ARGS ( &newSignature ) );
	if ( FAILED ( res ) ) {
		return -1;
	}

	m_RootSignatures.push_back ( newSignature );
	++m_nRootSignatures;

	return (m_nRootSignatures - 1);

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
    m_VertexShaders.push_back (blob);
    ++m_nVertexShaders;
    return (m_nVertexShaders - 1);
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
    m_PixelShaders.push_back (blob);
    ++m_nPixelShaders;
    return (m_nPixelShaders - 1);
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
    m_RasterizerDescs.push_back (rastDesc);
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
    psoDesc.pRootSignature = m_RootSignatures[rootsignature].Get();
    psoDesc.VS = { 
        reinterpret_cast<BYTE*>(m_VertexShaders[vshader]->GetBufferPointer ()), 
                                m_VertexShaders[vshader]->GetBufferSize () };
    psoDesc.PS = { 
        reinterpret_cast<BYTE*>(m_PixelShaders[pshader]->GetBufferPointer ()),
                                m_PixelShaders[pshader]->GetBufferSize () };
    psoDesc.BlendState = blendDesc;
    psoDesc.RasterizerState = m_RasterizerDescs[rasterizer];
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = topology;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.InputLayout = m_LayoutDescs[layout];
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
    m_LayoutDescs.push_back (layoutDesc);
    ++m_nLayoutDescs;
    return (m_nLayoutDescs - 1);
}

int FatDXFramework::CreatePositionTextureLayout () {
	D3D12_INPUT_ELEMENT_DESC vertDesc[2] ={};
	{
		vertDesc[0].SemanticName = "POSITION";
		vertDesc[0].SemanticIndex = 0;
		vertDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertDesc[0].AlignedByteOffset = 0;
		vertDesc[0].InputSlot = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		vertDesc[0].InstanceDataStepRate = 0;
		vertDesc[1].SemanticName = "TEXCOORD";
		vertDesc[1].SemanticIndex = 0;
		vertDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertDesc[1].AlignedByteOffset = 12;
		vertDesc[1].InputSlot = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		vertDesc[1].InstanceDataStepRate = 0;
	}

	m_ElementsDescs_.push_back (vertDesc[0]);
	m_ElementsDescs_.push_back (vertDesc[1]);


	D3D12_INPUT_LAYOUT_DESC layoutDesc ={};
	{
		layoutDesc.pInputElementDescs = &*(m_ElementsDescs_.end () - 2);
		layoutDesc.NumElements = 2;
	}
	m_LayoutDescs.push_back (layoutDesc);
	++m_nLayoutDescs;
	return (m_nLayoutDescs - 1);
}

int FatDXFramework::CreateGeometry () {
    m_Geometries_.push_back (std::make_shared<std::vector<float>> ());
    ++m_nGeometries_;
    return (m_nGeometries_ - 1);
}

void FatDXFramework::AddGeometry (const int & gidx, const std::vector<float>& gdata) {
	if (gidx < m_Geometries_.size ()) {
		auto geometry = m_Geometries_[gidx];
		geometry->insert (geometry->end (), gdata.begin (), gdata.end ());
	}
}

int FatDXFramework::CreateBufferFromGeometry (const int & gidx, const int& stride, int* viewidx) {
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
        vbView.StrideInBytes = stride;
    }

    m_VertexBufferViews_.push_back (vbView);
    *viewidx = m_nVertexBuffersViews_;
    ++m_nVertexBuffersViews_;
    return (m_nResourcses_ - 1);
}

int FatDXFramework::CreateTextureResource( const int & width, const int & height, const int & samples, const DXGI_FORMAT & format ) {
	D3D12_DESCRIPTOR_HEAP_DESC descSrvHeapDesc = {};
	{
		descSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descSrvHeapDesc.NumDescriptors = 1;
		descSrvHeapDesc.NodeMask = 1;
		descSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	}

	D3D12_DESCRIPTOR_HEAP_DESC descRtvHeapDesc = {};
	{
		descSrvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descSrvHeapDesc.NumDescriptors = 1;
		descSrvHeapDesc.NodeMask = 1;
		descSrvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	}

	ComPtr<ID3D12DescriptorHeap> srv_desc_heap;
	HRESULT res = m_Device_->CreateDescriptorHeap( &descSrvHeapDesc, IID_PPV_ARGS( &srv_desc_heap ) );
	if ( FAILED( res ) ) {
		assert( !"Failed to create SRV descriptor heap" );
	}

	ComPtr<ID3D12DescriptorHeap> rtv_desc_heap;
	res = m_Device_->CreateDescriptorHeap( &descRtvHeapDesc, IID_PPV_ARGS( &rtv_desc_heap ) );
	if ( FAILED( res ) ) {
		assert( !"Failed to create RTV descriptor heap" );
	}

	D3D12_RESOURCE_DESC texResourceDesc = {};
	{
		texResourceDesc.Alignment = 0;
		texResourceDesc.DepthOrArraySize = 0;
		texResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		texResourceDesc.Format = format;
		texResourceDesc.Width = width;
		texResourceDesc.Height = height;
		texResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texResourceDesc.MipLevels = 1;
		texResourceDesc.SampleDesc.Count = samples;
		texResourceDesc.SampleDesc.Quality = 0;
		texResourceDesc.Width = width;
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
	D3D12_CPU_DESCRIPTOR_HANDLE textureHandle = srv_desc_heap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtv_desc_heap->GetCPUDescriptorHandleForHeapStart();

	res = m_Device_->CreateCommittedResource( &heapProps,
													  D3D12_HEAP_FLAG_NONE,
													  &texResourceDesc,
													  D3D12_RESOURCE_STATE_COMMON,
													  nullptr,
													  IID_PPV_ARGS  ( &texture_resource ) );
	if ( SUCCEEDED( res ) ) {
		m_Device_->CreateShaderResourceView( texture_resource.Get(), nullptr, textureHandle);
		m_Device_->CreateRenderTargetView( texture_resource.Get(), nullptr, rtvHandle );
	}

	return 0;
}


void FatDXFramework::Render ()
{    
    ResourceStateTransition (m_SwapChainBuffers_[m_CurrentBuffer_], D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    
    m_CommandList_->RSSetViewports (1, &m_Viewport_);
    D3D12_RECT scissor{ 0.0f, 0.0f, m_ClientWidth_, m_ClientHeight_ };
    m_CommandList_->RSSetScissorRects (1, &scissor);    
    float c[] = { 0.1f, 0.3f, 0.5f, 1.0f };
    D3D12_CPU_DESCRIPTOR_HANDLE temp_desc = m_RtvHeap_->GetCPUDescriptorHandleForHeapStart ();
    temp_desc.ptr += m_CurrentBuffer_ * m_uSizeRtv_;
    m_CommandList_->OMSetRenderTargets (1, &temp_desc, true, &m_DsvHeapHandle_);
    //m_CommandList_->ClearRenderTargetView (temp_desc, c, 0, nullptr);
    m_CommandList_->ClearDepthStencilView (m_DsvHeapHandle_, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    
    m_CommandList_->SetPipelineState ( m_PSOs_[0].Get ( ) );
    m_CommandList_->SetGraphicsRootSignature (m_RootSignatures[0].Get ());

    m_CommandList_->IASetPrimitiveTopology (D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    int i = 0;
    for (auto g : m_Geometries_) {
        int num_of_verts = 4*g->size () / m_VertexBufferViews_[i].StrideInBytes;
        m_CommandList_->IASetVertexBuffers (0, 1, &m_VertexBufferViews_[i]);
        m_CommandList_->DrawInstanced (num_of_verts, 1, 0, 0);
        ++i;
    }

    ResourceStateTransition (m_SwapChainBuffers_[m_CurrentBuffer_], D3D12_RESOURCE_STATE_RENDER_TARGET,
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
	m_CommandAllocator_->Reset ();
	m_CommandList_->Reset (m_CommandAllocator_.Get (), nullptr);
}
