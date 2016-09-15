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
    int num_of_verts = 0;
    int stride = 0;
	for (auto c : m_ComponentsList) {
        int n = 0;
        auto vertex_data = c->GetVertexColorData ( i, width_, height_, 0.4f, &n, &stride );
		++i;        
        DxFramework->AddDataToGeometry (g, *vertex_data);
        num_of_verts += n;
	}
    int buffer_view = DxFramework->CreateBufferFromGeometry (g, 28, &color_buffer_view );
    auto interface_object = std::make_shared<FatDXFramework::Static3DObject>();
    {
        interface_object->NumOfVerts = num_of_verts;
        interface_object->VertexBufferView = buffer_view;
        interface_object->Visible = true;
        interface_object->RootSignature = color_root_signature;
        interface_object->Pso = color_pso;
    }

    DxFramework->AddStaticObject(interface_objects_set, interface_object );
}

void Form::SetBackPlainRenderTarget () {
    DxFramework->SetRenderTarget (back_texture, XMFLOAT4 (0.0f, 0.0f, 0.0f, 0.0f));
}

void Form::Update () {
    UserRender ();
    int objs[] ={ interface_objects_set };
    DxFramework->Render (objs, 1);
}

bool Form::InitDxContext (HWND hWnd) {    
    DxFramework->Initialize (hWnd_, width_, height_, DXGI_FORMAT_B8G8R8A8_UNORM);   
	rasterizer_desc = DxFramework->CreateRasterizerDescription ( D3D12_FILL_MODE_SOLID, 
																 D3D12_CULL_MODE_NONE, 
																 false, 1 );
	CreateContextForColorShader();
	CreateContextForTextureShader();
	float w = 0.5f;
	std::vector<float> render_plane_data = {
		-w,  w, 1.0f,  0.0f, 0.0f,
		w,  w, 1.0f,  1.0f, 0.0f,
		-w, -w, 1.0f,  0.0f, 1.0f,
		w, -w, 1.0f,  1.0f, 1.0f,
		 -w, -w, 1.0f, 0.0f, 1.0f,
		 w,  w, 1.0f,  1.0f,  0.0f
	};
	int g = DxFramework->CreateGeometry ();
	DxFramework->AddDataToGeometry ( g, render_plane_data );
	int buffer_view = DxFramework->CreateBufferFromGeometry ( g, 20, &texture_buffer_view );
    interface_objects_set = DxFramework->CreateStaticObjectsSet();

    back_texture = make_shared<Texture> (DxFramework->GetDevice (), D3D12_HEAP_TYPE_DEFAULT,
                                         width_, height_, true, true, 1);
    
    mat_buffer = std::make_shared<FatDXFramework::MatrixBuffer> (DxFramework);

    XMMATRIX tr = XMMatrixTranslation (0.0f, 0.0f, 0.0f);
    XMMATRIX view = XMMatrixPerspectiveFovLH (XM_PI / 2, (float)width_ / height_, 0.1f, 1000.0f);
    FXMVECTOR camera_position = { 0.0f, 0.0f, -2.0f, 0.0f};
    FXMVECTOR look ={ 0.5f,0.0f, 1.0f, 0.0f };
    FXMVECTOR up ={ 0.0f,1.0f,0.0f,0.0f };
    XMMATRIX lookat = XMMatrixLookAtLH (camera_position, look, up);
    (*mat_buffer)[0] *= tr * lookat * view;

	FatDXFramework::RootParameter rp;
	{
		rp.Texture = back_texture;
	}


    FatDXFramework::RootParameter rp_matrix;
    {
        rp_matrix.Type = D3D12_ROOT_PARAMETER_TYPE_CBV;
        rp_matrix.Slot = 1;
        rp_matrix.ConstantBuffer = mat_buffer->cb ()->GetGPUVirtualAddress ();
    }
    mat_buffer->Upload ();
    auto interface_object = std::make_shared<FatDXFramework::Static3DObject>();
    {
        interface_object->NumOfVerts = 6;
        interface_object->VertexBufferView = buffer_view;
        interface_object->Visible = true;
        interface_object->RootSignature = texture_root_signature;
        interface_object->Pso = texture_pso;
		interface_object->RootParameters.push_back( rp );
        interface_object->RootParameters.push_back (rp_matrix);
    }

    DxFramework->AddStaticObject( interface_objects_set, interface_object );

	//DxFramework->LoadTextureFromDds (back_texture, L"C:\\Users\\khaes\\Documents\\GitHub\\Pathtracing_lightmap\\bin\\x64\\Debug\\textures\\Dexter.dds");

    return true;
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

void Form::CreateContextForColorShader() {
	color_root_signature = DxFramework->CreateRootSignature ();
	int vertex_shader = DxFramework->
		CreateVertexShaderFromCso ( L"C:\\Users\\khaes\\Documents\\GitHub\\Pathtracing_lightmap\\bin\\x64\\Debug\\VS.cso" );
	int pixel_shader = DxFramework->
		CreatePixelShaderFromCso ( L"C:\\Users\\khaes\\Documents\\GitHub\\Pathtracing_lightmap\\bin\\x64\\Debug\\PS.cso" );
	rasterizer_desc = DxFramework->CreateRasterizerDescription ( D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, false, 1 );
	int layout = DxFramework->CreatePositionColorLayout ();
	color_pso = DxFramework->CreatePipelineStateObject ( color_root_signature,
														 vertex_shader,
														 pixel_shader,
														 rasterizer_desc,
														 layout,
														 D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE );
}

void Form::CreateContextForTextureShader() {
	texture_root_signature = DxFramework->CreateTextureRootSignature();
	int vertex_shader = DxFramework->
		CreateVertexShaderFromCso ( L"C:\\Users\\khaes\\Documents\\GitHub\\Pathtracing_lightmap\\bin\\x64\\Debug\\VS_tex.cso" );
	int pixel_shader = DxFramework->
		CreatePixelShaderFromCso ( L"C:\\Users\\khaes\\Documents\\GitHub\\Pathtracing_lightmap\\bin\\x64\\Debug\\PS_tex.cso" );
	int layout = DxFramework->CreatePositionTextureLayout();
	texture_pso = DxFramework->CreatePipelineStateObject ( texture_root_signature,
                                                           vertex_shader,
														   pixel_shader,
														   rasterizer_desc,
														   layout,
														   D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE );
}

void Form::ShowOnScreen () const {
	ShowWindow (hWnd_, SHOW_OPENWINDOW);
}

bool Form::CreateForm () {
    
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
    InitComponents ();
    
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