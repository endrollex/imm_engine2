////////////////
// imm_base_util.h
////////////////
////////////////
#ifndef IMM_BASE_UTIL_H
#define IMM_BASE_UTIL_H
#include <cassert>
#include <windows.h>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <map>
#include <codecvt>
#include <string>
#include <sstream>
#include <unordered_map>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <D3DCompiler.h>
using namespace DirectX;
namespace imm
{
////////////////
// DEBUG, IUnknown
////////////////
////////////////
#if defined(DEBUG) | defined(_DEBUG)
	#ifndef HR
	#define HR(x) {HRESULT hr_get = (x); if (FAILED(hr_get))\
		{std::string hrs(std::to_string(hr_get)); MessageBoxA(0, hrs.c_str(), "HRESULT", MB_OK); assert(false); abort();}}
	#endif
#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif
#define RELEASE_COM(x) {if (x) {x->Release(); x = nullptr;}}
#define SAFE_DELETE(x) {delete x; x = nullptr;}
// do nothing
#define DUMMY(x) (x)
// error messagebox
#define ERROR_MESA(x) {MessageBoxA(0, x, "ERROR", MB_OK); assert(false); abort();}
#define ERROR_MESA_PASS(x) {MessageBoxA(0, x, "ERROR", MB_OK);}
////////////////
// universal
////////////////
////////////////
namespace uni
{
static std::map<std::string, std::string> IMM_PATH;
static const bool ALWAYS_TRUE = true;
static const size_t VECTOR_RESERVE = 1000;
// for calclate UI size with factor
static const float UI_RESOLUTION_WIDTH = 1366.0f;
static const float UI_RESOLUTION_HEIGHT = 768.0f;
static const float FLOAT_4_ZERO[] = {0.0f, 0.0f, 0.0f, 0.0f};
// frame rate, Blender's default setting is 24.0f
static const float FRAME_RATE = 24.0f;
static const float FRAME_RATE_1DIV = 1.0f/FRAME_RATE;
static const float FPS60 = 60.0f;
static const float FPS60_1DIV = 1.0f/60.0f;
static const float FPS_MIN_REQ_1DIV = 1.0f/20.0f;
static const float TIME_1_MINITE = 60.0f;
static const float TIME_59_SECONDS = 59.0F;
static const float GAME_HP_BAR = 20.0f;
static const float AI_DT_LOGIC = 0.05f;
static const float AI_DT_PHY_FAST = 0.05f;
static const float AI_DT_PHY_SLOW = 0.1f;
static const float AI_DT_PHY_2SLOW = 0.5f;
static const float ATK_IMPULSE_DMG2 = 5.0f;
static const float PHY_DELTA_TIME_MAX = 1.0f/15.0f;
static const std::wstring SCENE_FIRST = L"_00";
static const std::wstring SCENE_SECOND = L"_01";
static std::string EMPTY_STRING = "";
static bool IS_STANDALONE_M3DTOB3M = false;
//
std::wstring cha_s2ws(const std::string &str)
{
	using convert_type_x = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type_x, wchar_t> converter_x;
	return converter_x.from_bytes(str);
}
//
std::string cha_ws2s(const std::wstring &wstr)
{
	using convert_type_x = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type_x, wchar_t> converter_x;
	return converter_x.to_bytes(wstr);
}
//
template <typename T>
std::string cha_hex(const T &input)
{
	std::stringstream stream;
	stream << std::hex << input;
	std::string result(stream.str());
	return result;
}
//
std::string format_hr(const HRESULT &hr_get, const std::string &s_func)
{
	std::string s_hr(cha_hex(hr_get));
	std::string s_file(__FILE__);
	std::string line_num(std::to_string(__LINE__));
	std::string text = s_hr+"\n"+s_file+"\n"+line_num+"\n"+s_func;
	return text;
}
}
//
#ifndef AbortIfFailed
#define AbortIfFailed(x) \
{ \
	HRESULT hr_get = (x); \
	if(FAILED(hr_get)) {MessageBoxA(NULL, uni::format_hr(hr_get, uni::cha_ws2s(L#x)).c_str(), "HRESULT", MB_OK); abort();} \
}
#endif
////////////////
// game_timer
////////////////
////////////////
class game_timer
{
public:
	game_timer();
	float total_time() const;
	float delta_time() const {return (float)m_DeltaTime;}
	double delta_time_test() const;
	void reset();
	void start();
	void stop();
	void tick();
//private:
	double m_SecPerCount = 0.0;
	double m_DeltaTime = -1.0;
	__int64 m_BaseTime = 0;
	__int64 m_PausedTime = 0;
	__int64 m_StopTime = 0;
	__int64 m_PrevTime = 0;
	__int64 m_CurrTime = 0;
	__int64 m_CurrTest = 0;
	bool m_Stopped = false;
};
//
game_timer::game_timer()
{
	__int64 counts_per_sec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&counts_per_sec);
	m_SecPerCount = 1.0/(double)counts_per_sec;
}
// Returns the total time elapsed since reset() was called, NOT counting any
// time when the clock is stopped.
float game_timer::total_time() const
{
	if (m_Stopped) return (float)(((m_StopTime-m_PausedTime)-m_BaseTime)*m_SecPerCount);
	else return (float)(((m_CurrTime-m_PausedTime)-m_BaseTime)*m_SecPerCount);
}
//
double game_timer::delta_time_test() const
{
	QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrTest);
	return (m_CurrTest-m_PrevTime)*m_SecPerCount;
}
//
void game_timer::reset()
{
	__int64 curr_time;
	QueryPerformanceCounter((LARGE_INTEGER*)&curr_time);
	m_BaseTime = curr_time;
	m_PrevTime = curr_time;
	m_StopTime = 0;
	m_Stopped  = false;
}
//
void game_timer::start()
{
	__int64 start_time;
	QueryPerformanceCounter((LARGE_INTEGER*)&start_time);
	if (m_Stopped) {
		m_PausedTime += (start_time-m_StopTime);
		m_PrevTime = start_time;
		m_StopTime = 0;
		m_Stopped  = false;
	}
}
//
void game_timer::stop()
{
	if (!m_Stopped) {
		__int64 curr_time;
		QueryPerformanceCounter((LARGE_INTEGER*)&curr_time);
		m_StopTime = curr_time;
		m_Stopped  = true;
	}
}
//
void game_timer::tick()
{
	if (m_Stopped) {m_DeltaTime = 0.0; return;}
	QueryPerformanceCounter((LARGE_INTEGER*)&m_CurrTime);
	// Time difference between this frame and the previous.
	m_DeltaTime = (m_CurrTime-m_PrevTime)*m_SecPerCount;
	// Prepare for next frame.
	m_PrevTime = m_CurrTime;
	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the
	// processor goes into a power save mode or we get shuffled to another
	// processor, then m_DeltaTime can be negative.
	if (m_DeltaTime < 0.0) m_DeltaTime = 0.0;
}
////////////////
// 
////////////////
////////////////
struct vertex_pc
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
};
////////////////
// math_helper
////////////////
////////////////
class math_helper
{
public:
	// Returns random float in [0, 1).
	static float rand_f() { return (float)(rand()) / (float)RAND_MAX; }
	// Returns random float in [a, b).
	static float rand_f(float a, float b) { return a + rand_f()*(b-a); }
	static int rand_i(int a, int b) { return a + rand() % ((b - a) + 1); }
	template<typename T> static T min_t(const T& a, const T& b) { return a < b ? a : b; }
	template<typename T> static T max_t(const T& a, const T& b) { return a > b ? a : b; }
	template<typename T> static T lerp_t(const T& a, const T& b, float t) { return a + (b-a)*t; }
	template<typename T> static T clamp_t(const T& x, const T& low, const T& high) { return x < low ? low : (x > high ? high : x); }
	// Returns the polar angle of the point (x,y) in [0, 2*PI).
	static float angle_from_xy(float x, float y);
	static DirectX::XMVECTOR spherical_to_cartesian(float radius, float theta, float phi);
	static DirectX::XMMATRIX inverse_transpose(DirectX::CXMMATRIX M);
	static DirectX::XMVECTOR rand_unit_vec3();
	static DirectX::XMVECTOR rand_hemisphere_unit_vec3(DirectX::XMVECTOR n);
	//
	static DirectX::XMFLOAT4X4 identity4x4() {
		static DirectX::XMFLOAT4X4 I(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		return I;
	}
	static const float infinity;
	static const float pi;
};
const float math_helper::infinity = FLT_MAX;
const float math_helper::pi = 3.1415926535f;
float math_helper::angle_from_xy(float x, float y)
{
	float theta = 0.0f;
	// Quadrant I or IV
	if(x >= 0.0f) {
		// If x = 0, then atanf(y/x) = +pi/2 if y > 0
		//                atanf(y/x) = -pi/2 if y < 0
		theta = atanf(y / x); // in [-pi/2, +pi/2]
		if(theta < 0.0f)
			theta += 2.0f*math_helper::pi; // in [0, 2*pi).
	}
	// Quadrant II or III
	else {
		theta = atanf(y/x) + math_helper::pi; // in [0, 2*pi).
	}
	return theta;
}
//
DirectX::XMVECTOR math_helper::spherical_to_cartesian(float radius, float theta, float phi)
{
	return DirectX::XMVectorSet(
		radius*sinf(phi)*cosf(theta),
		radius*cosf(phi),
		radius*sinf(phi)*sinf(theta),
		1.0f);
}
//
DirectX::XMMATRIX math_helper::inverse_transpose(DirectX::CXMMATRIX M)
{
	// Inverse-transpose is just applied to normals.  So zero out 
	// translation row so that it doesn't get into our inverse-transpose
	// calculation--we don't want the inverse-transpose of the translation.
	DirectX::XMMATRIX A = M;
	A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
	return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
}
//
DirectX::XMVECTOR math_helper::rand_unit_vec3() {
	XMVECTOR one  = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	// Keep trying until we get a point on/in the hemisphere.
	while (true) {
		// Generate random point in the cube [-1,1]^3.
		XMVECTOR v = XMVectorSet(rand_f(-1.0f, 1.0f), rand_f(-1.0f, 1.0f), rand_f(-1.0f, 1.0f), 0.0f);
		// Ignore points outside the unit sphere in order to get an even distribution 
		// over the unit sphere.  Otherwise points will clump more on the sphere near 
		// the corners of the cube.
		if( XMVector3Greater( XMVector3LengthSq(v), one) ) continue;
		return XMVector3Normalize(v);
	}
}
//
DirectX::XMVECTOR math_helper::rand_hemisphere_unit_vec3(DirectX::XMVECTOR n) {
	XMVECTOR one  = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR zero = XMVectorZero();
	// Keep trying until we get a point on/in the hemisphere.
	while (true) {
		// Generate random point in the cube [-1,1]^3.
		XMVECTOR v = XMVectorSet(rand_f(-1.0f, 1.0f), rand_f(-1.0f, 1.0f), rand_f(-1.0f, 1.0f), 0.0f);
		// Ignore points outside the unit sphere in order to get an even distribution 
		// over the unit sphere.  Otherwise points will clump more on the sphere near 
		// the corners of the cube.
		if( XMVector3Greater( XMVector3LengthSq(v), one) ) continue;
		// Ignore points in the bottom hemisphere.
		if( XMVector3Less( XMVector3Dot(n, v), zero ) ) continue;
		return XMVector3Normalize(v);
	}
}
////////////////
// 
////////////////
////////////////
template<typename T>
class upload_buffer
{
public:
	upload_buffer(ID3D12Device* device, UINT element_count, bool is_constant_buffer)
	{
		m_IsConstantBuffer = is_constant_buffer;
		m_ElementbyteSize = sizeof(T);
		// Constant buffer elements need to be multiples of 256 bytes.
		// This is because the hardware can only view constant data 
		// at m*256 byte offsets and of n*256 byte lengths. 
		// typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
		// UINT64 OffsetInBytes; // multiple of 256
		// UINT   SizeInBytes;   // multiple of 256
		// } D3D12_CONSTANT_BUFFER_VIEW_DESC;
		if (is_constant_buffer) m_ElementbyteSize = d3d_util::calc_constant_buffer_byte_size(sizeof(T));
		CD3DX12_HEAP_PROPERTIES heap_upload(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resource_desc_buffer = CD3DX12_RESOURCE_DESC::Buffer(m_ElementbyteSize*element_count);
		AbortIfFailed(device->CreateCommittedResource(
			&heap_upload,
			D3D12_HEAP_FLAG_NONE,
			&resource_desc_buffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_UploadBuffer)));
		AbortIfFailed(m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData)));
		// We do not need to unmap until we are done with the resource.  However, we must not write to
		// the resource while it is in use by the GPU (so we must use synchronization techniques).
	}
	upload_buffer(const upload_buffer& rhs) = delete;
	upload_buffer& operator=(const upload_buffer& rhs) = delete;
	~upload_buffer()
	{
		if (m_UploadBuffer != nullptr) m_UploadBuffer->Unmap(0, nullptr);
		m_MappedData = nullptr;
	}
	ID3D12Resource* resource() const
	{
		return m_UploadBuffer.Get();
	}
	void copy_data(int element_index, const T& data)
	{
		memcpy(&m_MappedData[element_index*m_ElementbyteSize], &data, sizeof(T));
	}
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
	BYTE* m_MappedData = nullptr;
	UINT m_ElementbyteSize = 0;
	bool m_IsConstantBuffer = false;
};
////////////////
// 
////////////////
////////////////
class d3d_util
{
public:
	static bool is_key_down(int v_key_code);
	static std::string to_string(HRESULT hr);
	static UINT calc_constant_buffer_byte_size(UINT byte_size)
	{
		// Constant buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes).  So round up to nearest
		// multiple of 256.  We do this by adding 255 and then masking off
		// the lower 2 bytes which store all bits < 256.
		// Example: Suppose byte_size = 300.
		// (300 + 255) & ~255
		// 555 & ~255
		// 0x022B & ~0x00ff
		// 0x022B & 0xff00
		// 0x0200
		// 512
		return (byte_size + 255) & ~255;
	}
	static Microsoft::WRL::ComPtr<ID3DBlob> load_binary(const std::wstring& filename);
	static Microsoft::WRL::ComPtr<ID3D12Resource> create_default_buffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmd_list,
		const void* init_data,
		UINT64 byte_size,
		Microsoft::WRL::ComPtr<ID3D12Resource>& upload_buffer);
	static Microsoft::WRL::ComPtr<ID3DBlob> compile_shader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);
};
ComPtr<ID3DBlob> d3d_util::compile_shader(
	const std::wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint,
	const std::string& target)
{
	UINT compile_flags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	HRESULT hr_compile = S_OK;
	ComPtr<ID3DBlob> byte_code = nullptr;
	ComPtr<ID3DBlob> errors;
	hr_compile = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compile_flags, 0, &byte_code, &errors);
	if(errors != nullptr) OutputDebugStringA((char*)errors->GetBufferPointer());
	AbortIfFailed(hr_compile);
	return byte_code;
}
//
Microsoft::WRL::ComPtr<ID3D12Resource> d3d_util::create_default_buffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmd_list,
	const void* init_data,
	UINT64 byte_size,
	Microsoft::WRL::ComPtr<ID3D12Resource>& upload_buffer)
{
	ComPtr<ID3D12Resource> default_buffer;
	// Create the actual default buffer resource.
	CD3DX12_HEAP_PROPERTIES heap_default(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resource_desc_buffer = CD3DX12_RESOURCE_DESC::Buffer(byte_size);
	CD3DX12_HEAP_PROPERTIES heap_upload(D3D12_HEAP_TYPE_UPLOAD);
	AbortIfFailed(device->CreateCommittedResource(
		&heap_default,
		D3D12_HEAP_FLAG_NONE,
		&resource_desc_buffer,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(default_buffer.GetAddressOf())));
	// In order to copy CPU memory data into our default buffer, we need to create
	// an intermediate upload heap. 
	AbortIfFailed(device->CreateCommittedResource(
		&heap_upload,
		D3D12_HEAP_FLAG_NONE,
		&resource_desc_buffer,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(upload_buffer.GetAddressOf())));
	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA sub_resource_data = {};
	sub_resource_data.pData = init_data;
	sub_resource_data.RowPitch = byte_size;
	sub_resource_data.SlicePitch = sub_resource_data.RowPitch;
	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	const D3D12_RESOURCE_BARRIER reso_barrier(
		CD3DX12_RESOURCE_BARRIER::Transition(
			default_buffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COPY_DEST));
	cmd_list->ResourceBarrier(1, &reso_barrier);
	UpdateSubresources<1>(cmd_list, default_buffer.Get(), upload_buffer.Get(), 0, 0, 1, &sub_resource_data);
	const D3D12_RESOURCE_BARRIER reso_barrier2(
		CD3DX12_RESOURCE_BARRIER::Transition(
			default_buffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_GENERIC_READ));
	cmd_list->ResourceBarrier(1, &reso_barrier2);
	// Note: upload_buffer has to be kept alive after the above function calls because
	// the command list has not been executed yet that performs the actual copy.
	// The caller can Release the upload_buffer after it knows the copy has been executed.
	return default_buffer;
}
////////////////
// mesh_geometry
////////////////
////////////////
// Defines a subrange of geometry in a mesh_geometry.  This is for when multiple
// geometries are stored in one vertex and index buffer.  It provides the offsets
// and data needed to draw a subset of geometry stores in the vertex and index 
// buffers so that we can implement the technique described by Figure 6.3.
struct submesh_geometry
{
	UINT index_count = 0;
	UINT start_index_location = 0;
	INT base_vertex_location = 0;
	// Bounding box of the geometry defined by this submesh. 
	// This is used in later chapters of the book.
	DirectX::BoundingBox bounds;
};
//
struct mesh_geometry
{
	// Give it a name so we can look it up by name.
	std::string name;
	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
	Microsoft::WRL::ComPtr<ID3DBlob> vertex_buffer_cpu = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> index_buffer_cpu  = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer_gpu = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> index_buffer_gpu = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertex_buffer_uploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> index_buffer_uploader = nullptr;
	// Data about the buffers.
	UINT vertex_byte_stride = 0;
	UINT vertex_buffer_byte_size = 0;
	DXGI_FORMAT index_format = DXGI_FORMAT_R16_UINT;
	UINT index_buffer_byte_size = 0;
	// A mesh_geometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, submesh_geometry> draw_args;
	D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view() const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = vertex_buffer_gpu->GetGPUVirtualAddress();
		vbv.StrideInBytes = vertex_byte_stride;
		vbv.SizeInBytes = vertex_buffer_byte_size;
		return vbv;
	}
	D3D12_INDEX_BUFFER_VIEW index_buffer_view() const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = index_buffer_gpu->GetGPUVirtualAddress();
		ibv.Format = index_format;
		ibv.SizeInBytes = index_buffer_byte_size;
		return ibv;
	}
	// We can free this memory after we finish upload to the GPU.
	void dispose_uploaders()
	{
		vertex_buffer_uploader = nullptr;
		index_buffer_uploader = nullptr;
	}
};
}
#endif