//#include "DXForm.h"
#include "DXForm.h"
#include "FatDXFramework.h"
#include "dxgi.h"
#include<functional>
#include "DirectXMath.h"

using namespace DirectX;

DXForm::DXForm()
{
    width_   = GetSystemMetrics (SM_CXSCREEN) - 100;
    height_  = GetSystemMetrics (SM_CYSCREEN) - 100;
    pos_x_   = 50;
    pos_y_   = 50;
	CreateForm();     
}

void DXForm::InitComponents ()
{
    
	auto CloseButton = std::make_shared<Button>(Button( width_ - 30, 10, 20, 20, { 0.1f, 0.8f,0.2f } ));
	CloseButton->OnMouseClick += [this](WPARAM wParam, LPARAM lParam) { OnCloseButtonMouseClick (wParam, lParam); };
    AddComponent (CloseButton);
    OnKeyDown +=  [this](WPARAM wParam, LPARAM lParam) { OnKeyDownHandler (wParam, lParam); };

    InitScene ();
}

void DXForm::OnKeyDownHandler (WPARAM wParam, LPARAM lParam) {
    if (wParam == VK_ESCAPE)
        PostQuitMessage (0);
}

void DXForm::OnCloseButtonMouseClick (WPARAM wParam, LPARAM lParam) {
    PostQuitMessage (0);
}

void DXForm::UserRender () {
    SetBackPlainRenderTarget ();
    int objs[] = { objects_set };
    DxFramework->DrawStaticObjectsSets (objs, 1);

}



void DXForm::InitScene () {
    float w = 0.8f;
    std::vector<float> render_plane_data ={
        -w,  w, 1.0f,  0.0f, 0.0f,
        w,  w, 1.0f,  1.5f, 0.0f,
        -w, -w, 1.0f,  0.0f, 1.0f,
        w, -w, 1.0f,  1.5f, 1.0f,
        -w, -w, 1.0f, 0.0f, 1.0f,
        w,  w, 1.0f,  1.5f,  0.0f
    };
    int g = DxFramework->CreateGeometry ();
    DxFramework->AddDataToGeometry (g, render_plane_data);
    int buffer_view = DxFramework->CreateBufferFromGeometry (g, 20, &texture_buffer_view);
    objects_set = DxFramework->CreateStaticObjectsSet ();
    wstring filename = L"C:\\Users\\khaes\\Documents\\GitHub\\Pathtracing_lightmap\\bin\\x64\\Debug\\textures\\Dexter.dds";
    dex_texture = make_shared<Texture> (DxFramework->GetDevice(), D3D12_HEAP_TYPE_DEFAULT, filename);
    dex_texture->UploadTexture (DxFramework->GetCommandList());

    FatDXFramework::RootParameter rp_texture;
    {
        rp_texture.Texture = dex_texture;
    }

    mat_buffer = std::make_shared<FatDXFramework::MatrixBuffer> (DxFramework);

    FatDXFramework::RootParameter rp_matrix;
    {
        rp_matrix.Type = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rp_matrix.Slot = 1;
        rp_matrix.ConstantBuffer = mat_buffer->cb ()->GetGPUVirtualAddress();
    }
    mat_buffer->Upload ();
    auto interface_object = std::make_shared<FatDXFramework::Static3DObject> ();
    {
        interface_object->NumOfVerts = 6;
        interface_object->VertexBufferView = buffer_view;
        interface_object->Visible = true;
        interface_object->RootSignature = texture_root_signature;
        interface_object->Pso = texture_pso;
        interface_object->RootParameters.push_back (rp_texture);
        interface_object->RootParameters.push_back (rp_matrix);
    }

    DxFramework->AddStaticObject (objects_set, interface_object);
}
