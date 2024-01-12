////////////////
// init_d3d
////////////////
////////////////
#ifndef UNICODE
#define UNICODE
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "imm_base.h"
#pragma comment(lib, "user32")
#pragma comment(lib, "imm32")
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "D3DCompiler")
#pragma comment(lib, "dxguid")
using namespace imm;
#include <DirectXColors.h>
#include <array>
//
struct vertex
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};
//
struct object_constants
{
	XMFLOAT4X4 world_view_proj = math_helper::identity4x4();
};
//
class imm_app: public base_win<imm_app>
{
public:
	void on_resize_drived();
	void update_scene(float dt);
	void draw_scene();
	void draw_scene_imm();
	imm_app();
	~imm_app();
	bool init_imm();
	void build_desc_heaps();
	void build_constant_buffers();
	void build_root_singnature();
	void build_shader_and_input_layout();
	void build_box_geometry();
	void build_pso();
	virtual void on_mouse_down(WPARAM btn_state, int x, int y) override;
	virtual void on_mouse_up(WPARAM btn_state, int x, int y) override;
	virtual void on_mouse_move(WPARAM btn_state, int x, int y) override;
	ComPtr<ID3D12DescriptorHeap> m_CbvHeap = nullptr;
	std::unique_ptr<upload_buffer<object_constants>> m_ObjectCB = nullptr;
	ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;
	ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	ComPtr<ID3DBlob> m_psByteCode = nullptr;
	std::unique_ptr<mesh_geometry> m_BoxGeo = nullptr;
	ComPtr<ID3D12PipelineState> m_PSO = nullptr;
	XMFLOAT4X4 m_Proj = math_helper::identity4x4();
	XMFLOAT4X4 m_View = math_helper::identity4x4();
	XMFLOAT4X4 m_World = math_helper::identity4x4();
	float m_Theta = 1.5f*XM_PI;
	float m_Phi = XM_PIDIV4;
	float m_Radius = 5.0f;
	POINT m_LastMousePos = {0, 0};
};
////////////////
//
////////////////
////////////////
int WINAPI wWinMain(HINSTANCE h_instance, HINSTANCE, PWSTR p_cmd_line, int m_cmd_show)
{
	DUMMY(p_cmd_line);
	DUMMY(m_cmd_show);
	imm_app win;
	if (!win.init_win(h_instance)) return 0;
	if (!win.init_d3d12()) return 0;
	if (!win.init_imm()) return 0;
	return win.message_loop();
}
//
//
void imm_app::on_resize_drived()
{
	on_resize_base();
	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*math_helper::pi, aspect_ratio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, P);
}
//
void imm_app::update_scene(float dt)
{
	DUMMY(dt);
	// Convert Spherical to Cartesian coordinates.
	float x = m_Radius*sinf(m_Phi)*cosf(m_Theta);
	float z = m_Radius*sinf(m_Phi)*sinf(m_Theta);
	float y = m_Radius*cosf(m_Phi);
	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_View, view);
	XMMATRIX world = XMLoadFloat4x4(&m_World);
	XMMATRIX proj = XMLoadFloat4x4(&m_Proj);
	XMMATRIX world_view_proj = world*view*proj;
	// Update the constant buffer with the latest worldViewProj matrix.
	object_constants obj_constants;
	XMStoreFloat4x4(&obj_constants.world_view_proj, XMMatrixTranspose(world_view_proj));
	m_ObjectCB->copy_data(0, obj_constants);
}
//
void imm_app::draw_scene()
{
	draw_scene_imm();
	//on_render_blank();
}
//
void imm_app::draw_scene_imm()
{
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	AbortIfFailed(m_DirectCmdListAlloc->Reset());
	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	AbortIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), m_PSO.Get()));
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);
	// Indicate a state transition on the resource usage.
	const D3D12_RESOURCE_BARRIER reso_barrier(
		CD3DX12_RESOURCE_BARRIER::Transition(
			current_back_buffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_CommandList->ResourceBarrier(1, &reso_barrier);
	// Clear the back buffer and depth buffer.
	D3D12_CPU_DESCRIPTOR_HANDLE cbuff_v = current_back_buffer_view();
	D3D12_CPU_DESCRIPTOR_HANDLE depthst_v = depth_stencil_view();
	m_CommandList->ClearRenderTargetView(cbuff_v, Colors::LightSteelBlue, 0, nullptr);
	m_CommandList->ClearDepthStencilView(depthst_v, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	// Specify the buffers we are going to render to.
	m_CommandList->OMSetRenderTargets(1, &cbuff_v, true, &depthst_v);
	ID3D12DescriptorHeap* descriptor_heaps[] = { m_CbvHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(descriptor_heaps), descriptor_heaps);
	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
	D3D12_VERTEX_BUFFER_VIEW box_vertex_buff_v = m_BoxGeo->vertex_buffer_view();
	D3D12_INDEX_BUFFER_VIEW box_index_buff_v = m_BoxGeo->index_buffer_view();
	m_CommandList->IASetVertexBuffers(0, 1, &box_vertex_buff_v);
	m_CommandList->IASetIndexBuffer(&box_index_buff_v);
	m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_CommandList->SetGraphicsRootDescriptorTable(0, m_CbvHeap->GetGPUDescriptorHandleForHeapStart());
	m_CommandList->DrawIndexedInstanced(m_BoxGeo->draw_args["box"].index_count, 1, 0, 0, 0);
	// Indicate a state transition on the resource usage.
	const D3D12_RESOURCE_BARRIER reso_barrier2(
		CD3DX12_RESOURCE_BARRIER::Transition(
			current_back_buffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));
	m_CommandList->ResourceBarrier(1, &reso_barrier2);
	// Done recording commands.
	AbortIfFailed(m_CommandList->Close());
 	// Add the command list to the queue for execution.
	ID3D12CommandList* cmds_lists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmds_lists), cmds_lists);
	// swap the back and front buffers
	AbortIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % m_SwapChainBufferCount;
	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	wait_for_gpu();
}
//
imm_app::imm_app()
{
	;
}
//
imm_app::~imm_app()
{
	;
}
//
bool imm_app::init_imm()
{
	AbortIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));
	build_desc_heaps();
	build_constant_buffers();
	build_root_singnature();
	build_shader_and_input_layout();
	build_box_geometry();
	build_pso();
	// Execute the initialization commands.
	AbortIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmds_lists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmds_lists), cmds_lists);
	// Wait until initialization is complete.
	wait_for_gpu();
	return true;
}
//
void imm_app::build_desc_heaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbv_heap_desc;
	cbv_heap_desc.NumDescriptors = 1;
	cbv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbv_heap_desc.NodeMask = 0;
	AbortIfFailed(m_Device->CreateDescriptorHeap(&cbv_heap_desc, IID_PPV_ARGS(&m_CbvHeap)));
}
//
void imm_app::build_constant_buffers()
{
	m_ObjectCB = std::make_unique<upload_buffer<object_constants>>(m_Device.Get(), 1, true);
	UINT obj_cb_byte_size = d3d_util::calc_constant_buffer_byte_size(sizeof(object_constants));
	D3D12_GPU_VIRTUAL_ADDRESS cb_address = m_ObjectCB->resource()->GetGPUVirtualAddress();
	// Offset to the ith object constant buffer in the buffer.
	int box_cb_index = 0;
	cb_address += box_cb_index*obj_cb_byte_size;
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
	cbv_desc.BufferLocation = cb_address;
	cbv_desc.SizeInBytes = d3d_util::calc_constant_buffer_byte_size(sizeof(object_constants));
	m_Device->CreateConstantBufferView(
		&cbv_desc,
		m_CbvHeap->GetCPUDescriptorHandleForHeapStart());
}
//
void imm_app::build_root_singnature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  
	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slot_root_parameter[1];
	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbv_table;
	cbv_table.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slot_root_parameter[0].InitAsDescriptorTable(1, &cbv_table);
	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC root_sig_desc(1, slot_root_parameter, 0, nullptr, 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serialized_root_sig = nullptr;
	ComPtr<ID3DBlob> error_blob = nullptr;
	HRESULT hr_serialze = D3D12SerializeRootSignature(&root_sig_desc, D3D_ROOT_SIGNATURE_VERSION_1,
		serialized_root_sig.GetAddressOf(), error_blob.GetAddressOf());
	if(error_blob != nullptr) {
		OutputDebugStringA((char*)error_blob->GetBufferPointer());
	}
	AbortIfFailed(hr_serialze);
	AbortIfFailed(m_Device->CreateRootSignature(
		0,
		serialized_root_sig->GetBufferPointer(),
		serialized_root_sig->GetBufferSize(),
		IID_PPV_ARGS(&m_RootSignature)));
	//
}
//
void imm_app::build_shader_and_input_layout()
{
	m_vsByteCode = d3d_util::compile_shader(L"shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	m_psByteCode = d3d_util::compile_shader(L"shaders\\color.hlsl", nullptr, "PS", "ps_5_0");
	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}
//
void imm_app::build_box_geometry()
{
	std::array<vertex, 8> vertices =
	{
		vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
	};
	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,
		// back face
		4, 6, 5,
		4, 7, 6,
		// left face
		4, 5, 1,
		4, 1, 0,
		// right face
		3, 2, 6,
		3, 6, 7,
		// top face
		1, 5, 6,
		1, 6, 2,
		// bottom face
		4, 0, 3,
		4, 3, 7
	};
	const UINT vb_byte_size = (UINT)vertices.size() * sizeof(vertex);
	const UINT ib_byte_size = (UINT)indices.size() * sizeof(std::uint16_t);
	m_BoxGeo = std::make_unique<mesh_geometry>();
	m_BoxGeo->name = "box_geo";
	AbortIfFailed(D3DCreateBlob(vb_byte_size, &m_BoxGeo->vertex_buffer_cpu));
	CopyMemory(m_BoxGeo->vertex_buffer_cpu->GetBufferPointer(), vertices.data(), vb_byte_size);
	AbortIfFailed(D3DCreateBlob(ib_byte_size, &m_BoxGeo->index_buffer_cpu));
	CopyMemory(m_BoxGeo->index_buffer_cpu->GetBufferPointer(), indices.data(), ib_byte_size);
	m_BoxGeo->vertex_buffer_gpu = d3d_util::create_default_buffer(m_Device.Get(),
		m_CommandList.Get(), vertices.data(), vb_byte_size, m_BoxGeo->vertex_buffer_uploader);
	m_BoxGeo->index_buffer_gpu = d3d_util::create_default_buffer(m_Device.Get(),
		m_CommandList.Get(), indices.data(), ib_byte_size, m_BoxGeo->index_buffer_uploader);
	m_BoxGeo->vertex_byte_stride = sizeof(vertex);
	m_BoxGeo->vertex_buffer_byte_size = vb_byte_size;
	m_BoxGeo->index_format = DXGI_FORMAT_R16_UINT;
	m_BoxGeo->index_buffer_byte_size = ib_byte_size;
	submesh_geometry submesh;
	submesh.index_count = (UINT)indices.size();
	submesh.start_index_location = 0;
	submesh.base_vertex_location = 0;
	m_BoxGeo->draw_args["box"] = submesh;
}
//
void imm_app::build_pso()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc;
	ZeroMemory(&pso_desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	pso_desc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
	pso_desc.pRootSignature = m_RootSignature.Get();
	pso_desc.VS = 
	{ 
		reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()), 
		m_vsByteCode->GetBufferSize() 
	};
	pso_desc.PS = 
	{ 
		reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()), 
		m_psByteCode->GetBufferSize() 
	};
	pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	pso_desc.SampleMask = UINT_MAX;
	pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	pso_desc.NumRenderTargets = 1;
	pso_desc.RTVFormats[0] = m_BackBufferFormat;
	pso_desc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	pso_desc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	pso_desc.DSVFormat = m_DepthStencilFormat;
	AbortIfFailed(m_Device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&m_PSO)));
}
//
void imm_app::on_mouse_down(WPARAM btn_state, int x, int y)
{
	btn_state;
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
	SetCapture(m_MainHwnd);
}
//
void imm_app::on_mouse_up(WPARAM btn_state, int x, int y)
{
	btn_state;
	x;
	y;
	ReleaseCapture();
}
//
void imm_app::on_mouse_move(WPARAM btn_state, int x, int y)
{
	if((btn_state & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - m_LastMousePos.y));
		// Update angles based on input to orbit camera around box.
		m_Theta += dx;
		m_Phi += dy;
		// Restrict the angle m_Phi.
		m_Phi = math_helper::clamp_t(m_Phi, 0.1f, math_helper::pi - 0.1f);
	}
	else if((btn_state & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f*static_cast<float>(x - m_LastMousePos.x);
		float dy = 0.005f*static_cast<float>(y - m_LastMousePos.y);
		// Update the camera radius based on input.
		m_Radius += dx - dy;
		// Restrict the radius.
		m_Radius = math_helper::clamp_t(m_Radius, 3.0f, 15.0f);
	}
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}
