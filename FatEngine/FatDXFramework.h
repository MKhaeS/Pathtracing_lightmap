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
	FatDXFramework();

    bool Initialize              ( const HWND& hWnd,
                                   const int&  width, const int& height,
                                   const DXGI_FORMAT& buffer_format );

    void Render ();	

    
    // --- High level user interface ---
    int CreateRootSignature ();

	int CreateTextureRootSignature ();

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

	int CreatePositionTextureLayout ();

    int CreateGeometry ();

    void AddGeometry (const int& gidx, const std::vector<float>& gdata);

    int CreateBufferFromGeometry (const int& gidx, const int& stride, int* viewidx);


    // --- Textures ---
	int CreateTexture2D( const int& width, const int& height, 
                         const int& samples, const DXGI_FORMAT& format );
    bool CreateRtvForTexture( const int& texture );
	bool CreateSrvForTexture( const int& texture );



    // --- Render ---
    void SetRenderTarget( const int& rtvTexture, const XMFLOAT4& clearColor );
    void DrawStaticObjectsSets(int* noObjects, const int& size);
    void RenderToTarget();


	struct RootParameter
	{
		D3D12_ROOT_PARAMETER_TYPE	Type		= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		int							Slot        =  0;
		int							Texture		= -1;
	};

    
    struct Static3DObject
    {
        int		                VertexBufferView	= 0;
        int                     NumOfVerts          = 0;
        int		                RootSignature		= 0;
        int		                Pso					= 0;
        bool	                Visible				= true;
        D3D_PRIMITIVE_TOPOLOGY  Topology            = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		std::vector<RootParameter>		RootParameters;

    };    

    typedef std::vector<std::shared_ptr<Static3DObject>> StaticObjectsSet;
    int AddStaticObject( const int& objectSet, std::shared_ptr<Static3DObject> object );
    int CreateStaticObjectsSet();

    struct Texture2D
    {
        ComPtr<ID3D12Resource>              Resource            = nullptr;
        ComPtr<ID3D12DescriptorHeap>        RtvDescriptorHeap   = nullptr;
        ComPtr<ID3D12DescriptorHeap>        SrvDescriptorHeap   = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE         RTV                 = {};
        D3D12_CPU_DESCRIPTOR_HANDLE         SRV                 = {};
        D3D12_RESOURCE_STATES               State               = D3D12_RESOURCE_STATE_COMMON;        
    };

private:

    // --- Initialization --- "FatDXInit.cpp"
	HWND                                    m_hWnd_                 = 0;

	ComPtr<ID3D12Debug>                     m_DebugController_      = nullptr;
	bool                                    m_bDebugLayerActive_    = false;

	ComPtr<IDXGIFactory4>                   m_Factory_              = nullptr;
	ComPtr<ID3D12Device>                    m_Device               = nullptr;	
    std::vector <ComPtr<IDXGIAdapter1>>	    m_SystemAdapters_;

    ComPtr<ID3D12Fence>                     m_Fence_                = nullptr;
    HANDLE                                  m_FenceEvent_           = nullptr;
    long int                                m_FenceValue_           = 0;

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

    D3D12_VIEWPORT                          m_Viewport_;    
    
    bool CreateFactory           ();
    void NumerateAdapters        ();
    bool CreateDeviceFromAdapter ( ComPtr<IDXGIAdapter1> noAdapter );
	bool CreateFence             ();
	void AssignDescriptorSizes   ();
	bool CreateCommandStructure  ();
	bool CreateSwapChain         ( const int&            width,          const int& height,
		                           const DXGI_FORMAT&    buffer_format,  const int& nBuffers);

	bool CreateDescriptorHeaps   ();
	bool CreateRenderTargetView  ();
    bool CreateDepthStencilView  ();
    void SetViewport             ( const float& top_left_x,  const float& top_left_y, 
                                   const float& width,       const float& height);
    //-end- --- Initialization ---


    // --- Resource manipulation --- "FatDXResourceManipulation.cpp"
    ComPtr<ID3D12Resource>      CreateGpuBuffer           ( const UINT64&   bufferSize );

    ComPtr<ID3D12Resource>      CreateUploadBuffer        ( const UINT64&   bufferSize );

    ComPtr<ID3D12Resource>      LoadDataToGpu             ( const void*                 vertexData,
                                                            const UINT64&               dataSize,
                                                            ComPtr<ID3D12Resource>&     uploadBuffer );

    bool                        CopyDataToBuffer          ( const ComPtr<ID3D12Resource>&   destination,
                                                            const ComPtr<ID3D12Resource>&   upload,
                                                            const void*                     pDataSource,
                                                            const UINT64&                   dataSize ) const;     

    void                        ResourceStateTransition   ( const ComPtr<ID3D12Resource>&   resource,
                                                            D3D12_RESOURCE_STATES           prev_state, 
                                                            D3D12_RESOURCE_STATES           next_state );
    // -end- --- Resource manipulation ---

    ComPtr<ID3DBlob> LoadBinary ( const std::wstring& filename );


    //Timer
    Time t0;

    //    
    
    void WaitSignal ();

    
    // --- Pipeline stages elements---
    std::vector<ComPtr<ID3D12RootSignature>>                m_RootSignatures;
    int                                                     m_nRootSignatures         = 0;
    std::vector<ComPtr<ID3DBlob>>                           m_VertexShaders;
    int                                                     m_nVertexShaders          = 0;
    std::vector<ComPtr<ID3DBlob>>                           m_PixelShaders;
    int                                                     m_nPixelShaders           = 0;
    std::vector<D3D12_RASTERIZER_DESC>                      m_RasterizerDescs;
    int                                                     m_nRasterizerDescs        = 0;
    std::vector<D3D12_INPUT_LAYOUT_DESC>                    m_LayoutDescs;
    int                                                     m_nLayoutDescs            = 0;
    std::vector<ComPtr<ID3D12PipelineState>>                m_Psos;
    int                                                     m_nPsos                   = 0;
    std::vector<D3D12_INPUT_ELEMENT_DESC>                   m_ElementsDescs;


    std::vector<std::shared_ptr<std::vector<float>>>        m_Geometries;
    int                                                     m_nGeometries             = 0;
    std::vector<ComPtr<ID3D12Resource>>                     m_Resources;
    int                                                     m_nResourcses             = 0;


    std::vector<ComPtr<ID3D12DescriptorHeap>>               m_DescriptorHeaps;
    int                                                     m_nDescriptorHeaps        = 0;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE>                m_DescriptorHandles;
    int                                                     m_nDescriptorHandles      = 0;

    std::vector<D3D12_VERTEX_BUFFER_VIEW>                   m_VertexBufferViews;
    int                                                     m_nVertexBuffersViews     = 0;

    std::vector<std::shared_ptr<Static3DObject>>            m_StaticObjects;
    int                                                     m_nStaticObjects          = 0;
    std::vector<std::unique_ptr<StaticObjectsSet>>          m_StaticObjectsSets;
    int                                                     m_nStaticObjectsSets      = 0;

    std::vector<std::unique_ptr<Texture2D>>                 m_Textures;
    int                                                     m_nTextures               = 0;







    
};




