#pragma once

#include "FatDXFramework.h"


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
    res = m_Device->CreateRootSignature (0, serializedRS->GetBufferPointer (),
                                          serializedRS->GetBufferSize (), IID_PPV_ARGS (&newSignature));
    if (FAILED (res)) {
        return -1;
    }

    m_RootSignatures.push_back (newSignature);
    ++m_nRootSignatures;

    return (m_nRootSignatures - 1);
}

int FatDXFramework::CreateTextureRootSignature () {
    D3D12_DESCRIPTOR_RANGE descRange = {};
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
        descTable.NumDescriptorRanges = 1;
        descTable.pDescriptorRanges = &descRange;
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
    res = m_Device->CreateRootSignature ( 0, serializedRS->GetBufferPointer (),
                                           serializedRS->GetBufferSize (), IID_PPV_ARGS ( &newSignature ) );
    if ( FAILED ( res ) ) {
        return -1;
    }

    m_RootSignatures.push_back ( newSignature );
    ++m_nRootSignatures;

    return (m_nRootSignatures - 1);
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

    m_ElementsDescs.push_back (vertDesc[0]);
    m_ElementsDescs.push_back (vertDesc[1]);


    D3D12_INPUT_LAYOUT_DESC layoutDesc ={};
    {
        layoutDesc.pInputElementDescs = &*(m_ElementsDescs.end() - 2);
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

	m_ElementsDescs.push_back (vertDesc[0]);
	m_ElementsDescs.push_back (vertDesc[1]);


	D3D12_INPUT_LAYOUT_DESC layoutDesc ={};
	{
		layoutDesc.pInputElementDescs = &*(m_ElementsDescs.end () - 2);
		layoutDesc.NumElements = 2;
	}
	m_LayoutDescs.push_back (layoutDesc);
	++m_nLayoutDescs;
	return (m_nLayoutDescs - 1);
}

int FatDXFramework::CreateGeometry () {
    m_Geometries.push_back (std::make_shared<std::vector<float>> ());
    ++m_nGeometries;
    return (m_nGeometries - 1);
}

void FatDXFramework::AddGeometry (const int & gidx, const std::vector<float>& gdata) {
	if (gidx < m_Geometries.size ()) {
		auto geometry = m_Geometries[gidx];
		geometry->insert (geometry->end (), gdata.begin (), gdata.end ());
	}
}

int FatDXFramework::CreateBufferFromGeometry (const int & gidx, const int& stride, int* viewidx) {
    int buffer_size = 4 * m_Geometries[gidx]->size ();   

    ComPtr<ID3D12Resource> uploadbuffer;
    ComPtr<ID3D12Resource> buffer = LoadDataToGpu (m_Geometries[gidx]->data (), buffer_size, uploadbuffer);
    m_CommandList_->Close ();
    ComPtr<ID3D12CommandList> cmdsList[] ={ m_CommandList_.Get () };
    m_CommandQueue_->ExecuteCommandLists (_countof (cmdsList), cmdsList->GetAddressOf ());
    HRESULT hr = m_CommandQueue_->Signal (m_Fence_.Get (), m_FenceValue_);
    WaitSignal ();

    m_Resources.push_back (buffer);
    ++m_nResourcses;

    D3D12_VERTEX_BUFFER_VIEW vbView;
    {
        vbView.BufferLocation = buffer->GetGPUVirtualAddress ();
        vbView.SizeInBytes = buffer_size;
        vbView.StrideInBytes = stride;
    }

    m_VertexBufferViews.push_back (vbView);
    *viewidx = m_nVertexBuffersViews;
    ++m_nVertexBuffersViews;
    return (m_nVertexBuffersViews - 1);
}


int FatDXFramework::AddStaticObject( const int& objectSet, std::shared_ptr<Static3DObject> object ) {
    m_StaticObjects.push_back( object );    
    ++m_nStaticObjects;

    if ( objectSet < m_nStaticObjectsSets ) {
        m_StaticObjectsSets[objectSet]->push_back( object );
    }

    return (m_nStaticObjects - 1);
}

int FatDXFramework::CreateStaticObjectsSet() {
    m_StaticObjectsSets.push_back( std::make_unique<StaticObjectsSet>() );
    ++m_nStaticObjectsSets;
    return (m_nStaticObjectsSets - 1);
}



void FatDXFramework::WaitSignal ()
{
    if (m_Fence_->GetCompletedValue () < m_FenceValue_) {
        m_Fence_->SetEventOnCompletion (m_FenceValue_, m_FenceEvent_);
        WaitForSingleObject (m_FenceEvent_, INFINITE);
    }
    ++m_FenceValue_;
	m_CommandAllocator_->Reset ();
	m_CommandList_->Reset (m_CommandAllocator_.Get (), nullptr);
}
