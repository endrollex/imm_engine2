////////////////
// imm_base.h
////////////////
////////////////
#ifndef IMM_BASE_H
#define IMM_BASE_H
#include <windowsx.h>
#include <windows.h>
#include <string>
#include <imm.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using Microsoft::WRL::ComPtr;
#include "d3dx12.h"
#include "imm_base_util.h"
namespace imm
{
////////////////
// base_win
////////////////
////////////////
template <class DERIVED_TYPE>
class base_win
{
public:
	static LRESULT CALLBACK window_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param);
	base_win();
	virtual ~base_win();
	BOOL init_win(HINSTANCE h_instance);
	bool init_d3d12();
	bool create_d2d_device();
	bool create_d3d_device();
	void get_hardware_adapter(IDXGIFactory1* p_factory, IDXGIAdapter1** pp_adapter, bool is_high_perform = true);
	void log_adapters();
	void log_outputs(IDXGIAdapter1* adapter);
	void create_command_objects();
	void create_swap_chain();
	void create_rtv_and_dsv_desc_heaps();
	void on_resize_base();
	void toggle_fullscreen();
	void wait_for_gpu();
	D3D12_CPU_DESCRIPTOR_HANDLE depth_stencil_view() const;
	ID3D12Resource* current_back_buffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE current_back_buffer_view() const;
	void on_render_blank();
	HWND get_hwnd() const {return m_MainHwnd;}
	float aspect_ratio() const {return static_cast<float>(m_ClientWidth)/static_cast<float>(m_ClientHeight);}
	int message_loop();
	virtual void on_resize_drived() = 0;
	virtual void update_scene(float dt) = 0;
	virtual void draw_scene() = 0;
	virtual LRESULT handle_message(UINT u_msg, WPARAM w_param, LPARAM l_param);
	virtual void handle_wm_size(WPARAM w_param, LPARAM l_param);
	// Convenience overrides for handling mouse input.
	virtual void on_mouse_down(WPARAM btn_state, int x, int y) {DUMMY(x); DUMMY(y); DUMMY(btn_state);}
	virtual void on_mouse_up(WPARAM btn_state, int x, int y) {DUMMY(x); DUMMY(y); DUMMY(btn_state);}
	virtual void on_mouse_move(WPARAM btn_state, int x, int y) {DUMMY(x); DUMMY(y); DUMMY(btn_state);}
	virtual void on_mouse_wheel(WPARAM btn_state, int x, int y) {DUMMY(x); DUMMY(y); DUMMY(btn_state);}
	virtual void on_input_char(WPARAM w_param, LPARAM l_param) {DUMMY(w_param); DUMMY(l_param);}
	virtual void on_input_keydown(WPARAM w_param, LPARAM l_param) {DUMMY(w_param); DUMMY(l_param);}
	virtual void on_input_keyup(WPARAM w_param, LPARAM l_param) {DUMMY(w_param); DUMMY(l_param);}
	virtual void game_suspend(const bool &is_stop);
	virtual PCWSTR class_name() const {return L"Immature Engine Class";}
	void calc_frmae_stats();
	//
	HWND m_MainHwnd = nullptr;
	bool m_Paused = false;
	bool m_Minimized = false;
	bool m_Maximized = false;
	bool m_FullScreen = false;
	bool m_Resizing = false;
	bool m_IsLockFrameRate = false;
	int m_ClientWidth = 1067;
	int m_ClientHeight = 599;
	LONG m_ClientMinX = 400;
	LONG m_ClientMinY = 300;
	float m_AspectRatio = 0.0f;
	double m_FrameDeltaLock = 1.0/60.0;
	FLOAT m_DpiX = 96.0f;
	FLOAT m_DpiY = 96.0f;
	std::wstring m_WindowName = L"D3D12 Demo";
	RECT m_WindowRect;
	LONG m_WindowStyle;
	ComPtr<ID3D12Device> m_Device;
	ComPtr<ID3D12Fence> m_Fence;
	ComPtr<IDXGIFactory4> m_DxgiFact;
	ComPtr<ID3D12CommandQueue> m_CommandQueue;
	ComPtr<ID3D12CommandAllocator> m_DirectCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	ComPtr<IDXGISwapChain3> m_SwapChain;
	ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
	static const int m_SwapChainBufferCount = 2;
	int m_CurrBackBuffer = 0;
	ComPtr<ID3D12Resource> m_SwapChainBuffer[m_SwapChainBufferCount];
	ComPtr<ID3D12Resource> m_DepthStencilBuffer;
	bool m_4xMsaaState = false;
	UINT m_4xMsaaQuality = 0;
	UINT m_RtvDescriptorSize = 0;
	UINT m_DsvDescriptorSize = 0;
	UINT m_CbvSrvUavDescriptorSize = 0;
	UINT m_FrameIndex = 0;
	UINT64 m_CurrentFence = 0;
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	D3D12_VIEWPORT m_ScreenViewport; 
	D3D12_RECT m_ScissorRect;
	game_timer m_Timer;
private:
	base_win(const base_win &rhs) = delete;
	base_win &operator=(const base_win &rhs) = delete;
};
////////////////
//
////////////////
////////////////
template <class DERIVED_TYPE>
base_win<DERIVED_TYPE>::base_win()
{
	//IMM_PATH_init();
	ImmDisableIME(0xffffffff);
}
//
template <class DERIVED_TYPE>
base_win<DERIVED_TYPE>::~base_win()
{
	if(m_Device != nullptr) wait_for_gpu();
	OutputDebugString(L"IMM Quit");
}
//
template <class DERIVED_TYPE>
LRESULT CALLBACK base_win<DERIVED_TYPE>::window_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {
	DERIVED_TYPE *p_this = NULL;
	if (u_msg == WM_NCCREATE) {
		CREATESTRUCT *p_create = (CREATESTRUCT*)l_param;
		p_this = (DERIVED_TYPE*)p_create->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)p_this);
		p_this->m_MainHwnd = hwnd;
	}
	else p_this = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (p_this) return p_this->handle_message(u_msg, w_param, l_param);
	else return DefWindowProc(hwnd, u_msg, w_param, l_param);
}
//
template <class DERIVED_TYPE>
BOOL base_win<DERIVED_TYPE>::init_win(HINSTANCE h_instance)
{
	OutputDebugString(L"IMM init_win");
	// DPI in game always 96.0f, UI corresponding screen's height and width, not only DPI
	assert(SetProcessDPIAware());
	WNDCLASSEX wc = {0};
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = DERIVED_TYPE::window_proc;
	wc.hInstance     = h_instance;
	wc.lpszClassName = class_name();
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	RegisterClassEx(&wc);
	m_WindowRect = {0, 0, m_ClientWidth, m_ClientHeight};
	m_WindowStyle = WS_OVERLAPPEDWINDOW;
	AdjustWindowRect(&m_WindowRect, m_WindowStyle, false);
	int width  = m_WindowRect.right - m_WindowRect.left;
	int height = m_WindowRect.bottom - m_WindowRect.top;
	m_MainHwnd = CreateWindow(
		class_name(),
		m_WindowName.c_str(),
		m_WindowStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		nullptr,
		nullptr,
		h_instance,
		this
	);
	ShowWindow(m_MainHwnd, SW_SHOW);
	return (m_MainHwnd ? true : false);
}
//
template <class DERIVED_TYPE>
bool base_win<DERIVED_TYPE>::init_d3d12()
{
	if (!create_d3d_device()) return false;
	create_command_objects();
	create_swap_chain();
	create_rtv_and_dsv_desc_heaps();
	if (!create_d2d_device()) return false;
	on_resize_drived();
	return true;
}
//
template <class DERIVED_TYPE>
bool base_win<DERIVED_TYPE>::create_d2d_device()
{
	return true;
}
//
template <class DERIVED_TYPE>
bool base_win<DERIVED_TYPE>::create_d3d_device()
{
	UINT dxgi_factory_flags = 0;
#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
	ComPtr<ID3D12Debug> debug_controller;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)))) {
		debug_controller->EnableDebugLayer();
		// Enable additional debug layers.
		dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
	}}
