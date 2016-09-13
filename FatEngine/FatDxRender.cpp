#include "FatDXFramework.h"



void FatDXFramework::Render () {
    m_CommandList_->RSSetViewports ( 1, &m_Viewport_ );
    D3D12_RECT scissor { 0.0f, 0.0f, m_ClientWidth_, m_ClientHeight_ };
    m_CommandList_->RSSetScissorRects ( 1, &scissor );
    ResourceStateTransition ( m_SwapChainBuffers_[m_CurrentBuffer_],
                              D3D12_RESOURCE_STATE_PRESENT,
                              D3D12_RESOURCE_STATE_RENDER_TARGET );
    float c[] = { 0.1f, 0.3f, 0.5f, 1.0f };
    D3D12_CPU_DESCRIPTOR_HANDLE temp_desc = m_RtvHeap_->GetCPUDescriptorHandleForHeapStart ();
    temp_desc.ptr += m_CurrentBuffer_ * m_uSizeRtv_;
    m_CommandList_->OMSetRenderTargets ( 1, &temp_desc, true, &m_DsvHeapHandle_ );
    m_CommandList_->ClearRenderTargetView ( temp_desc, c, 0, nullptr );
    m_CommandList_->ClearDepthStencilView ( m_DsvHeapHandle_, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr );


    
    for ( int i = 0; i < m_StaticObjectsSets.size(); ++i ) {
        for ( auto o : *m_StaticObjectsSets[i]) {
            int n_rs = o->RootSignature;
            int n_pso = o->Pso;
            int n_vbview = o->VertexBufferView;
            m_CommandList_->IASetPrimitiveTopology ( o->Topology );
            m_CommandList_->SetGraphicsRootSignature ( m_RootSignatures[n_rs].Get () );

			for ( int i = 0; i < o->RootParameters.size(); ++i ) {
				RootParameter rp = o->RootParameters[i];
				switch ( rp.Type ) {
				case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE :
					if ( rp.Texture > -1 ) {
						Texture2D& tex = *m_Textures[rp.Texture];
						ResourceStateTransition ( tex.Resource,
												  tex.State,
												  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );
						m_CommandList_->
							SetDescriptorHeaps( 1, tex.SrvDescriptorHeap.GetAddressOf() );
						m_CommandList_->
							SetGraphicsRootDescriptorTable( rp.Slot,
															tex.SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart() );
					} else {

					}
					break;
				}
			}


            m_CommandList_->SetPipelineState ( m_Psos[n_pso].Get () );
            m_CommandList_->IASetVertexBuffers ( 0, 1, &m_VertexBufferViews[n_vbview] );
            m_CommandList_->DrawInstanced ( o->NumOfVerts, 1, 0, 0 );
        }
    }

    ResourceStateTransition ( m_SwapChainBuffers_[m_CurrentBuffer_], D3D12_RESOURCE_STATE_RENDER_TARGET,
                              D3D12_RESOURCE_STATE_PRESENT );

    m_CommandList_->Close ();
    ComPtr<ID3D12CommandList> new_cmdsList[] = { m_CommandList_.Get () };
    m_CommandQueue_->ExecuteCommandLists ( _countof ( new_cmdsList ), new_cmdsList->GetAddressOf () );
    m_CommandQueue_->Signal ( m_Fence_.Get (), m_FenceValue_ );

    m_SwapChain_->Present ( 0, 0 );

    m_CurrentBuffer_ = (m_CurrentBuffer_ + 1) % 2;
    WaitSignal ();
}



void FatDXFramework::SetRenderTarget( const int & rtvTexture, const XMFLOAT4& clearColor ) {
    m_CommandList_->RSSetViewports ( 1, &m_Viewport_ );
    D3D12_RECT scissor { 0.0f, 0.0f, m_ClientWidth_, m_ClientHeight_ };
    m_CommandList_->RSSetScissorRects ( 1, &scissor );
    ResourceStateTransition ( m_Textures[rtvTexture]->Resource,
                              m_Textures[rtvTexture]->State,
                              D3D12_RESOURCE_STATE_RENDER_TARGET );
    m_Textures[rtvTexture]->State = D3D12_RESOURCE_STATE_RENDER_TARGET;
    m_CommandList_->OMSetRenderTargets ( 1, &(m_Textures[rtvTexture]->RTV), true, &m_DsvHeapHandle_ );
    float color[] = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
    m_CommandList_->ClearRenderTargetView ( m_Textures[rtvTexture]->RTV, color, 0, nullptr );
    m_CommandList_->ClearDepthStencilView ( m_DsvHeapHandle_, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr );
}



void FatDXFramework::DrawStaticObjectsSets( int* noObjects, const int & size ) {
    for (int i = 0; i < m_StaticObjectsSets.size (); ++i) {
        for (auto o : *m_StaticObjectsSets[i]) {
            int n_rs = o->RootSignature;
            int n_pso = o->Pso;
            int n_vbview = o->VertexBufferView;
            m_CommandList_->IASetPrimitiveTopology (o->Topology);
            m_CommandList_->SetGraphicsRootSignature (m_RootSignatures[n_rs].Get ());

            for (int i = 0; i < o->RootParameters.size (); ++i) {
                RootParameter rp = o->RootParameters[i];
                switch (rp.Type) {
                case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
                    if (rp.Texture > -1) {
                        Texture2D& tex = *m_Textures[rp.Texture];
                        ResourceStateTransition (tex.Resource,
                                                 tex.State,
                                                 D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
                        m_CommandList_->
                            SetDescriptorHeaps (1, tex.SrvDescriptorHeap.GetAddressOf ());
                        m_CommandList_->
                            SetGraphicsRootDescriptorTable (rp.Slot,
                                                            tex.SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart ());
                    } else {

                    }
                    break;
                }
            }


            m_CommandList_->SetPipelineState (m_Psos[n_pso].Get ());
            m_CommandList_->IASetVertexBuffers (0, 1, &m_VertexBufferViews[n_vbview]);
            m_CommandList_->DrawInstanced (o->NumOfVerts, 1, 0, 0);
        }
    }
}



void FatDXFramework::RenderToTarget() {
    m_CommandList_->Close ();
    ComPtr<ID3D12CommandList> new_cmdsList[] = { m_CommandList_.Get () };
    m_CommandQueue_->ExecuteCommandLists ( _countof ( new_cmdsList ), new_cmdsList->GetAddressOf () );
    m_CommandQueue_->Signal ( m_Fence_.Get (), m_FenceValue_ );

    WaitSignal ();
}





