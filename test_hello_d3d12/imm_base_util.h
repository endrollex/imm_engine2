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
// static
////////////////
////////////////
namespace glo{
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
}
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
//
#ifndef AbortIfFailed
#define AbortIfFailed(x) \
{ \
	HRESULT hr_get = (x); \
	if(FAILED(hr_get)) {MessageBoxA(NULL, format_hr(hr_get, cha_ws2s(L#x)).c_str(), "HRESULT", MB_OK); abort();} \
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
}
#endif