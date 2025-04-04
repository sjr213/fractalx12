#include "pch.h"
#include "Core12.h"

#include <afxwin.h>
#include "d3dUtil.h"
#include "MyDxHelpers.h"
#include "MyDxUtil.h"
#include "NarrowCast.h"
#include "GeometryGenerator.h"
#include "RotationParams.h"

using namespace DirectX;
using namespace DXF;
using namespace Microsoft::WRL;

namespace DxSupport
{

	namespace
	{
		static float RoundDegrees(float deg)
		{
			return fmod(deg, 360.0f);
		}

		void ReleaseMesh(MeshGeometry& meshGeo)
		{
			meshGeo.DisposeUploaders();
			SAFE_RELEASE(meshGeo.VertexBufferCPU);
			SAFE_RELEASE(meshGeo.IndexBufferCPU);
			meshGeo.VertexBufferGPU = nullptr;
			meshGeo.IndexBufferGPU = nullptr;
		}

		float GetAverageYvalue(const std::vector<Vertex>& vertices)
		{
			if (vertices.empty())
				return 0.0f;

			float sum = 0.0;
			for (const auto& v : vertices)
				sum += v.Pos.y;

			return sum / vertices.size();
		}
	}

Core12::Core12()
{}

Core12::~Core12()
{
	if (m_d3dDevice != nullptr)
	{
		// The first two calls might not be necessary
		ClearRenderItems();
		Draw();
		FlushCommandQueue();
	}
		
	ShuttingDown();
}

void Core12::ClearResources()
{
	m_allRitems.clear();

	Core12Base::ClearResources();

	SAFE_RELEASE(m_shaders[OpaquePs]);
	SAFE_RELEASE(m_shaders[StdVs]);
	
	if (m_rootSignature)
		m_rootSignature = nullptr;

	auto mainGeo = m_geometries[MainGeo].get();
	if (mainGeo)
		ReleaseMesh(*mainGeo);

	auto backgndGeo = m_geometries[BackgroundGeo].get();
	if (backgndGeo)
		ReleaseMesh(*backgndGeo);

	m_srvDescriptorHeap = nullptr;

	ClearTextures();

	m_geometries.clear();
}

void Core12::ClearTextures()
{
	for (auto& tex : m_textures)
	{
		if (tex.second)
		{
			tex.second->UploadHeap = nullptr;
			tex.second->Resource = nullptr;
		}
	}
}

void Core12::SetVertices(std::vector<Vertex>&& vertices)
{
	m_vertices = vertices;
}

void Core12::SetIndices(std::vector<std::int32_t>&& indices)
{
	m_indices = indices;
}

void Core12::SetTextureColors(std::vector<uint32_t> colors)
{
	m_colors = colors;
}

DXF::Vertex<float> Core12::GetCamera() const
{
	return m_camera;
}

void Core12::SetCamera(const DXF::Vertex<float>& camera)
{
	m_camera = camera;
}

DXF::Vertex<float> Core12::GetTarget() const
{
	return m_target;
}

void Core12::SetTarget(const DXF::Vertex<float>& target)
{
	m_target = target;
}

DXF::Vertex<float> Core12::GetTargetBackground() const
{
	return m_targetBackground;
}

void Core12::SetTargetBackground(const DXF::Vertex<float>& targetBackground)
{
	m_targetBackground = targetBackground;
}

void Core12::SetPerspective(float nearView, float farView)
{
	m_near = nearView;
	m_far = farView;

	m_projMatrix = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), m_near, m_far);
	XMStoreFloat4x4(&m_proj, m_projMatrix);
}

float Core12::GetNearPlane() const
{
	return m_near;
}

float Core12::GetFarPlane() const
{
	return m_far;
}

void Core12::SetRotationGroup(const RotationGroup& rg)
{
	m_rotationGroup = rg;
}

RotationGroup Core12::GetRotationGroup() const
{
	return m_rotationGroup;
}

void Core12::SetBackgroundColor(const DirectX::XMFLOAT4& backgroundColor)
{
	m_backgroundColor = backgroundColor;
}

void Core12::SetAmbientColor(const DirectX::XMFLOAT4& ambientColor)
{
	m_ambientLight = ambientColor;
}

void Core12::SetDiffuseAlbedo(const DirectX::XMFLOAT4& diffuseAlbedo)
{
	m_diffuseAlbedo = diffuseAlbedo;
}

void Core12::SetFresnelR0(const DirectX::XMFLOAT3& fresnelR0)
{
	m_fresnelR0 = fresnelR0;
}

void Core12::SetRoughness(float roughness)
{
	m_roughness = roughness;
}

void Core12::SetLight(int index, DirectX::XMFLOAT3& color, DirectX::XMFLOAT3& direction)
{
	if (index < 0 || index > 2)
	{
		assert(false);
		return;
	}

	m_lights[index].Strength = color;
	m_lights[index].Direction = direction;
}

