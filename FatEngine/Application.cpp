#include <thread>
#include <algorithm>
#include <memory>

#include "GlobalHeader.h"
#include "Application.h"
#include "DXForm.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT MSG, WPARAM wParam, LPARAM lParam)
{
	switch (MSG)
	{
	case WM_KEYDOWN:
		Application::formHandleList[hWnd]->OnKeyDown(wParam, lParam);
		break;
    case WM_LBUTTONDOWN:
        Application::formHandleList[hWnd]->OnMouseClick (wParam, lParam);
        break;
	}
    
	return DefWindowProc(hWnd, MSG, wParam, lParam);
}


int WINAPI Application::WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pszCmdLine, int nCmdShow)
{
	Application::hInstance	= hInstance;
	auto dxMainForm				= std::make_unique<DXForm>();    
	Run();
	return 0;
}

HINSTANCE Application::GetInstance()
{
	return Application::hInstance;
}

void Application::AddNewForm(Form* newForm)
{
	formHandleList.insert({newForm->hWnd_, newForm });
}

void Application::RemoveForm(Form* oldForm)
{
	formHandleList.erase(oldForm->hWnd_);
}

void Application::Run()
{	
	MSG msg = { 0 };
	if (!formHandleList.empty()) {
		while (msg.message != WM_QUIT) {
			for (const auto& e : formHandleList) {
				if (int i = PeekMessage(&msg, e.first, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
                else {
                    e.second->Update ();
                }
			}
		}
	}
}

HINSTANCE Application::hInstance					= nullptr;
std::map<HWND, Form*> Application::formHandleList	= {};
const std::shared_ptr<FatDXFramework> Application::DxFrameWork = std::make_shared<FatDXFramework> ();