#endif
	AbortIfFailed(CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&m_DxgiFact)));
	ComPtr<IDXGIAdapter1> hardware_adapter;
	get_hardware_adapter(m_DxgiFact.Get(), &hardware_adapter, true);
	HRESULT hardware_res = D3D12CreateDevice(hardware_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
	if(FAILED(hardware_res)) {ERROR_MESA_PASS("Failed to create D3D12 Device."); return false;}
	//
	AbortIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
	m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms_quality_levels;
	ms_quality_levels.Format = m_BackBufferFormat;
	ms_quality_levels.SampleCount = 4;
	ms_quality_levels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	ms_quality_levels.NumQualityLevels = 0;
	AbortIfFailed(m_Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &ms_quality_levels, sizeof(ms_quality_levels)));
	m_4xMsaaQuality = ms_quality_levels.NumQualityLevels;
	if (!(m_4xMsaaQuality > 0)) {ERROR_MESA_PASS("Check feature unexpected: ms_quality_levels"); return false;}
	ComPtr<IDXGIFactory6> factory6;
	AbortIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory6)));
	BOOL allow_tearing = FALSE;
	AbortIfFailed(factory6->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing)));
	if (!allow_tearing) {ERROR_MESA_PASS("Check feature unexpected: allow_tearing"); return false;}