void Core12::SetWorldScale(const DXF::Vertex<float>& scale) 
{
	m_worldScale = scale;
}

DXF::Vertex<float> Core12::GetWorldScale() const
{
	return m_worldScale;
}

void Core12::SetBackgroundScale(const DXF::Vertex<float>& scale)
{
	m_backgroundScale = scale;
}

DXF::Vertex<float> Core12::GetBackgroundScale() const
{
	return m_backgroundScale;
}

bool Core12::Initialize(HWND mainWnd, int width, int height)
{
	if (!Core12Base::Initialize(mainWnd, width, height)) 
		return false;

	return BuildAll();
}

bool Core12::WasInitialized() const
{
	return MainWnd() != NULL;
}

bool Core12::BuildAll()
{
	if (MainWnd() == NULL)
		return false;

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(m_commandList->Reset(m_directCmdListAlloc.Get(), nullptr));

	// Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
	m_cbvSrvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); 

	LoadTextures();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildMainModelGeometry();
	BuildBackgroundGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSOs();

	// Execute the initialization commands.
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}

void Core12::Resize(int width, int height)
{
	Core12Base::Resize(width, height);

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	m_projMatrix = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), m_near, m_far);
	XMStoreFloat4x4(&m_proj, m_projMatrix);
}

bool Core12::IsReadyToRender() // Need to think about this
{
	return m_mainRitem != nullptr;
}

