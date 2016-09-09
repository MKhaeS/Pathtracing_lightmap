#include "GlobalHeader.h"
#include "Form.h"
#include <thread>
#include <Windowsx.h>


Form::Form () {
	appInstance_ = Application::GetInstance ();
    DxFramework  = Application::DxFrameWork;
}

void Form::UpdateView () {
    int g = DxFramework->CreateGeometry ();
	int i = 0;
	for (auto c : m_ComponentsList) {
		auto vertex_data = c->GetVertexColorData (i, width_, height_, 0.4f);		
		++i;        
        DxFramework->AddGeometry (g, *vertex_data);
	}
    int view;
    int buffer = DxFramework->CreateBufferFromGeometry (g, &view);    
}

void Form::Update () {
    
}

bool Form::InitDxContext (HWND hWnd) {
    
    DxFramework->Initialize (hWnd_, width_, height_, DXGI_FORMAT_R8G8B8A8_UNORM);
    int root_signature = DxFramework->CreateRootSignature ();
    int vertex_shader = DxFramework->
        CreateVertexShaderFromCso (L"C:\\Users\\khaes\\Documents\\Visual Studio 2015\\Projects\\FatEngine\\bin\\x64\\Debug\\VS.cso");
    int pixel_shader = DxFramework->
        CreatePixelShaderFromCso (L"C:\\Users\\khaes\\Documents\\Visual Studio 2015\\Projects\\FatEngine\\bin\\x64\\Debug\\PS.cso");
    int rasterizer_desc = DxFramework->CreateRasterizerDescription (D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, false, 1);
    int layout = DxFramework->CreatePositionColorLayout ();
    int pso = DxFramework->CreatePipelineStateObject (root_signature, vertex_shader, pixel_shader, rasterizer_desc, layout, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    return false;
}

void Form::OnMouseClickHandler (WPARAM wParam, LPARAM lParam) {
    int xpos = GET_X_LPARAM (lParam);
    int ypos = GET_Y_LPARAM (lParam);
    for (auto c : m_ComponentsList) {
        if (c->OnMouseClick.GetNumOfFunctions () > 0) {
            if (xpos > c->m_Pos_x_ && xpos < c->m_Pos_x_ + c->m_Width_ &&
                ypos > c->m_Pos_y_ && ypos < c->m_Pos_y_ + c->m_Height_) {
                c->OnMouseClick (wParam, lParam);
            }

        }
    }
}

void Form::ShowOnScreen () const {
	ShowWindow (hWnd_, SHOW_OPENWINDOW);
}

bool Form::CreateForm () {
    InitComponents ();
	wc_.lpszClassName      = "BasicWndNoCaption";
	wc_.style              = CS_HREDRAW | CS_VREDRAW;
	wc_.hInstance          = appInstance_;
	wc_.lpfnWndProc        = WndProc;
	wc_.cbClsExtra         = 0;
	wc_.cbWndExtra         = 0;
	wc_.hCursor            = LoadCursor (0, IDC_ARROW);
	wc_.hIcon              = LoadIcon (0, IDI_APPLICATION);
	wc_.hbrBackground      = (HBRUSH)GetStockObject (WHITE_BRUSH);
	wc_.lpszMenuName       = nullptr;

	if (!RegisterClass (&wc_))
		return false;

	hWnd_ = CreateWindow ("BasicWndNoCaption",
		"FatEngine",
		WS_POPUP,
		pos_x_, pos_y_,
		width_, height_,
		0, 0, appInstance_, 0);
	if (!hWnd_) {
		return false;
	}
	Application::AddNewForm (this);
	ShowOnScreen ();

    OnMouseClick += [this](WPARAM wParam, LPARAM lParam) { OnMouseClickHandler (wParam, lParam); };
    
    InitDxContext (hWnd_);
    UpdateView ();
	return true;
}

HWND Form::GetHWND () {
	return hWnd_;
}

Form::~Form () {
	Application::RemoveForm (this);
}

void Form::AddComponent (std::shared_ptr<Component> component) {
	m_ComponentsList.push_back (component);    
}