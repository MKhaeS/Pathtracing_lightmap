#pragma once
//#include "Form.h"
#include <Windows.h>
#include "Form.h"
#include <memory>
#include "FatDXFramework.h"

using namespace std;

class DXForm final : private Form
{
public:
	DXForm();
    using Form::GetHWND;

private:
    int objects_set;

    std::unique_ptr<FatDXFramework> dxContext;
	virtual void InitComponents () override;
    void OnKeyDownHandler (WPARAM wParam, LPARAM lParam);

    void OnCloseButtonMouseClick (WPARAM wParam, LPARAM lParam);
    void OnMouseClick (WPARAM wParam, LPARAM lParam);

    virtual void UserRender () override;
    std::shared_ptr<FatDXFramework::MatrixBuffer> mat_buffer;
	shared_ptr<Texture> dex_texture;
    void InitScene ();
};