void Core12::Update(float totalTimeSecs, float deltaTimeSecs)
{
	SetWorlds(totalTimeSecs);
	UpdateCamera();

	// Cycle through the circular frame resource array.
	m_currFrameResourceIndex = (m_currFrameResourceIndex + 1) % g_NumFrameResources;
	m_currFrameResource = m_frameResources[m_currFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	while (m_currFrameResource->Fence != 0 && m_fence->GetCompletedValue() < m_currFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_currFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	UpdateObjectCBs();
	UpdateMaterialCBs();
	UpdateMainPassCB(totalTimeSecs, deltaTimeSecs);
	UpdateReflectedPassCB();	// probably don't currently need this
}

// I don't understand why but we must do this before we change the texture
void Core12::UpdateLight(float totalTimeSecs, float deltaTimeSecs)
{
	for (int x = 0; x < g_NumFrameResources; ++x)
	{
		// Cycle through the circular frame resource array.
		m_currFrameResourceIndex = (m_currFrameResourceIndex + 1) % g_NumFrameResources;
		m_currFrameResource = m_frameResources[m_currFrameResourceIndex].get();

		// Has the GPU finished processing the commands of the current frame resource?
		// If not, wait until the GPU has completed commands up to this fence point.
		while (m_currFrameResource->Fence != 0 && m_fence->GetCompletedValue() < m_currFrameResource->Fence)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
			ThrowIfFailed(m_fence->SetEventOnCompletion(m_currFrameResource->Fence, eventHandle));
			WaitForSingleObject(eventHandle, INFINITE);
			CloseHandle(eventHandle);
		}

		UpdateMaterialCBs();
	}
}

void Core12::Draw()
{
	auto cmdListAlloc = m_currFrameResource->CmdListAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(m_commandList->Reset(cmdListAlloc.Get(), m_PSOs[OpaguePSO].Get()));

	m_commandList->RSSetViewports(1, &m_screenViewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// Indicate a state transition on the resource usage.
	auto backBuffer = CurrentBackBuffer();
	auto barrierToRenderTarget = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrierToRenderTarget);

	// Clear the back buffer and depth buffer.
	m_commandList->ClearRenderTargetView(CurrentBackBufferView(), (float*)&m_mainPassCB.FogColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	auto backBufferDesc = CurrentBackBufferView();
	auto depthStencilDesc = DepthStencilView();
	m_commandList->OMSetRenderTargets(1, &backBufferDesc, true, &depthStencilDesc);

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvDescriptorHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

//	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

	// Draw opaque items--floors, walls, skull.
	auto passCB = m_currFrameResource->PassCB->Resource();
	m_commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
	DrawRenderItems(m_commandList.Get(), m_ritemLayer[(int)RenderLayer::Opaque]);

	// Restore main pass constants and stencil ref.
	m_commandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());
	m_commandList->OMSetStencilRef(0);

	if (m_background.ShowShadow)
	{
		// Draw shadows
		m_commandList->SetPipelineState(m_PSOs[ShadowPSO].Get());
		DrawRenderItems(m_commandList.Get(), m_ritemLayer[(int)RenderLayer::Shadow]);
	}

	// Indicate a state transition on the resource usage.
	auto barrierToPresent = CD3DX12_RESOURCE_BARRIER::Transition(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &barrierToPresent);

	// Done recording commands.
	ThrowIfFailed(m_commandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	RenderSelectedRect();

	// Swap the back and front buffers
	ThrowIfFailed(m_swapChain->Present(0, 0));
	m_currBackBuffer = (m_currBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	m_currFrameResource->Fence = ++m_currentFence;

	// Notify the fence when the GPU completes commands up to this fence point.
	m_commandQueue->Signal(m_fence.Get(), m_currentFence);
}

void Core12::RenderSelectedRect()		// for 2D
{
	if (!m_drawRect)
		return;

	D2D1_SIZE_F rtSize = m_d2dRenderTargets[m_currBackBuffer]->GetSize();

	// Acquire our wrapped render target resource for the current back buffer.
	m_d3d11On12Device->AcquireWrappedResources(m_wrappedBackBuffers[m_currBackBuffer].GetAddressOf(), 1);

	// Render text directly to the back buffer.
	m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[m_currBackBuffer].Get());
	m_d2dDeviceContext->BeginDraw();
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

	D2D1_RECT_F rectangle1 = MyDxUtil::CalculateD2Rect(rtSize, m_relativeSelectedRect);

	m_d2dDeviceContext->DrawRectangle(&rectangle1, m_selectedRectBrush.Get(), 2.0f);

	ThrowIfFailed(m_d2dDeviceContext->EndDraw());

	// Release our wrapped render target resource. Releasing 
	// transitions the back buffer resource to the state specified
	// as the OutState when the wrapped resource was created.
	m_d3d11On12Device->ReleaseWrappedResources(m_wrappedBackBuffers[m_currBackBuffer].GetAddressOf(), 1);

	// Flush to submit the 11 command list to the shared command queue.
	m_d3d11DeviceContext->Flush();
}

void Core12::SetWorlds(float totalTime)
{
	float radians = totalTime / 2;
	float degrees = XMConvertToDegrees(radians);
	float degreesX = -1.0f * degrees;
	auto transMatrix = XMMatrixTranslation(m_target.X, m_target.Y, m_target.Z);

	if (m_rotationGroup.RotationParamsMain.Action == RotationAction::RotateX)
	{
		m_rotationGroup.RotationParamsMain.AngleXDegrees = RoundDegrees(degreesX);
	}
	else if (m_rotationGroup.RotationParamsMain.Action == RotationAction::RotateY)
	{
		m_rotationGroup.RotationParamsMain.AngleYDegrees = RoundDegrees(degrees);
	}
	else if (m_rotationGroup.RotationParamsMain.Action == RotationAction::RotateZ)
	{
		m_rotationGroup.RotationParamsMain.AngleZDegrees = RoundDegrees(degrees);
	}
	else if (m_rotationGroup.RotationParamsMain.Action == RotationAction::RotateXY)
	{
		m_rotationGroup.RotationParamsMain.AngleXDegrees = RoundDegrees(degreesX);
		m_rotationGroup.RotationParamsMain.AngleYDegrees = RoundDegrees(degrees);
	}
	else if (m_rotationGroup.RotationParamsMain.Action == RotationAction::RotateYZ)
	{
		m_rotationGroup.RotationParamsMain.AngleYDegrees = RoundDegrees(degrees);
		m_rotationGroup.RotationParamsMain.AngleZDegrees = RoundDegrees(degrees);
	}
	else if (m_rotationGroup.RotationParamsMain.Action == RotationAction::RotateXZ)
	{
		m_rotationGroup.RotationParamsMain.AngleXDegrees = RoundDegrees(degreesX);
		m_rotationGroup.RotationParamsMain.AngleZDegrees = RoundDegrees(degrees);
	}
	else if (m_rotationGroup.RotationParamsMain.Action == RotationAction::RotateAll)
	{
		m_rotationGroup.RotationParamsMain.AngleYDegrees = RoundDegrees(degrees);
		m_rotationGroup.RotationParamsMain.AngleZDegrees = RoundDegrees(degrees);
		m_rotationGroup.RotationParamsMain.AngleXDegrees = RoundDegrees(degreesX);
	}

	XMMATRIX mainScale = XMMatrixScaling(m_worldScale.X, m_worldScale.Y, m_worldScale.Z);
	XMMATRIX rotateY = XMMatrixRotationY(XMConvertToRadians(m_rotationGroup.RotationParamsMain.AngleYDegrees));
	XMMATRIX rotateX = XMMatrixRotationX(XMConvertToRadians(m_rotationGroup.RotationParamsMain.AngleXDegrees));
	XMMATRIX rotateZ = XMMatrixRotationZ(XMConvertToRadians(m_rotationGroup.RotationParamsMain.AngleZDegrees));
	m_mainWorldMatrix = mainScale * rotateY * rotateX * rotateZ * transMatrix;
	XMStoreFloat4x4(&m_mainRitem->World, m_mainWorldMatrix);

	m_mainRitem->NumFramesDirty = g_NumFrameResources;

	SetBackgroundWorld(m_mainWorldMatrix);
}

void Core12::SetBackgroundWorld(const XMMATRIX& mainWorld)
{
	if (ShouldShowBackground() == false)
		return;

	XMMATRIX backgndWorld = mainWorld;
	if (m_rotationGroup.RotationType != RotationSelectionType::LockBackgroundOnModel)
	{
		auto backgrndTransMatrix = XMMatrixTranslation(m_targetBackground.X, m_targetBackground.Y, m_targetBackground.Z);
		XMMATRIX bckScale = XMMatrixScaling(m_backgroundScale.X, m_backgroundScale.Y, m_backgroundScale.Z);
		XMMATRIX bckRotateY = XMMatrixRotationY(XMConvertToRadians(m_rotationGroup.RotationParamsBackground.AngleYDegrees));
		XMMATRIX bckRotateX = XMMatrixRotationX(XMConvertToRadians(m_rotationGroup.RotationParamsBackground.AngleXDegrees));
		XMMATRIX bckRotateZ = XMMatrixRotationZ(XMConvertToRadians(m_rotationGroup.RotationParamsBackground.AngleZDegrees));
		backgndWorld = bckScale * bckRotateY * bckRotateX * bckRotateZ * backgrndTransMatrix;
	}

	XMStoreFloat4x4(&m_backgroundRitem->World, backgndWorld);
	m_backgroundRitem->NumFramesDirty = g_NumFrameResources;

	if (m_background.ShowShadow == false)
		return;

	XMVECTOR shadowPlane = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // xz plane
	XMVECTOR toMainLight = -XMLoadFloat3(&m_mainPassCB.Lights[0].Direction);
	XMMATRIX S = XMMatrixShadow(shadowPlane, toMainLight);
	XMMATRIX shadowOffsetY = XMMatrixTranslation(0.0f, 0.001f, 0.0f);

	float shadowY = GetAverageYvalue(m_vertices2);
	auto shadowTransMatrix = XMMatrixTranslation(0, shadowY, 0);

	if(m_background.WorldType == ShadowWorldType::Background)
		XMStoreFloat4x4(&m_shadowRitem->World,  S * shadowTransMatrix * shadowOffsetY * backgndWorld);
	else // if WorldType == ShadowWorldType::MainModel
		XMStoreFloat4x4(&m_shadowRitem->World, S * shadowTransMatrix * shadowOffsetY * mainWorld);

	m_shadowRitem->NumFramesDirty = g_NumFrameResources;
}

void Core12::UpdateCamera()
{
	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(m_camera.X, m_camera.Y, m_camera.Z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_viewMatrix = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_view, m_viewMatrix);
}

void Core12::UpdateObjectCBs()
{
	auto currObjectCB = m_currFrameResource->ObjectCB.get();
	for (auto& e : m_allRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		while (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}

void Core12::UpdateMaterialCBs()
{
	auto currMaterialCB = m_currFrameResource->MaterialCB.get();
	for (auto& e : m_materials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void Core12::UpdateMainPassCB(float totalTimeSecs, float deltaTimeSecs)
{
	XMMATRIX view = XMLoadFloat4x4(&m_view);
	XMMATRIX proj = XMLoadFloat4x4(&m_proj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	auto viewDet = XMMatrixDeterminant(view);
	XMMATRIX invView = XMMatrixInverse(&viewDet, view);
	auto projDet = XMMatrixDeterminant(proj);
	XMMATRIX invProj = XMMatrixInverse(&projDet, proj);
	auto viewProjDet = XMMatrixDeterminant(viewProj);
	XMMATRIX invViewProj = XMMatrixInverse(&viewProjDet, viewProj);

	XMStoreFloat4x4(&m_mainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_mainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_mainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_mainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_mainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_mainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	m_mainPassCB.EyePosW = m_eyePos;
	m_mainPassCB.RenderTargetSize = XMFLOAT2((float)m_clientWidth, (float)m_clientHeight);
	m_mainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_clientWidth, 1.0f / m_clientHeight);
	m_mainPassCB.NearZ = m_near;
	m_mainPassCB.FarZ = m_far;
	m_mainPassCB.TotalTime = totalTimeSecs;
	m_mainPassCB.DeltaTime = deltaTimeSecs;
	m_mainPassCB.AmbientLight = m_ambientLight;
	m_mainPassCB.Lights[0] = m_lights[0];
	m_mainPassCB.Lights[1] = m_lights[1];
	m_mainPassCB.Lights[2] = m_lights[2];
	m_mainPassCB.FogColor = m_backgroundColor;

	// Main pass stored in index 2
	auto currPassCB = m_currFrameResource->PassCB.get();
	currPassCB->CopyData(0, m_mainPassCB);
}

void Core12::UpdateReflectedPassCB()
{
	m_reflectedPassCB = m_mainPassCB;

	XMVECTOR mirrorPlane = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f); // xy plane
	XMMATRIX R = XMMatrixReflect(mirrorPlane);

	// Reflect the lighting.
	for (int i = 0; i < 3; ++i)
	{
		XMVECTOR lightDir = XMLoadFloat3(&m_mainPassCB.Lights[i].Direction);
		XMVECTOR reflectedLightDir = XMVector3TransformNormal(lightDir, R);
		XMStoreFloat3(&m_reflectedPassCB.Lights[i].Direction, reflectedLightDir);
	}

	// Reflected pass stored in index 1
	auto currPassCB = m_currFrameResource->PassCB.get();
	currPassCB->CopyData(1, m_reflectedPassCB);
}

void Core12::ReloadTextures()
{
	if (m_d3dDevice == nullptr)
		return;

	// the textures don't exist yet they will be created later during initialization
	// otherwise continue to replace the textures
	if (m_textures.empty())
		return;

	// Cycle through the circular frame resource array.
	m_currFrameResourceIndex = (m_currFrameResourceIndex + 1) % g_NumFrameResources;
	m_currFrameResource = m_frameResources[m_currFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	while (m_currFrameResource->Fence != 0 && m_fence->GetCompletedValue() < m_currFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_currFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	auto cmdListAlloc = m_currFrameResource->CmdListAlloc;

	ThrowIfFailed(cmdListAlloc->Reset());
	ThrowIfFailed(m_commandList->Reset(cmdListAlloc.Get(), nullptr));

	LoadTextures();

	// Execute the initialization commands.
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

void Core12::LoadTextures()
{
	// from https://stackoverflow.com/questions/35568302/what-is-the-d3d12-equivalent-of-d3d11-createtexture2d

	m_textures[MainTexture].reset();
	auto colorTex = std::make_unique<Texture>();
	colorTex->Name = MainTexture;

	// Create the texture.
	UINT textureWidth = DxCore::narrow<UINT>(m_colors.size());
	UINT textureHeight = 1;

	// Describe and create a Texture2D.
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// each element is [0,1]
	textureDesc.Width = textureWidth;				
	textureDesc.Height = textureHeight;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	auto heapPropDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&heapPropDefault,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(colorTex->Resource.ReleaseAndGetAddressOf())));

	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(colorTex->Resource.Get(), 0, 1);

	// Create the GPU upload buffer.
	auto heapPropUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resourceDescr = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
	ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
		&heapPropUpload,
		D3D12_HEAP_FLAG_NONE,
		&resourceDescr,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(colorTex->UploadHeap.ReleaseAndGetAddressOf())));

	// Copy data to the intermediate upload heap and then schedule a copy 
	// from the upload heap to the Texture2D.
	std::vector<UINT8> texture = MyDxUtil::GenerateTextureData(m_colors);

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = &texture[0];
	textureData.RowPitch = textureWidth * sizeof(uint8_t);				// not sure about this
	textureData.SlicePitch = textureData.RowPitch * textureHeight;

	UpdateSubresources(m_commandList.Get(), colorTex->Resource.Get(), colorTex->UploadHeap.Get(), 0, 0, 1, &textureData);
	auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(colorTex->Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_commandList->ResourceBarrier(1, &resourceBarrier);
	
	m_textures[MainTexture] = std::move(colorTex);

	// Background texture
	/////////////////////////////////////////////////////////////////////////////////
	m_textures[BackgroundTexture].reset();

	if (ShouldShowBackground() == false)
		return;

	auto backgroundTex = std::make_unique<Texture>();
	backgroundTex->Name = BackgroundTexture;
	backgroundTex->Filename = m_background.ImageFilename;
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_d3dDevice.Get(),
		m_commandList.Get(), backgroundTex->Filename.c_str(),
		backgroundTex->Resource, backgroundTex->UploadHeap));

	m_textures[BackgroundTexture] = std::move(backgroundTex);
}

void Core12::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Performance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_d3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
}

void Core12::BuildDescriptorHeaps()
{
	bool makeBackgroundTextureHeap = ShouldShowBackground();
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = makeBackgroundTextureHeap ? 2 : 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(m_srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto colorTex = m_textures[MainTexture]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = colorTex->GetDesc().Format;						// might need to hard code this to DXGI_FORMAT_R8G8B8A8_UNORM
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = DxCore::narrow_cast<UINT>(-1);
	m_d3dDevice->CreateShaderResourceView(colorTex.Get(), &srvDesc, hDescriptor);

	if (!makeBackgroundTextureHeap)
		return;

	hDescriptor.Offset(1, m_cbvSrvDescriptorSize);

	auto backgroundTex = m_textures[BackgroundTexture]->Resource;
	srvDesc.Format = backgroundTex->GetDesc().Format;
	m_d3dDevice->CreateShaderResourceView(backgroundTex.Get(), &srvDesc, hDescriptor);
}

static std::wstring GetModulePath()
{
	TCHAR fullpath[_MAX_PATH];
	GetModuleFileName(NULL, fullpath, _MAX_PATH);

	wchar_t path[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];

	errno_t err = _wsplitpath_s(fullpath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, ext, _MAX_EXT);
	if (err != 0)
	{
		AfxMessageBox(L"Error splitting path, bye!", MB_OK);
		exit(1);
	}

	err = _wmakepath_s(path, _MAX_PATH, drive, dir, nullptr, nullptr);
	if (err != 0)
	{
		AfxMessageBox(L"Error making path, bye!", MB_OK);
		exit(1);
	}

	return std::wstring(path);
}

static std::wstring MakeShaderPath(const std::wstring& folder, const std::wstring& shaderFilename)
{
	auto path = GetModulePath();
	path += folder;
	path += L'\\';
	path += shaderFilename;

	return path;
}

void Core12::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO defines[] =
	{
		"FOG", "1",
		NULL, NULL
	};
/*
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"FOG", "1",
		"ALPHA_TEST", "1",
		NULL, NULL
	};
*/
	m_shaders[StdVs] = d3dUtil::CompileShader(MakeShaderPath(L"Shaders", L"Default1dTexture.hlsl"), nullptr, "VS", "vs_5_0");
	m_shaders[OpaquePs] = d3dUtil::CompileShader(MakeShaderPath(L"Shaders", L"Default1dTexture.hlsl"), defines, "PS", "ps_5_0");
//	m_shaders["alphaTestedPS"] = d3dUtil::CompileShader(L"Shaders\\DefaultStencil.hlsl", alphaTestDefines, "PS", "ps_5_0");

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void Core12::BuildMainModelGeometry()
{
	m_geometries[MainGeo] = nullptr;
	if(m_indices.empty() || m_vertices.empty())
		return;

	// new 
	SubmeshGeometry mainSubmesh;
	mainSubmesh.IndexCount = DxCore::narrow<UINT>(m_indices.size());
	mainSubmesh.StartIndexLocation = 0;
	mainSubmesh.BaseVertexLocation = 0;

	const UINT vbByteSize = DxCore::narrow<UINT>(m_vertices.size() * sizeof(Vertex));
	const UINT ibByteSize = DxCore::narrow<UINT>(m_indices.size() * sizeof(std::uint32_t));

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = MainGeo;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), m_vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), m_indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_commandList.Get(), m_vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_commandList.Get(), m_indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs[MainGeo] = mainSubmesh;

	m_geometries[MainGeo] = std::move(geo);
}

void Core12::BuildBackgroundGeometry()
{
	m_geometries[BackgroundGeo] = nullptr;
	if (m_indices2.empty() || m_vertices2.empty())
		return;

	if (! ShouldShowBackground())
		return;

	SubmeshGeometry mainSubmesh;
	mainSubmesh.IndexCount = DxCore::narrow<UINT>(m_indices2.size());
	mainSubmesh.StartIndexLocation = 0;
	mainSubmesh.BaseVertexLocation = 0;

	const UINT vbByteSize = DxCore::narrow<UINT>(m_vertices2.size() * sizeof(Vertex));
	const UINT ibByteSize = DxCore::narrow<UINT>(m_indices2.size() * sizeof(std::uint32_t));

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = BackgroundGeo;

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), m_vertices2.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), m_indices2.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_commandList.Get(), m_vertices2.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_commandList.Get(), m_indices2.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R32_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs[BackgroundGeo] = mainSubmesh;

	m_geometries[BackgroundGeo] = std::move(geo);
}

