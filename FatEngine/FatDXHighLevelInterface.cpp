#include "FatDXFramework.h"


namespace FatDx
{
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
            rastDesc.FillMode               = fillmode;
            rastDesc.CullMode               = cullmode;
            rastDesc.FrontCounterClockwise  = FALSE;
            rastDesc.DepthBias              = D3D12_DEFAULT_DEPTH_BIAS;
            rastDesc.DepthBiasClamp         = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            rastDesc.SlopeScaledDepthBias   = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            rastDesc.DepthClipEnable        = TRUE;
            rastDesc.AntialiasedLineEnable  = FALSE;
            rastDesc.ConservativeRaster     = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
            rastDesc.MultisampleEnable      = multisample;
            rastDesc.ForcedSampleCount      = nsamples;
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
        {
            psoDesc.pRootSignature              = m_RootSignatures[rootsignature].Get ();
            psoDesc.VS                          ={
                reinterpret_cast<BYTE*>(m_VertexShaders[vshader]->GetBufferPointer ()),
                m_VertexShaders[vshader]->GetBufferSize () };
            psoDesc.PS                          ={
                reinterpret_cast<BYTE*>(m_PixelShaders[pshader]->GetBufferPointer ()),
                m_PixelShaders[pshader]->GetBufferSize () };
            psoDesc.BlendState                  = blendDesc;
            psoDesc.RasterizerState             = m_RasterizerDescs[rasterizer];
            psoDesc.SampleMask                  = UINT_MAX;
            psoDesc.PrimitiveTopologyType       = topology;
            psoDesc.NumRenderTargets            = 1;
            psoDesc.RTVFormats[0]               = DXGI_FORMAT_B8G8R8A8_UNORM;
            psoDesc.SampleDesc.Count            = 1;
            psoDesc.SampleDesc.Quality          = 0;
            psoDesc.InputLayout                 = m_LayoutDescs[layout];
        }

        ComPtr<ID3D12PipelineState> pso;
        HRESULT res = m_Device->CreateGraphicsPipelineState (&psoDesc, IID_PPV_ARGS (&pso));
        m_Psos.push_back (pso);
        ++m_nPsos;
        return (m_nPsos - 1);
    }
}