#include "Window.h"

#include "Logger.h"

#include "backends/imgui_impl_win32.cpp"
#include "backends/imgui_impl_win32.h"

#include "backends/imgui_impl_dx11.cpp"
#include "backends/imgui_impl_dx11.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

Window::Window(std::string name, int width, int height)
{
	Initialize(name, width, height);
}

Window::~Window()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
	CleanupD3D();
}

void Window::Initialize(std::string name, int width, int height)
{
	m_Width = width; 
	m_Height = height;

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"SpotiVolClientWindowClass";
	RegisterClass(&wc);
	m_WindowHandle = CreateWindowEx(
		0,
		wc.lpszClassName,
		std::wstring(name.begin(), name.end()).c_str(),
		WS_POPUP | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, NULL, wc.hInstance, NULL
	);

	if (!m_WindowHandle)
	{
		Logger::Error("Failed to create window");
		return;
	}

	SetWindowLongPtr(m_WindowHandle, GWLP_USERDATA, (LONG_PTR)this);


	if (m_WindowHandle)
	{
		ShowWindow(m_WindowHandle, SW_SHOW);
	}

	if (!InitD3D())
	{
		Logger::Error("Failed to initialize D3D11");
	}

	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(m_WindowHandle);
	ImGui_ImplDX11_Init(m_Device, m_DeviceContext);

	printf("Window created, D3D initialized\n");
}

void Window::Update()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if(msg.message == WM_QUIT)	
			m_ShouldClose = true;

	}
}

void Window::Destroy()
{
	m_ShouldClose = true;
	CleanupRenderTarget();
	CleanupD3D();
	DestroyWindow(m_WindowHandle);
}

bool Window::InitD3D()
{
	HRESULT hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&m_Device,
		nullptr,
		&m_DeviceContext
	);

	if (FAILED(hr))
	{
		Logger::Error("Failed to create D3D11 device: {}", hr);
		return false;
	}

	DXGI_SWAP_CHAIN_DESC1  swapChainDesc = {};
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Width = m_Width;
	swapChainDesc.Height = m_Height;

	IDXGIFactory2* dxgiFactory = 0;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	if (FAILED(hr))
	{
		Logger::Error("Couldn't create DXGIFactory");
		return false;
	}

	hr = dxgiFactory->CreateSwapChainForHwnd(m_Device, m_WindowHandle, &swapChainDesc, NULL, NULL, &m_SwapChain);

	if(FAILED(hr))
	{
		Logger::Error("Couldn't create swap chain");
		return false;
	}

	CreateRenderTarget();
	return true;
}

void Window::CleanupD3D()
{
	if (m_SwapChain)
	{
		m_SwapChain->Release();
		m_SwapChain = nullptr;
	}
	if (m_DeviceContext)
	{
		m_DeviceContext->Release();
		m_DeviceContext = nullptr;
	}
	if (m_Device)
	{
		m_Device->Release();
		m_Device = nullptr;
	}
}

void Window::CreateRenderTarget()
{
	HRESULT hr = m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&m_BackBuffer));

	if (FAILED(hr))
	{
		Logger::Error("Couldn't get back buffer");
		return;
	}

	hr = m_Device->CreateRenderTargetView(m_BackBuffer, NULL, &m_RenderTargetView);

	if (FAILED(hr))
	{
		Logger::Error("Couldn't create render target view");
		return;
	}
	m_BackBuffer->Release();
}

void Window::CleanupRenderTarget()
{
	if (m_RenderTargetView)
	{
		m_RenderTargetView->Release();
		m_RenderTargetView = nullptr;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	if(!window)
		return DefWindowProc(hwnd, msg, wParam, lParam);

	switch (msg)
	{
	case WM_SIZE:
		if (window->GetDevice() && wParam != SIZE_MINIMIZED)
		{
			window->CleanupRenderTarget();

			uint32_t newWidth = LOWORD(lParam);
			uint32_t newHeight = HIWORD(lParam);

			window->GetSwapChain()->ResizeBuffers(0,
				newWidth,
				newHeight,
				DXGI_FORMAT_UNKNOWN,
				0);
			window->CreateRenderTarget();
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	// making the window draggable
	case WM_NCHITTEST:
	{
		if (ImGui::IsAnyItemHovered())
			return HTCLIENT;

		return HTCAPTION;
	}
	case WM_QUIT:
	{
		Logger::Error("Got quit message");
		window->Destroy();
	}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Window::NewFrame()
{
	// Start frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Window::PresentFrame()
{
	ImGui::Render();

	float clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, NULL);
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView, clear_color);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	m_SwapChain->Present(1, 0);
}