void Core12::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//
	// PSO for opaque objects.
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	opaquePsoDesc.pRootSignature = m_rootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_shaders[StdVs]->GetBufferPointer()),
		m_shaders[StdVs]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_shaders[OpaquePs]->GetBufferPointer()),
		m_shaders[OpaquePs]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = m_backBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = m_depthStencilFormat;
	opaquePsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&m_PSOs[OpaguePSO])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;
	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;

	// We are going to draw shadows with transparency, so base it off the transparency description.
	D3D12_DEPTH_STENCIL_DESC shadowDSS;
	shadowDSS.DepthEnable = true;
	shadowDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	shadowDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	shadowDSS.StencilEnable = true;
	shadowDSS.StencilReadMask = 0xff;
	shadowDSS.StencilWriteMask = 0xff;

	shadowDSS.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	// We are not rendering back facing polygons, so these settings do not matter.
	shadowDSS.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	shadowDSS.BackFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
	shadowDSS.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC shadowPsoDesc = transparentPsoDesc;
	shadowPsoDesc.DepthStencilState = shadowDSS;
	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&shadowPsoDesc, IID_PPV_ARGS(&m_PSOs[ShadowPSO])));
}

void Core12::BuildFrameResources()
{
	if (m_allRitems.empty())
		return;

	m_frameResources.clear();
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		m_frameResources.push_back(std::make_unique<FrameResourceFx>(m_d3dDevice.Get(),
			2, (UINT)m_allRitems.size(), (UINT)m_materials.size()));
	}
}

