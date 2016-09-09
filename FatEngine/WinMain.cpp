#include "WinMain.h"
#include "Application.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pszCmdLine, int nCmdShow) {	
    SetThreadAffinityMask (GetCurrentThread (), 1);
	return Application::WinMain(hInstance, hPrevInstance, pszCmdLine, nCmdShow);		
}

