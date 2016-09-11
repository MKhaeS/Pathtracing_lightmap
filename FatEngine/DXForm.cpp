//#include "DXForm.h"
#include "DXForm.h"
#include "FatDXFramework.h"
#include "dxgi.h"
#include<functional>
#include "DirectXMath.h"

using namespace DirectX;

DXForm::DXForm()
{
	CreateForm();     
}

void DXForm::InitComponents ()
{
    width_   = GetSystemMetrics(SM_CXSCREEN) - 100;
    height_  = GetSystemMetrics (SM_CYSCREEN) - 100;
    pos_x_   = 50;
    pos_y_   = 50;
	auto CloseButton = std::make_shared<Button>(Button( width_ - 30, 10, 20, 20, { 0.1f, 0.8f,0.2f } ));
	CloseButton->OnMouseClick += [this](WPARAM wParam, LPARAM lParam) { OnCloseButtonMouseClick (wParam, lParam); };
    AddComponent (CloseButton);
    OnKeyDown +=  [this](WPARAM wParam, LPARAM lParam) { OnKeyDownHandler (wParam, lParam); };
}

void DXForm::OnKeyDownHandler (WPARAM wParam, LPARAM lParam) {
    if (wParam == VK_ESCAPE)
        PostQuitMessage (0);
}

void DXForm::OnCloseButtonMouseClick (WPARAM wParam, LPARAM lParam) {
    PostQuitMessage (0);
}