XMFLOAT4 ToDxColor(const DxColor::ColorArgb& color)
{
	XMFLOAT4 dxColor;
	dxColor.x = static_cast<float>(color.R) / 255;
	dxColor.y = static_cast<float>(color.G) / 255;
	dxColor.z = static_cast<float>(color.B) / 255;
	dxColor.w = static_cast<float>(color.A) / 255;

	return dxColor;
}

void Core12::BuildMaterials()
{
	auto defaultMat = std::make_unique<Material>();
	defaultMat->Name = MainMat;
	defaultMat->MatCBIndex = 0;
	defaultMat->DiffuseSrvHeapIndex = 0;
	defaultMat->DiffuseAlbedo = m_diffuseAlbedo;
	defaultMat->FresnelR0 = m_fresnelR0;
	defaultMat->Roughness = m_roughness;

	auto backgroundMat = std::make_unique<Material>();
	backgroundMat->Name = BackgroundMat;
	backgroundMat->MatCBIndex = 1;
	backgroundMat->DiffuseSrvHeapIndex = 1;
	backgroundMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	backgroundMat->FresnelR0 = XMFLOAT3(0.07f, 0.07f, 0.07f);
	backgroundMat->Roughness = 0.3f;

	auto shadowMat = std::make_unique<Material>();
	shadowMat->Name = ShadowMat;
	shadowMat->MatCBIndex = 2;
	shadowMat->DiffuseSrvHeapIndex = 1; // same as the background model
	// default XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f);
	shadowMat->DiffuseAlbedo = ToDxColor(m_background.ShadowColor);
	shadowMat->FresnelR0 = XMFLOAT3(0.001f, 0.001f, 0.001f);
	shadowMat->Roughness = 0.0f;

	m_materials[MainMat] = std::move(defaultMat);
	m_materials[BackgroundMat] = std::move(backgroundMat);
	m_materials[ShadowMat] = std::move(shadowMat);
}

