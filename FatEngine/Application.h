#pragma once

#include <Windows.h>

#include <vector>
#include <map>
#include <memory>

#include "Form.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

class Application
{
public:
	static int WINAPI				WinMain(HINSTANCE, HINSTANCE, PSTR, int);
	static HINSTANCE				GetInstance();
	static void						AddNewForm(Form*);
	static void						RemoveForm(Form*);

    static const std::shared_ptr<FatDXFramework> DxFrameWork;

private:
	static HINSTANCE				hInstance;
	static std::map<HWND, Form*>	formHandleList;

	static void						Run();
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    
};