#ifdef _DEBUG
	log_adapters();
#endif
	return true;
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::get_hardware_adapter(IDXGIFactory1* p_factory, IDXGIAdapter1** pp_adapter, bool is_high_perform)
{
	DXGI_GPU_PREFERENCE gpu_pref = DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE;
	if (!is_high_perform) gpu_pref = DXGI_GPU_PREFERENCE_UNSPECIFIED;
	*pp_adapter = nullptr;
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(p_factory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
		for (UINT adapterIndex = 0;
			SUCCEEDED(factory6->EnumAdapterByGpuPreference(
				adapterIndex,
				gpu_pref,
				IID_PPV_ARGS(&adapter)));
			++adapterIndex)
		{
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) break;
		}
	}
	if(adapter.Get() == nullptr) {
		for (UINT adapterIndex = 0; SUCCEEDED(p_factory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex) {
			DXGI_ADAPTER_DESC1 desc;
			adapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) break;
		}
	}
	DXGI_ADAPTER_DESC1 desc;
	adapter->GetDesc1(&desc);
	std::wstring text = L"IMM Adapter in use: ";
	text += desc.Description;
	text += L"\n";
	OutputDebugString(text.c_str());
	*pp_adapter = adapter.Detach();
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::log_adapters()
{
	UINT i = 0;
	IDXGIAdapter1* adapter = nullptr;
	std::vector<IDXGIAdapter1*> adapterList;
	while(m_DxgiFact->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);
		std::wstring text = L"IMM Adapter option: ";
		text += desc.Description;
		text += L"\n";
		OutputDebugString(text.c_str());
		adapterList.push_back(adapter);
		++i;
	}
	for(size_t cnt = 0; cnt < adapterList.size(); ++cnt)
	{
		log_outputs(adapterList[cnt]);
		RELEASE_COM(adapterList[cnt]);
	}
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::log_outputs(IDXGIAdapter1* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while(adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);
		std::wstring text = L"IMM Output option: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());
		RELEASE_COM(output);
		++i;
	}
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::create_command_objects()
{
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	AbortIfFailed(m_Device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_CommandQueue)));
	AbortIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_DirectCmdListAlloc.GetAddressOf())));
	AbortIfFailed(m_Device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_DirectCmdListAlloc.Get(),
		nullptr,
		IID_PPV_ARGS(m_CommandList.GetAddressOf())));
	// Start off in a closed state.  This is because the first time we refer 
	// to the command list we will Reset it, and it needs to be closed before
	// calling Reset.
	AbortIfFailed(m_CommandList->Close());
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::create_swap_chain()
{
	// Release the previous swapchain we will be recreating.
	m_SwapChain.Reset();
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swap_desc = {};
	swap_desc.Width = m_ClientWidth;
	swap_desc.Height = m_ClientHeight;
	swap_desc.Format = m_BackBufferFormat;
	swap_desc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	swap_desc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	swap_desc.SampleDesc.Count = 1;
	swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_desc.BufferCount = m_SwapChainBufferCount;
	swap_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	ComPtr<IDXGISwapChain1> swap_chain;
	AbortIfFailed(m_DxgiFact->CreateSwapChainForHwnd(
		m_CommandQueue.Get(),
		m_MainHwnd,
		&swap_desc,
		nullptr,
		nullptr,
		swap_chain.GetAddressOf()));
	//
	AbortIfFailed(m_DxgiFact->MakeWindowAssociation(m_MainHwnd, DXGI_MWA_NO_ALT_ENTER));
	AbortIfFailed(swap_chain.As(&m_SwapChain));
	m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
	ComPtr<IDXGIOutput> p_output;
	HRESULT hr_swap;
	hr_swap = m_SwapChain->GetContainingOutput(&p_output);
	std::wstring str = std::to_wstring(hr_swap);
	OutputDebugString(str.c_str());
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::create_rtv_and_dsv_desc_heaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_d = {};
	rtv_heap_d.NumDescriptors = m_SwapChainBufferCount;
	rtv_heap_d.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtv_heap_d.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtv_heap_d.NodeMask = 0;
	AbortIfFailed(m_Device->CreateDescriptorHeap(&rtv_heap_d, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));
	D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_d = {};
	dsv_heap_d.NumDescriptors = 1;
	dsv_heap_d.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsv_heap_d.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsv_heap_d.NodeMask = 0;
	AbortIfFailed(m_Device->CreateDescriptorHeap(&dsv_heap_d, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::on_resize_base()
{
	OutputDebugString(L"IMM on_resize_base");
	if (!m_Device) return;
	assert(m_Device);
	assert(m_SwapChain);
	assert(m_DirectCmdListAlloc);
	// Flush before changing any resources.
	wait_for_gpu();
	AbortIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));
	// Release the previous resources we will be recreating.
	for (int i = 0; i < m_SwapChainBufferCount; ++i) m_SwapChainBuffer[i].Reset();
	m_DepthStencilBuffer.Reset();
	// Resize the swap chain.
	DXGI_SWAP_CHAIN_DESC desc = {};
	m_SwapChain->GetDesc(&desc);
	AbortIfFailed(m_SwapChain->ResizeBuffers(
		m_SwapChainBufferCount,
		m_ClientWidth,
		m_ClientHeight,
		m_BackBufferFormat,
		desc.Flags));
	m_CurrBackBuffer = 0;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < m_SwapChainBufferCount; i++)
	{
		AbortIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_SwapChainBuffer[i])));
		m_Device->CreateRenderTargetView(m_SwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_RtvDescriptorSize);
	}
	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depth_stencil_d;
	depth_stencil_d.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depth_stencil_d.Alignment = 0;
	depth_stencil_d.Width = m_ClientWidth;
	depth_stencil_d.Height = m_ClientHeight;
	depth_stencil_d.DepthOrArraySize = 1;
	depth_stencil_d.MipLevels = 1;
	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depth_stencil_d.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depth_stencil_d.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	depth_stencil_d.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	depth_stencil_d.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depth_stencil_d.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	D3D12_CLEAR_VALUE opt_clear;
	opt_clear.Format = m_DepthStencilFormat;
	opt_clear.DepthStencil.Depth = 1.0f;
	opt_clear.DepthStencil.Stencil = 0;
	CD3DX12_HEAP_PROPERTIES heap_default(D3D12_HEAP_TYPE_DEFAULT);
	AbortIfFailed(m_Device->CreateCommittedResource(
		&heap_default,
		D3D12_HEAP_FLAG_NONE,
		&depth_stencil_d,
		D3D12_RESOURCE_STATE_COMMON,
		&opt_clear,
		IID_PPV_ARGS(m_DepthStencilBuffer.GetAddressOf())));
	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;
	dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
	dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Format = m_DepthStencilFormat;
	dsv_desc.Texture2D.MipSlice = 0;
	m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), &dsv_desc, depth_stencil_view());
	const D3D12_RESOURCE_BARRIER reso_barrier(
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_DepthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_DEPTH_WRITE));
	// Transition the resource from its initial state to be used as a depth buffer.
	m_CommandList->ResourceBarrier(1, &reso_barrier);
	//
	// Execute the resize commands.
	AbortIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmds_lists[] = {m_CommandList.Get()};
	m_CommandQueue->ExecuteCommandLists(_countof(cmds_lists), cmds_lists);
	// Wait until resize is complete.
	wait_for_gpu();
	// Update the viewport transform to cover the client area.
	m_ScreenViewport.TopLeftX = 0;
	m_ScreenViewport.TopLeftY = 0;
	m_ScreenViewport.Width    = static_cast<float>(m_ClientWidth);
	m_ScreenViewport.Height   = static_cast<float>(m_ClientHeight);
	m_ScreenViewport.MinDepth = 0.0f;
	m_ScreenViewport.MaxDepth = 1.0f;
	m_ScissorRect = { 0, 0, m_ClientWidth, m_ClientHeight };
	//
	std::string text = "IMM "+std::to_string(m_ClientWidth)+" "+std::to_string(m_ClientHeight);
	OutputDebugString(uni::cha_s2ws(text).c_str());
	//
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::toggle_fullscreen()
{
	if (!m_SwapChain) return;
	if (m_FullScreen) {
		OutputDebugString(L"IMM toggle_fullscreen m_FullScreen");
		// Restore the window's attributes and size.
		SetWindowLong(m_MainHwnd, GWL_STYLE, m_WindowStyle);
		SetWindowPos(
			m_MainHwnd,
			HWND_NOTOPMOST,
			m_WindowRect.left,
			m_WindowRect.top,
			m_WindowRect.right - m_WindowRect.left,
			m_WindowRect.bottom - m_WindowRect.top,
			SWP_FRAMECHANGED | SWP_NOACTIVATE);
		ShowWindow(m_MainHwnd, SW_NORMAL);
		m_FullScreen = false;
		return;
	}
	GetWindowRect(m_MainHwnd, &m_WindowRect);
	SetWindowLong(m_MainHwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));
	RECT full_rect;
	DEVMODE dev_mode = {};
	dev_mode.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dev_mode);
	full_rect = {
		dev_mode.dmPosition.x,
		dev_mode.dmPosition.y,
		dev_mode.dmPosition.x + static_cast<LONG>(dev_mode.dmPelsWidth),
		dev_mode.dmPosition.y + static_cast<LONG>(dev_mode.dmPelsHeight)
	};
	SetWindowPos(
		m_MainHwnd,
		HWND_TOP,
		full_rect.left,
		full_rect.top,
		full_rect.right,
		full_rect.bottom,
		SWP_FRAMECHANGED | SWP_NOACTIVATE);
	ShowWindow(m_MainHwnd, SW_MAXIMIZE);
	m_FullScreen = true;
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::wait_for_gpu()
{
	// Advance the fence value to mark commands up to this fence point.
	m_CurrentFence++;
	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	AbortIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));
	// Wait until the GPU has completed commands up to this fence point.
	if(m_Fence->GetCompletedValue() < m_CurrentFence)
	{
		HANDLE event_handle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		// Fire event when GPU hits current fence.  
		AbortIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, event_handle));
		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(event_handle, INFINITE);
		CloseHandle(event_handle);
	}
}
//
template <class DERIVED_TYPE>
D3D12_CPU_DESCRIPTOR_HANDLE base_win<DERIVED_TYPE>::depth_stencil_view() const
{
	return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
}
//
template <class DERIVED_TYPE>
ID3D12Resource* base_win<DERIVED_TYPE>::current_back_buffer() const
{
	return m_SwapChainBuffer[m_CurrBackBuffer].Get();
}
//
template <class DERIVED_TYPE>
D3D12_CPU_DESCRIPTOR_HANDLE base_win<DERIVED_TYPE>::current_back_buffer_view() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrBackBuffer,
		m_RtvDescriptorSize);
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::on_render_blank()
{
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	AbortIfFailed(m_DirectCmdListAlloc->Reset());
	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	AbortIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));
	const D3D12_RESOURCE_BARRIER reso_barrier(
		CD3DX12_RESOURCE_BARRIER::Transition(
			current_back_buffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET));
	// Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &reso_barrier);
	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);
	// Clear the back buffer and depth buffer.
	const float clear_color[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	D3D12_CPU_DESCRIPTOR_HANDLE cbuff_v = current_back_buffer_view();
	D3D12_CPU_DESCRIPTOR_HANDLE depthst_v = depth_stencil_view();
	m_CommandList->ClearRenderTargetView(cbuff_v, clear_color, 0, nullptr);
	m_CommandList->ClearDepthStencilView(depthst_v, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	// Specify the buffers we are going to render to.
	m_CommandList->OMSetRenderTargets(1,
		&cbuff_v,
		true,
		&depthst_v);
	const D3D12_RESOURCE_BARRIER reso_barrier2(
		CD3DX12_RESOURCE_BARRIER::Transition(
			current_back_buffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));
	// Indicate a state transition on the resource usage.
	m_CommandList->ResourceBarrier(1, &reso_barrier2);
	// Done recording commands.
	AbortIfFailed(m_CommandList->Close());
	// Add the command list to the queue for execution.
	ID3D12CommandList* cmds_lists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmds_lists), cmds_lists);
	// swap the back and front buffers
	AbortIfFailed(m_SwapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING));
	m_CurrBackBuffer = m_SwapChain->GetCurrentBackBufferIndex();
	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	wait_for_gpu();
}
//
template <class DERIVED_TYPE>
int base_win<DERIVED_TYPE>::message_loop()
{
	OutputDebugString(L"IMM message_loop");
	MSG msg = {};
	m_Timer.reset();
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			m_Timer.tick();
			if (!m_Paused) {
				calc_frmae_stats();
				update_scene(m_Timer.delta_time());
				draw_scene();
				if (m_IsLockFrameRate) {
					DWORD time_wait = static_cast<DWORD>((m_FrameDeltaLock-m_Timer.delta_time_test())*1000.0);
					if (time_wait > 0 && time_wait < 100) Sleep(time_wait);
				}
			}
			else Sleep(100);
		}
	}
	return (int)msg.wParam;
}
//
template <class DERIVED_TYPE>
LRESULT base_win<DERIVED_TYPE>::handle_message(UINT u_msg, WPARAM w_param, LPARAM l_param)
{
	switch (u_msg) {
	case WM_CREATE:
		{
		OutputDebugString(L"IMM WM_CREATE");
		// Save the DXSample* passed in to CreateWindow.
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(l_param);
		SetWindowLongPtr(m_MainHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;
	// WM_ACTIVATE is sent when the window is activated or deactivated.
	// We pause the game when the window is deactivated and unpause it
	// when it becomes active.
	case WM_ACTIVATE:
		if (LOWORD(w_param) == WA_INACTIVE) {
			game_suspend(true);
		}
		else {
			game_suspend(false);
		}
		return 0;
	// WM_SIZE is sent when the user resizes the window.
	case WM_SIZE:
		handle_wm_size(w_param, l_param);
		return 0;
	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_Resizing = true;
		game_suspend(true);
		return 0;
	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		m_Resizing = false;
		game_suspend(false);
		on_resize_drived();
		return 0;
	// The WM_MENUCHAR message is sent when a menu is active and the user presses
	// a key that does not correspond to any mnemonic or accelerator key.
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);
	// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)l_param)->ptMinTrackSize.x = m_ClientMinX;
		((MINMAXINFO*)l_param)->ptMinTrackSize.y = m_ClientMinY;
		return 0;
	//
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		on_mouse_down(w_param, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		on_mouse_up(w_param, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));
		return 0;
	case WM_MOUSEMOVE:
		on_mouse_move(w_param, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));
		return 0;
	case WM_MOUSEWHEEL:
		on_mouse_wheel(w_param, GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param));
		return 0;
	//
	case WM_CHAR:
		on_input_char(w_param, l_param);
		return 0;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		OutputDebugString(L"IMM WM_SYSKEYDOWN");
		if ((w_param == VK_RETURN) && (l_param & (1 << 29))) {
			toggle_fullscreen();
		}
		if ((w_param == VK_F4) && (l_param & (1 << 29))) {
			PostQuitMessage(0);
			return 0;
	case WM_KEYUP:
		on_input_keyup(w_param, l_param);
		return 0;
	case WM_CLOSE:
		OutputDebugString(L"IMM WM_CLOSE");
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		}
		return 0;
	}
	return DefWindowProc(m_MainHwnd, u_msg, w_param, l_param);
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::handle_wm_size(WPARAM w_param, LPARAM l_param)
{
	OutputDebugString(L"IMM handle_wm_size");
	// Save the new client area dimensions.
	m_ClientWidth  = LOWORD(l_param);
	m_ClientHeight = HIWORD(l_param);
	if (!m_Device) return;
	if (w_param == SIZE_MINIMIZED) {
		game_suspend(true);
		m_Minimized = true;
		m_Maximized = false;
		return;
	}
	if (w_param == SIZE_MAXIMIZED) {
		game_suspend(false);
		m_Minimized = false;
		m_Maximized = true;
		on_resize_drived();
		return;
	}
	if (w_param == SIZE_RESTORED) {
		// Restoring from minimized state?
		if (m_Minimized) {
			game_suspend(false);
			m_Minimized = false;
			on_resize_drived();
			return;
		}
		// Restoring from maximized state?
		if (m_Maximized) {
			game_suspend(false);
			m_Maximized = false;
			on_resize_drived();
			return;
		}
		if (m_Resizing) {
			// If user is dragging the resize bars, we do not resize
			// the buffers here because as the user continuously
			// drags the resize bars, a stream of WM_SIZE messages are
			// sent to the window, and it would be pointless (and slow)
			// to resize for each WM_SIZE message received from dragging
			// the resize bars.  So instead, we reset after the user is
			// done resizing the window and releases the resize bars, which
			// sends a WM_EXITSIZEMOVE message.
			return;
		}
		else {
			// API call such as SetWindowPos or m_SwapChain->SetFullscreenState.
			on_resize_drived();
			return;
		}
	}
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::game_suspend(const bool &is_stop)
{
	// derived game_suspend() must call this base version
	m_Paused = is_stop;
	if (is_stop) m_Timer.stop();
	else m_Timer.start();
}
//
template <class DERIVED_TYPE>
void base_win<DERIVED_TYPE>::calc_frmae_stats()
{
	// Code computes the average frames per second, and also the
	// average time it takes to render one frame.  These stats
	// are appended to the window caption bar.
	static int frame_cnt = 0;
	static float time_elapsed = 0.0f;
	frame_cnt++;
	// Compute averages over one second period.
	if ((m_Timer.total_time()-time_elapsed) >= 1.0f) {
		float fps = (float)frame_cnt; // fps = frame_cnt / 1
		float mspf = 1000.0f / fps;
		std::wstring fps_str = std::to_wstring(fps);
		std::wstring mspf_tr = std::to_wstring(mspf);
		std::wstring window_text = m_WindowName + L" fps: " + fps_str + L" mspf: " + mspf_tr;
		SetWindowText(m_MainHwnd, window_text.c_str());
		// Reset for next average.
		frame_cnt = 0;
		time_elapsed += 1.0f;
	}
}
}
#endif