void Core12::ClearRenderItems()
{
	m_allRitems.clear();
	m_ritemLayer[(int)RenderLayer::Opaque].clear();
	m_ritemLayer[(int)RenderLayer::Shadow].clear();
}

void Core12::BuildRenderItems()
{
	m_allRitems.clear();
	m_ritemLayer[(int)RenderLayer::Opaque].clear();
	m_ritemLayer[(int)RenderLayer::Shadow].clear();

	if (!m_geometries[MainGeo])
		return;

	auto mainModelRitem = std::make_unique<RenderItem>();
	mainModelRitem->World = MathHelper::Identity4x4();
	mainModelRitem->TexTransform = MathHelper::Identity4x4();
	mainModelRitem->ObjCBIndex = 0;
	mainModelRitem->Mat = m_materials[MainMat].get();						
	mainModelRitem->Geo = m_geometries[MainGeo].get();
	mainModelRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	mainModelRitem->IndexCount = mainModelRitem->Geo->DrawArgs[MainGeo].IndexCount;
	mainModelRitem->StartIndexLocation = mainModelRitem->Geo->DrawArgs[MainGeo].StartIndexLocation;
	mainModelRitem->BaseVertexLocation = mainModelRitem->Geo->DrawArgs[MainGeo].BaseVertexLocation;
	m_mainRitem = mainModelRitem.get();
	m_ritemLayer[(int)RenderLayer::Opaque].push_back(mainModelRitem.get());

	m_allRitems.push_back(std::move(mainModelRitem));

	if (! ShouldShowBackground())
		return;

	auto backgroundRitem = std::make_unique<RenderItem>();
	backgroundRitem->World = MathHelper::Identity4x4();
	backgroundRitem->TexTransform = MathHelper::Identity4x4();
	backgroundRitem->ObjCBIndex = 1;
	backgroundRitem->Mat = m_materials[BackgroundMat].get();
	backgroundRitem->Geo = m_geometries[BackgroundGeo].get();
	backgroundRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	backgroundRitem->IndexCount = backgroundRitem->Geo->DrawArgs[BackgroundGeo].IndexCount;
	backgroundRitem->StartIndexLocation = backgroundRitem->Geo->DrawArgs[BackgroundGeo].StartIndexLocation;
	backgroundRitem->BaseVertexLocation = backgroundRitem->Geo->DrawArgs[BackgroundGeo].BaseVertexLocation;
	m_backgroundRitem = backgroundRitem.get();
	m_ritemLayer[(int)RenderLayer::Opaque].push_back(backgroundRitem.get());

	m_allRitems.push_back(std::move(backgroundRitem));

	if (m_background.ShowShadow == false)
	{
		m_shadowRitem = nullptr;
		return;
	}		

	// Shadowed background will have different world matrix, so it needs to be its own render item.
	auto shadowedRitem = std::make_unique<RenderItem>();
	*shadowedRitem = *m_mainRitem;
	shadowedRitem->ObjCBIndex = 2;
	shadowedRitem->Mat = m_materials[ShadowMat].get();
	m_shadowRitem = shadowedRitem.get();
	m_ritemLayer[(int)RenderLayer::Shadow].push_back(shadowedRitem.get());

	m_allRitems.push_back(std::move(shadowedRitem));
}

