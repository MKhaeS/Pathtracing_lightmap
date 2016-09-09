#pragma once
//#include "Form.h"
#include <Windows.h>
#include "Form.h"
#include <memory>
#include "FatDXFramework.h"

class DXForm final : private Form
{
public:
	DXForm();
    virtual void Update () override;
    using Form::GetHWND;

private:
    std::unique_ptr<FatDXFramework> dxContext;
	virtual void InitComponents () override;
    void OnKeyDownHandler (WPARAM wParam, LPARAM lParam);

    void OnCloseButtonMouseClick (WPARAM wParam, LPARAM lParam);
    void OnMouseClick (WPARAM wParam, LPARAM lParam);
};

