#ifndef GFX_HPP
#define GFX_HPP

#define _USE_MATH_DEFINES
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

class gfx
{
public:
	static gfx& get_instance()
	{
		static gfx instance;
		return instance;
	}
	void init(int win_w, int win_h);
	void deinit();
	void resize(int win_w, int win_h);
	void render();
	void process_gui_events(SDL_Event& event, bool& want_mouse, bool& want_keyboard);

private:
	/**
	 * @brief Make the constructor private so we can make this thing a singleton
	 */
	gfx() = default;

	int win_w;
	int win_h;

	//std::unique_ptr<fox::counter> fps_counter;
	double render_time;
	int frames, framerate;

	SDL_Window* window;
	SDL_GLContext context;
	// an empty vertex array object to bind to
	uint32_t default_vao;

	static bool show_demo_window;
};

#endif