void Core12::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = m_currFrameResource->ObjectCB->Resource();
	auto matCB = m_currFrameResource->MaterialCB->Resource();

	// For each render item...
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		auto vertexBufferView = ri->Geo->VertexBufferView();
		cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
		auto indexBufferView = ri->Geo->IndexBufferView();
		cmdList->IASetIndexBuffer(&indexBufferView);
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, m_cbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Core12::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

static const GUID GetDxImageFormat(const CString& ext)
{
	if (ext.CompareNoCase(_T("png")) == 0)
		return GUID_ContainerFormatPng;

	if (ext.CompareNoCase(_T("jpg")) == 0)
		return GUID_ContainerFormatJpeg;

	return GUID_ContainerFormatBmp;
}

bool Core12::DrawImage(const CString& filename, const CString& ext)
{
	if (!IsReadyToRender())
		return false;

	Draw();

	Core12Base::DrawImage(filename, GetDxImageFormat(ext));

	return true;
}

void Core12::SetBackground(const BackgroundImageParams& bkgndParams,
	std::vector<Vertex>&& vertices, std::vector<std::int32_t>&& indices)
{
	m_background = bkgndParams;
	m_vertices2 = vertices;
	m_indices2 = indices;
}

// All required data present and flag is true
bool Core12::ShouldShowBackground() const
{
	return (m_background.ShowBackgroundModel && ! m_background.ImageFilename.empty() &&
		! m_indices2.empty() && ! m_vertices2.empty());
}

