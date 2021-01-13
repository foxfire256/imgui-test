#include "gfx.hpp"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#define print_opengl_error() print_opengl_error2((char *)__FILE__, __LINE__)
int print_opengl_error2(char* file, int line);

bool gfx::show_demo_window = true;

void gfx::init(int win_w, int win_h)
{
	this->win_w = win_w;
	this->win_h = win_h;

	first_pass = true;

	std::string window_title = "ImGUI Testing";

	int ret;
	ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	if(ret < 0)
	{
		printf("Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_version compiled;
	SDL_version linked;

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);
	printf("SDL2 compiled version %d.%d.%d\n",
		compiled.major, compiled.minor, compiled.patch);
	printf("SDL2 linked version %d.%d.%d\n",
		linked.major, linked.minor, linked.patch);
	printf("SDL2 revision number: %s\n", SDL_GetRevision());

	window = SDL_CreateWindow(window_title.c_str(),
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		win_w, win_h,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if(!window)
	{
		printf("Couldn't create window: %s\n", SDL_GetError());
		SDL_Quit();
		exit(-1);
	}

	SDL_ShowWindow(window);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_RETAINED_BACKING, 1);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	//SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
		SDL_GL_CONTEXT_PROFILE_CORE);

	context = SDL_GL_CreateContext(window);

	ret = SDL_GL_MakeCurrent(window, context);
	if(ret)
	{
		printf("ERROR could not make GL context current after init!\n");
		if(window)
			SDL_DestroyWindow(window);

		SDL_Quit();
		exit(1);
	}

	// 0 = no vsync
	SDL_GL_SetSwapInterval(0);

	std::cout << "Running on platform: " << SDL_GetPlatform() << std::endl;
	std::cout << "Number of logical CPU cores: " << SDL_GetCPUCount() << std::endl;
	int ram_mb = SDL_GetSystemRAM();
	char buffer[8];
	snprintf(buffer, 8, "%.1f", ram_mb / 1024.0f);
	std::cout << "System RAM " << ram_mb << "MB (" << buffer << " GB)\n";

	// init glew first
	glewExperimental = GL_TRUE; // Needed in core profile
	if(glewInit() != GLEW_OK)
	{
		printf("Failed to initialize GLEW\n");
		exit(-1);
	}

	// HACK: to get around initial glew error with core profiles
	GLenum gl_err = glGetError();
	while(gl_err != GL_NO_ERROR)
	{
		//printf("glError in file %s @ line %d: %s (after glew init)\n",
		//	(char *)__FILE__, __LINE__, gluErrorString(gl_err));
		gl_err = glGetError();
	}

	printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
	printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
	printf("GL_SHADING_LANGUAGE_VERSION: %s\n",
		glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("GLEW library version %s\n", glewGetString(GLEW_VERSION));
	GLint out_verts;
	glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &out_verts);
	printf("GL_MAX_GEOMETRY_OUTPUT_VERTICES: %d\n", out_verts);
	if(print_opengl_error())
	{
		exit(-1);
	}
	fflush(stdout);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImFontConfig config;
	config.OversampleH = 2;
	config.OversampleV = 1;
	config.GlyphExtraSpacing.x = 1.0f;
	config.SizePixels = 16.0f;
	// the first font will be the default one used
	font_16 = io.Fonts->AddFontDefault(&config);
	config.SizePixels = 12.0f;
	font_12 = io.Fonts->AddFontDefault(&config);
	config.SizePixels = 24.0f;
	font_24 = io.Fonts->AddFontDefault(&config);
	config.SizePixels = 32.0f;
	font_32 = io.Fonts->AddFontDefault(&config);

	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	const char* glsl_version = "#version 150";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// init basic OpenGL stuff
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//glEnable(GL_DEPTH_TEST);
	//glDisable(GL_BLEND); // alpha channel
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glEnable(GL_POLYGON_SMOOTH);
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	// OpenGL 3.2 core requires a VAO to be bound to use a VBO
	// WARNING: GLES 2.0 does not support VAOs
	glGenVertexArrays(1, &default_vao);
	glBindVertexArray(default_vao);

	if(print_opengl_error())
	{
		deinit();
		exit(-1);
	}
}

void gfx::deinit()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void gfx::process_gui_events(SDL_Event& event, bool& want_mouse, bool& want_keyboard)
{
	ImGui_ImplSDL2_ProcessEvent(&event);
	ImGuiIO& io = ImGui::GetIO();
	want_mouse = io.WantCaptureMouse;
	want_keyboard = io.WantCaptureKeyboard;
}

void gfx::resize(int win_w, int win_h)
{
	this->win_w = win_w;
	this->win_h = win_h;

	glViewport(0, 0, win_w, win_h);
}

void gfx::render()
{
	int status = SDL_GL_MakeCurrent(window, context);
	if(status)
	{
		printf("SDL_GL_MakeCurrent() failed in render(): %s\n",
			SDL_GetError());
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);

		SDL_Quit();
		exit(1);
	}

	glClear(GL_COLOR_BUFFER_BIT);


	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	if(first_pass)
	{
		// this doesn't work so well for the demo window
		ImVec2 pos = { 1, 1 };
		ImGui::SetNextWindowPos(pos);
		ImVec2 size = { (float)win_w - 2, (float)win_h - 2 };
		ImGui::SetNextWindowSize(size);
		std::cout << "Set window pos = " << pos[0] << ", " << pos[1] << std::endl;
		std::cout << "Set window size = " << size[0] << ", " << size[1] << std::endl;
	}

	if(print_opengl_error())
	{
		deinit();
		exit(-1);
	}

	show_demo_window = true;

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if(show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	if(!show_demo_window)
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		//ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if(ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
	if(print_opengl_error())
	{
		deinit();
		exit(-1);
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if(print_opengl_error())
	{
		deinit();
		exit(-1);
	}

	if(first_pass)
	{
		first_pass = false;
	}

	SDL_GL_SwapWindow(window);
}


//------------------------------------------------------------------------------
// Returns 1 if an OpenGL error occurred, 0 otherwise.
int print_opengl_error2(char* file, int line)
{
	GLenum gl_err;
	int	ret_code = 0;

	gl_err = glGetError();
	while(gl_err != GL_NO_ERROR)
	{
		printf("glError in file %s @ line %d: %s\n", file, line,
			gluErrorString(gl_err));

		ret_code = 1;
		gl_err = glGetError();
	}
	return ret_code;
}

