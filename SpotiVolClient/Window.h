#pragma once
#include <windows.h>
#include <string>

#include <d3d11.h>
#include <dxgi1_2.h>

class Window
{
public:
	Window() = default;
	Window(std::string name, int width, int height);
	~Window();
	void Initialize(std::string name, int width, int height);
	void Update();
	void Destroy();
	bool ShouldClose() const { return m_ShouldClose; }

	float GetWindowWidth() { return m_Width; }
	float GetWindowHeight() { return m_Height; }

	void NewFrame();
	void PresentFrame();

	ID3D11Device* GetDevice() const { return m_Device; }
	ID3D11DeviceContext* GetDeviceContext() const { return m_DeviceContext; }
	ID3D11RenderTargetView* GetRenderTargetView() const { return m_RenderTargetView; }
	IDXGISwapChain1* GetSwapChain() const { return m_SwapChain; }
	void CleanupD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
private:
	HWND m_WindowHandle;
	int m_Width, m_Height;
	bool m_ShouldClose = false;

private:
	bool InitD3D();

	ID3D11Texture2D* m_BackBuffer = nullptr;
	ID3D11Device* m_Device = nullptr;
	ID3D11DeviceContext* m_DeviceContext = nullptr;
	IDXGISwapChain1* m_SwapChain = nullptr;
	ID3D11RenderTargetView* m_RenderTargetView = nullptr;
};

