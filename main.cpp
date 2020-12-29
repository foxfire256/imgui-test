
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

#include <SDL2/SDL.h>

#include "gfx.hpp"

int done;
int win_w, win_h;

int main(int argc, char** argv)
{
	win_w = 640;
	win_h = 480;
	done = 0;

	std::unique_ptr<gfx> g(new gfx());
	
	g.get()->init(win_w, win_h);

	SDL_Event event;
	while(!done)
	{
		while(SDL_PollEvent(&event))
		{
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
					g.get()->resize(event.window.data1, event.window.data2);
					break;
				default:
					break;
				}
			default:
				break;
			}
		}

		g.get()->render();
	}

	return 0;
}
