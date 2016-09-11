#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <fstream>

#include "EventHandler.h"
#include "Button.h"


class Form {
	Form(const Form&)				= delete;
public:    
    HWND GetHWND ();
protected:
	int width_	= 512;
	int height_	= 512;
	int pos_x_	= 100;
	int pos_y_	= 100;

	EventHandler OnKeyDown;
	EventHandler OnKeyUp;
	EventHandler OnMouseClick;
	EventHandler OnMouseMove;

	void			ShowOnScreen() const;
	virtual bool	CreateForm() final;	
	virtual void InitComponents () = 0;   

	Form();
	~Form();

	void AddComponent (std::shared_ptr<Component> component);
    virtual void UpdateView () final;

    std::shared_ptr<FatDXFramework> DxFramework;
	
private:
	HWND		hWnd_			= 0;
	WNDCLASS	wc_				= { 0 };
	HINSTANCE	appInstance_	= 0;

	std::vector<std::shared_ptr<Component>> m_ComponentsList;

		
	virtual void Update () final;

	friend class Application;
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	std::vector<float> m_ComponentsVertexData_;

    

    bool InitDxContext (HWND hWnd);

    void OnMouseClickHandler (WPARAM wParam, LPARAM lParam);

	//Inreface

	int color_root_signature = 0;
	int color_pso = 0;
	int texture_root_signature = 0;
	int texture_pso = 0;
	int rasterizer_desc = 0;
	int color_buffer_view = 0;
	int texture_buffer_view = 0;

	void CreateContextForColorShader();
	void CreateContextForTextureShader();

};

