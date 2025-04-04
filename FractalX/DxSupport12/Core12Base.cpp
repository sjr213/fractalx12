
#include "pch.h"
#include "Core12Base.h"

#include "d3dUtil.h"
#include <d3d11on12.h>
#include <dwrite.h>
#include <../packages/directxtk12_desktop_2017.2021.11.8.1/include/ScreenGrab.h>
#include <wincodec.h>

using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;
using namespace DxSupport;

Core12Base::Core12Base()
	: m_screenViewport()
	, m_scissorRect()
{}

Core12Base::~Core12Base()
{}

void Core12Base::ShuttingDown()
{
	if (!m_d3dDevice)
		return;

#if defined(DEBUG) || defined(_DEBUG) 
	ID3D12DebugDevice* debugDevice = NULL;
	m_d3dDevice->QueryInterface(&debugDevice);
	ClearResources();
	debugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
	debugDevice = nullptr;
#else
	ClearResources();
#endif
}

void Core12Base::ClearResources()
{
	BOOL fs = false;

	if (m_swapChain)
	{
		if (SUCCEEDED(m_swapChain->GetFullscreenState(&fs, NULL)))
			m_swapChain->SetFullscreenState(false, NULL);
	}

	ThrowIfFailed(m_commandList->Reset(m_directCmdListAlloc.Get(), nullptr));

	m_swapChain.Reset();

	for (int i = 0; i < SwapChainBufferCount; ++i)
	{
		m_swapChainBuffer[i].Reset();
		m_wrappedBackBuffers[i].Reset();	
		m_d2dRenderTargets[i].Reset();		
	}

	m_swapChain = nullptr;
	m_depthStencilBuffer = nullptr;

	m_fence = nullptr;

	for (auto& frameResource : m_frameResources)
	{
		if (frameResource)
			frameResource.reset();
	}

	m_commandQueue = nullptr;
	m_rtvHeap = nullptr;
	m_commandList = nullptr;
	m_directCmdListAlloc = nullptr;
	
	m_PSOs.clear();

	m_dsvHeap = nullptr;
}

HWND Core12Base::MainWnd()const
{
	return m_hWnd;
}
float Core12Base::AspectRatio()const
{
	return static_cast<float>(m_clientWidth) / m_clientHeight;
}

bool Core12Base::Get4xMsaaState()const
{
	return m_4xMsaaState;
}

void Core12Base::Set4xMsaaState(bool value)
{
	if (m_4xMsaaState != value)
	{
		m_4xMsaaState = value;

		// Recreate the swapchain and buffers with new multisample settings.
		CreateSwapChain();
		Resize(m_clientWidth, m_clientWidth);
	}
}

bool Core12Base::Initialize(HWND mainWnd, int width, int height)
{
	m_clientWidth = max(1, width);
	m_clientHeight = max(1, height);

	m_hWnd = mainWnd;

	if (m_d3dDevice == nullptr)
	{
		if (!InitDirect3D())
			return false;
	}

	// Do the initial resize code.
	Resize(width, height);

	return true;
}

void Core12Base::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())));


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(m_dsvHeap.GetAddressOf())));
}

CSize Core12Base::GetClientSize()
{
	return CSize(m_clientWidth, m_clientHeight);
}

void Core12Base::ClearD11Context()
{
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	m_d3d11DeviceContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
	m_d2dDeviceContext->SetTarget(nullptr);
	m_d3d11DeviceContext->ClearState();
	m_d3d11DeviceContext->Flush();
}

void Core12Base::PrepForResizing()
{
	// Flush before changing any resources.
	FlushCommandQueue();

	ThrowIfFailed(m_commandList->Reset(m_directCmdListAlloc.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < SwapChainBufferCount; ++i)
	{
		auto swapChainCount = m_swapChainBuffer[i].Reset();
		while (swapChainCount > 0)
			swapChainCount = m_swapChainBuffer[i].Reset();

		auto wrappedBackBufCount = m_wrappedBackBuffers[i].Reset();
		while (wrappedBackBufCount > 0)
			wrappedBackBufCount = m_wrappedBackBuffers[i].Reset();

		auto renderTargetCount = m_d2dRenderTargets[i].Reset();
		while (renderTargetCount > 0)
			renderTargetCount = m_d2dRenderTargets[i].Reset();
	}
	auto depthStencilCount = m_depthStencilBuffer.Reset();
	assert(depthStencilCount == 0);

	ClearD11Context();
}

void Core12Base::CreateWrappedBackBuffers()
{
	//////////////////////////////////////
	// Query the desktop's dpi settings, which will be used to create
	// D2D's render targets.

	auto dpi = GetDpiForWindow(m_hWnd);
	ASSERT(dpi > 0);
	float dpiX = static_cast<float>(dpi);
	float dpiY = static_cast<float>(dpi);
	// m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpiX,
		dpiY
	);

	//////////////////////////////////////

	m_currBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffer[i])));
		m_d3dDevice->CreateRenderTargetView(m_swapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_rtvDescriptorSize);

		//////////////////////////////////////
		// Create a wrapped 11On12 resource of this back buffer. Since we are 
		// rendering all D3D12 content first and then all D2D content, we specify 
		// the In resource state as RENDER_TARGET - because D3D12 will have last 
		// used it in this state - and the Out resource state as PRESENT. When 
		// ReleaseWrappedResources() is called on the 11On12 device, the resource 
		// will be transitioned to the PRESENT state.
		D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
		ThrowIfFailed(m_d3d11On12Device->CreateWrappedResource(
			m_swapChainBuffer[i].Get(),
			&d3d11Flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(&m_wrappedBackBuffers[i])
		));

		// Create a render target for D2D to draw directly to this back buffer.
		ComPtr<IDXGISurface> surface;
		ThrowIfFailed(m_wrappedBackBuffers[i].As(&surface));
		ThrowIfFailed(m_d2dDeviceContext->CreateBitmapFromDxgiSurface(
			surface.Get(),
			&bitmapProperties,
			&m_d2dRenderTargets[i]
		));
	}
}

