#pragma once

#include <vector>

#include "FatDxResource.h"
#include "FatDxBuffer.h"
#include "dxgi1_4.h"

namespace FatDx
{
    class Texture : public Resource
    {
    public:
        Texture ( ComPtr<ID3D12Device>   device,     const D3D12_HEAP_TYPE&  heaptype,
                  const wstring&         filename                                    );

        Texture ( ComPtr<ID3D12Device>   device,     const D3D12_HEAP_TYPE&  heaptype,
                  const int&             width,      const int&              height,                 
                  const bool&            srv,        const bool&             rtv,
                  const int&             samples                                     );

        void UploadTexture              (ComPtr<ID3D12GraphicsCommandList> cmdlist);

        void SetAsRenderTarget          (ComPtr<ID3D12GraphicsCommandList> cmdlist);
        void SetAsPixelShaderResource   (ComPtr<ID3D12GraphicsCommandList> cmdlist);

        D3D12_CPU_DESCRIPTOR_HANDLE* Rtv ();
        ID3D12DescriptorHeap** RtvHeapAddress ();
        D3D12_CPU_DESCRIPTOR_HANDLE* Srv ();
        D3D12_GPU_DESCRIPTOR_HANDLE SrvGpuHandle ();
        ID3D12DescriptorHeap** SrvHeapAddress ();
        

    private:
        DXGI_FORMAT                     m_Format    = DXGI_FORMAT_B8G8R8A8_UNORM;
        ComPtr<ID3DBlob>                m_Data      = nullptr;
        ComPtr<ID3D12DescriptorHeap>    m_SrvHeap   = nullptr;
        ComPtr<ID3D12DescriptorHeap>    m_RtvHeap   = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE     m_Srv       = {};
        D3D12_CPU_DESCRIPTOR_HANDLE     m_Rtv       = {};

		unique_ptr<Buffer> m_UploadBuffer;

        void CreateTextureResource ( const int&    width,      const int&       height,
                                     const bool&   srv,        const bool&      rtv,
                                     const int&    samples                            );
        
        void CreateRenderTargetView ();
        void CreateShaderResourceView ();

        void UploadToDefaultBuffer (ComPtr<ID3D12GraphicsCommandList> cmdlist);
        void UploadToDynamicBuffer (ComPtr<ID3D12GraphicsCommandList> cmdlist);     // Upload buffer is called here dynamic, because 
                                                                                    // it is used to store data, which changes every
                                                                                    // frame (or frequently enough). Otherwise it is 
                                                                                    // better to use other types of buffer heaps.
    };
}
