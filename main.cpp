
#define _USE_MATH_DEFINES

// hacks for NVidia Optimus
// 1 use NVidia, 0 don't unless maybe you set it as default
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000000;
}
#endif

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <SDL2/SDL.h>

#include "gfx.hpp"

int done;
int win_w, win_h;

int main(int argc, char** argv)
{
	win_w = 768;
	win_h = 768;
	done = 0;

	gfx::get_instance().init(win_w, win_h);

	SDL_Event event;
	while(!done)
	{
		while(SDL_PollEvent(&event))
		{
			bool want_mouse;
			bool want_keyboard;
			gfx::get_instance().process_gui_events(event, want_mouse, want_keyboard);
			switch(event.type)
			{
			case SDL_KEYDOWN:
				if(event.key.keysym.sym == SDLK_ESCAPE)
				{
					done = 1;
				}
				break;
			case SDL_KEYUP:
				break;
			case SDL_MOUSEBUTTONDOWN:
				std::cout << "Mouse button: " << std::to_string(event.button.button) << " at (" << event.button.x << ", "
					<< event.button.y << ")";
				if(want_mouse)
					std::cout << " <<ImGui wants the mouse>> ";
				std::cout << std::endl;
				break;
			case SDL_MOUSEMOTION:
				break;
			case SDL_QUIT:
				done = 1;
				break;
			case SDL_WINDOWEVENT:
				switch(event.window.event)
				{
				case SDL_WINDOWEVENT_CLOSE:
					done = 1;
					break;
				case SDL_WINDOWEVENT_RESIZED:
					gfx::get_instance().resize(event.window.data1, event.window.data2);
					break;
				default:
					break;
				}
			default:
				break;
			}
		}

		gfx::get_instance().render();
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}

	return 0;
}