void Core12::RebuildModelsAndTexture()
{
	// need to reset command queue
	ThrowIfFailed(m_commandList->Reset(m_directCmdListAlloc.Get(), nullptr));

	LoadTextures();
	BuildDescriptorHeaps();
	BuildMainModelGeometry();
	BuildBackgroundGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();

	// Execute the initialization commands.
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();
}

std::optional<DXF::Vertex<float>> Core12::Map2Dto3D(int x, int y)
{
	if (!m_d3dDevice)
		return std::nullopt;

	if (m_indices.empty() || m_vertices.empty())
		return std::nullopt;

	XMVECTOR pointA = XMVectorSet(static_cast<float>(x), static_cast<float>(y), 0.0f, 0.0f);
	XMVECTOR pointB = XMVectorSet(static_cast<float>(x), static_cast<float>(y), -1.0f, 0.0f);

	XMVECTOR pointATransform = XMVector3Unproject(pointA, m_screenViewport.TopLeftX, m_screenViewport.TopLeftY,
		m_screenViewport.Width, m_screenViewport.Height, m_screenViewport.MinDepth, m_screenViewport.MaxDepth,
		m_projMatrix, m_viewMatrix, m_mainWorldMatrix);

	XMVECTOR pointBTransform = XMVector3Unproject(pointB, m_screenViewport.TopLeftX, m_screenViewport.TopLeftY,
		m_screenViewport.Width, m_screenViewport.Height, m_screenViewport.MinDepth, m_screenViewport.MaxDepth,
		m_projMatrix, m_viewMatrix, m_mainWorldMatrix);

	XMVECTOR projectedVector = pointATransform - pointBTransform;

	double closestDistance = DBL_MAX;
	if (!DxHelpers::GetIntersection(m_vertices, m_indices, pointATransform, projectedVector, closestDistance))
		return std::nullopt;

	return DxHelpers::Create3DTuple(closestDistance, pointATransform, projectedVector);
}

void Core12::SetSelectedRect(const CRect& clientRect, CPoint topLeft, CPoint bottomRight, bool draw)
{
	m_drawRect = draw;

	if (clientRect.Width() == 0 || clientRect.Height() == 0)
		m_drawRect = false;

	if (! m_drawRect)
		return;

	m_relativeSelectedRect = MyDxUtil::MakeRelativeRect(clientRect, topLeft, bottomRight);
}

} // namespace DxSupport