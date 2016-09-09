#pragma once
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")

#include <assert.h>
#include <vector>
#include <string>
#include <memory>
#include <fstream>

#include <wrl.h>

#include "d3dcompiler.h"
#include "d3d12.h"
#include "dxgi1_4.h"
#include "DirectXMath.h"

#include "Time.h"

using namespace Microsoft::WRL;
using namespace DirectX;

class FatDXFramework final
{
public:
	explicit FatDXFramework();
	
	void NumerateOutputsForDevice(const int& noDevice);

	bool Initialize(const HWND& hWnd,
                    const int&  width,    const int& height,
		            const DXGI_FORMAT& buffer_format);

    void Update ();

    // -- User interface --
    int CreateRootSignature ();

    int CreateVertexShaderFromCso (const std::wstring& filename);

    int CreatePixelShaderFromCso  (const std::wstring& filename);

    int CreateRasterizerDescription (const D3D12_FILL_MODE& fillmode, 
                                     const D3D12_CULL_MODE& cullmode, 
                                     const bool& multisample, 
                                     const int& samplecount);

    int CreatePipelineStateObject (const int& rootsignature, 
                                   const int& vshader, 
                                   const int& pshader, 
                                   const int& rasterizer, 
                                   const int& layout,
                                   const D3D12_PRIMITIVE_TOPOLOGY_TYPE& topology );

    int CreatePositionColorLayout ();

    int CreateGeometry ();

    void AddGeometry (const int& gidx, const std::vector<float>& gdata);

    int CreateBufferFromGeometry (const int& gidx, int* viewidx);    



private:
	HWND                                    m_hWnd_;

	ComPtr<ID3D12Debug>                     m_DebugController_      = nullptr;
	bool                                    m_bDebugLayerActive_    = false;

	ComPtr<IDXGIFactory4>                   m_Factory_              = nullptr;
	ComPtr<ID3D12Device>                    m_Device_               = nullptr;	
    std::vector <ComPtr<IDXGIAdapter1>>	    m_SystemAdapters_;

    ComPtr<ID3D12Fence>                     m_Fence_                = nullptr;
    HANDLE                                  m_FenceEvent_           = nullptr;
    int                                     m_FenceValue_           = 0;

	ComPtr<ID3D12CommandQueue>              m_CommandQueue_         = nullptr;
	ComPtr<ID3D12CommandAllocator>          m_CommandAllocator_     = nullptr;
	ComPtr<ID3D12GraphicsCommandList>       m_CommandList_          = nullptr;

	ComPtr<IDXGISwapChain3>                 m_SwapChain_            = nullptr;
    DXGI_SWAP_CHAIN_DESC1                   m_SwapChainDescription_ = { 0 };
    DXGI_SAMPLE_DESC                        m_SampleDescription_    = { 0 };
    std::vector <ComPtr<ID3D12Resource>>    m_SwapChainBuffers_;
    ComPtr<ID3D12Resource>                  m_DepthStencilBuffer_   = nullptr;
    int                                     m_nSwapChainBuffers_;
    int                                     m_CurrentBuffer_;
    UINT                                    m_ClientWidth_;
    UINT                                    m_ClientHeight_;
	
    ComPtr<ID3D12DescriptorHeap>            m_RtvHeap_              = nullptr;
	ComPtr<ID3D12DescriptorHeap>            m_DsvHeap_              = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE             m_RtvHeapHandle_;
    D3D12_CPU_DESCRIPTOR_HANDLE             m_DsvHeapHandle_;
    UINT                                    m_uSizeRtv_;
    UINT                                    m_uSizeDsv_;
    UINT                                    m_uSizeCbvSrv_;

	
	std::vector <ComPtr<IDXGIOutput>>       m_DeviceOutputs_;

    D3D12_VIEWPORT                          m_Viewport_;    

	bool CreateFactory();
	void NumerateAdapters();
	bool CreateDeviceFromAdapter(ComPtr<IDXGIAdapter1> noAdapter);
	bool CreateFence();
	void AssignDescriptorSizes();
	bool CreateCommandStructure();
	bool CreateSwapChain(const int&                     width, const int& height,
		                 const DXGI_FORMAT&     buffer_format, const int& nBuffers);

	bool                 CreateDescriptorHeaps();
	bool                CreateRenderTargetView();
    bool                CreateDepthStencilView();
    void                           SetViewport( const float& top_left_x,  const float& top_left_y, 
                                                const float& width,       const float& height);



    // Resource manipulations
    ComPtr<ID3D12Resource>    CreateGpuBuffer ( const UINT64& bufferSize );

    ComPtr<ID3D12Resource> CreateUploadBuffer ( const UINT64& bufferSize );

    ComPtr<ID3D12Resource>      LoadDataToGpu ( const void* vertexData,
                                                const UINT64& dataSize,
                                                ComPtr<ID3D12Resource>& uploadBuffer );

    bool                     CopyDataToBuffer ( const ComPtr<ID3D12Resource>& destination,
                                                const ComPtr<ID3D12Resource>& upload,
                                                const void* pDataSource,
                                                const UINT64& dataSize ) const;     

    void                  ChangeResourceState ( const ComPtr<ID3D12Resource>& resource,
                                                D3D12_RESOURCE_STATES prev_state, 
                                                D3D12_RESOURCE_STATES next_state );
    /////////////////////////
    ComPtr<ID3D12Resource> m_VertBuffer_ = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_VBview_;
    
    //
    ComPtr<ID3D12PipelineState> m_Pso_ = nullptr;
    ComPtr<ID3D12RootSignature> m_RootSignature_ = nullptr;
    void BuildPSO();
    ComPtr<ID3DBlob> LoadBinary ( const std::wstring& filename );


    //Timer
    Time t0;

    //    
    void Render ();
    void WaitSignal ();

    
    //User service collections;
    std::vector<ComPtr<ID3D12RootSignature>>                m_RootSignatures_;
    int                                                     m_nRootSignatures_         = 0;
    std::vector<ComPtr<ID3DBlob>>                           m_VertexShaders_;
    int                                                     m_nVertexShaders_          = 0;
    std::vector<ComPtr<ID3DBlob>>                           m_PixelShaders_;
    int                                                     m_nPixelShaders_           = 0;
    std::vector<D3D12_RASTERIZER_DESC>                      m_RasterizerDescs_;
    int                                                     m_nRasterizerDescs         = 0;
    std::vector<D3D12_INPUT_LAYOUT_DESC>                    m_LayoutDescs_;
    int                                                     m_nLayoutDescs_            = 0;
    std::vector<ComPtr<ID3D12PipelineState>>                m_PSOs_;
    int                                                     m_nPSOs_                   = 0;
    std::vector<D3D12_INPUT_ELEMENT_DESC>                   m_ElementsDescs_;
    std::vector<std::shared_ptr<std::vector<float>>>        m_Geometries_;
    int                                                     m_nGeometries_             = 0;
    std::vector<ComPtr<ID3D12Resource>>                     m_Resources_;
    int                                                     m_nResourcses_             = 0;
    std::vector<ComPtr<ID3D12DescriptorHeap>>               m_DescriptorHeaps_;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>                m_DescriptorHandles_;
    std::vector<D3D12_VERTEX_BUFFER_VIEW>                   m_VertexBufferViews_;
    int                                                     m_nVertexBuffersViews_     = 0;




    
};