void Core12Base::Resize(int width, int height)
{
	assert(m_d3dDevice);
	assert(m_swapChain);
	assert(m_directCmdListAlloc);

	m_clientWidth = max(1, width);
	m_clientHeight = max(1, height);

	PrepForResizing();

	// Resize the swap chain.
	ThrowIfFailed(m_swapChain->ResizeBuffers(
		SwapChainBufferCount,
		m_clientWidth, m_clientHeight,
		m_backBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	CreateWrappedBackBuffers();

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_clientWidth;
	depthStencilDesc.Height = m_clientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_depthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(m_depthStencilBuffer.GetAddressOf())));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_depthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, DepthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_depthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	m_commandList->ResourceBarrier(1, &barrier);

	// Execute the resize commands.
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	FlushCommandQueue();

	// Update the viewport transform to cover the client area.
	m_screenViewport.TopLeftX = 0;
	m_screenViewport.TopLeftY = 0;
	m_screenViewport.Width = static_cast<float>(m_clientWidth);
	m_screenViewport.Height = static_cast<float>(m_clientHeight);
	m_screenViewport.MinDepth = 0.0f;
	m_screenViewport.MaxDepth = 1.0f;

	m_scissorRect = { 0, 0, m_clientWidth, m_clientHeight };
}

bool Core12Base::InitDirect3D()
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_d3dDevice));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_d3dDevice)));
	}

	ThrowIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&m_fence)));

	m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_cbvSrvUavDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Check 4X MSAA quality support for our back buffer format.
	// All Direct3D 11 capable devices support 4X MSAA for all render 
	// target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = m_backBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(m_d3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

#ifdef _DEBUG
	LogAdapters();
#endif

	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();

	InitDx11OnDx12();

	return true;
}

void Core12Base::InitDx11OnDx12()
{
	UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	ComPtr<ID3D11Device> d3d11Device;

	ThrowIfFailed(D3D11On12CreateDevice(m_d3dDevice.Get(),
		flags,
		nullptr,
		0,
		reinterpret_cast<IUnknown**>(m_commandQueue.GetAddressOf()),
		1,
		0,
		&d3d11Device,
		&m_d3d11DeviceContext,
		nullptr
	));

	// Query the 11On12 device from the 11 device.
	ThrowIfFailed(d3d11Device.As(&m_d3d11On12Device));

	CreateD2Write();
	CreateSelectedRectBrush();		
}

void Core12Base::CreateD2Write()
{
	D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
	D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

#if defined(_DEBUG)
	// Enable the D2D debug layer.
	d2dFactoryOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, (void**)&m_d2dFactory));
	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	ThrowIfFailed(m_d3d11On12Device.As(&dxgiDevice));
	ThrowIfFailed(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));
	ThrowIfFailed(m_d2dDevice->CreateDeviceContext(deviceOptions, &m_d2dDeviceContext));
	ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dWriteFactory));
}

void Core12Base::CreateSelectedRectBrush()
{
	ThrowIfFailed(m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_selectedRectBrush));
}

void Core12Base::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_directCmdListAlloc.GetAddressOf())));

	ThrowIfFailed(m_d3dDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_directCmdListAlloc.Get(), // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(m_commandList.GetAddressOf())));

	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	m_commandList->Close();
}

void Core12Base::CreateSwapChain()
{
	// Release the previous swapchain we will be recreating.
	m_swapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_clientWidth;
	sd.BufferDesc.Height = m_clientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = m_backBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = m_hWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(m_dxgiFactory->CreateSwapChain(
		m_commandQueue.Get(),
		&sd,
		m_swapChain.GetAddressOf()));
}

void Core12Base::FlushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	m_currentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (m_fence->GetCompletedValue() < m_currentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, NULL, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_currentFence, eventHandle));

		if (eventHandle == NULL)
			return;

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

ID3D12Resource* Core12Base::CurrentBackBuffer()const
{
	return m_swapChainBuffer[m_currBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Core12Base::CurrentBackBufferView()const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_currBackBuffer,
		m_rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE Core12Base::DepthStencilView()const
{
	return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void Core12Base::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;
	while (m_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		adapterList.push_back(adapter);

		++i;
	}

	for (size_t j = 0; j < adapterList.size(); ++j)
	{
		LogAdapterOutputs(adapterList[j]);
		ReleaseCom(adapterList[j]);
	}
}

void Core12Base::LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		LogOutputDisplayModes(output, m_backBufferFormat);

		ReleaseCom(output);

		++i;
	}
}

void Core12Base::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}

void Core12Base::DrawImage(const CString& filename, const GUID& ext)
{
	auto resource = CurrentBackBuffer();

	auto result = DirectX::SaveWICTextureToFile(
		m_commandQueue.Get(),
		resource,
		ext,
		filename,
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PRESENT);

	ThrowIfFailed(result);
}