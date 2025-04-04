#pragma once

#include <d3d12.h>
#include <d2d1_3.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d11on12.h>
#include "FrameResourceFx.h"

// this will only call release if an object exists (prevents exceptions calling release on non existent objects)
#define SAFE_RELEASE(p) { if ( (p) ) { (p)->Release(); (p) = nullptr; } }

// Adapted from Luna Dx12 Box6 project d3dApp.h without window instance 
class Core12Base
{
protected:

	Core12Base();
	Core12Base(const Core12Base& rhs) = delete;
	Core12Base& operator=(const Core12Base& rhs) = delete;
	virtual ~Core12Base();

public:
	HWND MainWnd()const;
	float AspectRatio()const;

	bool Get4xMsaaState()const;
	void Set4xMsaaState(bool value);

	virtual bool Initialize(HWND mainWnd, int width, int height);
	virtual void Resize(int width, int height);
	virtual void Draw() = 0;
	CSize GetClientSize();

protected:
	virtual void CreateRtvAndDsvDescriptorHeaps();

	bool InitDirect3D();	
	void CreateCommandObjects();
	void CreateSwapChain();

	void InitDx11OnDx12();
	void CreateD2Write();
	void CreateSelectedRectBrush();

	void FlushCommandQueue();

	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

	void DrawImage(const CString& filename, const GUID& ext);

	void ShuttingDown();
	virtual void ClearResources();

	void ClearD11Context();
	void PrepForResizing();
	void CreateWrappedBackBuffers();

protected:

	HWND m_hWnd = nullptr;			// main window handle
	bool m_Resizing = false;

	// Set true to use 4X MSAA (§4.1.8).  The default is false.
	bool m_4xMsaaState = false;		// 4X MSAA enabled
	UINT m_4xMsaaQuality = 0;		// quality level of 4X MSAA


	Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> m_d3dDevice;

	// for drawing 2D	
	Microsoft::WRL::ComPtr<ID2D1Factory3> m_d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Device2> m_d2dDevice;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
	Microsoft::WRL::ComPtr<IDWriteFactory> m_dWriteFactory;
	Microsoft::WRL::ComPtr<ID3D11On12Device> m_d3d11On12Device;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_selectedRectBrush;

	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_currentFence = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_directCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

	static const int SwapChainBufferCount = 2;
	int m_currBackBuffer = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_swapChainBuffer[SwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11Resource> m_wrappedBackBuffers[SwapChainBufferCount];	// for drawing 2D
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> m_d2dRenderTargets[SwapChainBufferCount];		// for drawing 2D

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_PSOs;

	D3D12_VIEWPORT m_screenViewport;
	D3D12_RECT m_scissorRect;

	UINT m_rtvDescriptorSize = 0;
	UINT m_dsvDescriptorSize = 0;
	UINT m_cbvSrvUavDescriptorSize = 0;

	std::vector<std::unique_ptr<DxSupport::FrameResourceFx>> m_frameResources;
	DxSupport::FrameResourceFx* m_currFrameResource = nullptr;
	int m_currFrameResourceIndex = 0;

	D3D_DRIVER_TYPE m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT m_backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int m_clientWidth = 800;
	int m_clientHeight = 600;
};
