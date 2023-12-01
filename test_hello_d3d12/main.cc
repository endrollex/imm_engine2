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
using namespace imm;
class imm_app: public base_win<imm_app>
{
public:
	void on_resize_drived();
	void update_scene(float dt) {DUMMY(dt);}
	void draw_scene();
	imm_app();
	~imm_app();
	bool init_imm();
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
}
void imm_app::draw_scene()
{
	on_render_blank();
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
	return true;
}
