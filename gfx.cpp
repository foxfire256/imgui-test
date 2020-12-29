#include "gfx.hpp"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

//#include "fox/gfx/eigen_opengl.hpp"
//#include "shader.hpp"
//#include "uniform.hpp"

#define print_opengl_error() print_opengl_error2((char *)__FILE__, __LINE__)
int print_opengl_error2(char* file, int line);

#if defined(_WIN32) || defined(_WIN64)
std::string data_root = "C:/dev/imgui-testing";
#else // Linux or Unix
std::string data_root = std::string(getenv("HOME")) + "/dev/imgui-testing";
#endif

void gfx::init(int win_w, int win_h)
{
	this->win_w = win_w;
	this->win_h = win_h;

	render_time = 0.0;
	frames = framerate = 0;
	//fps_counter = std::unique_ptr<fox::counter>(new fox::counter());

	std::string window_title = "imgui testing";

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
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

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
	print_opengl_error();
	fflush(stdout);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
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

	print_opengl_error();

	trans = { 0.0f, 0.0f, 0.0f };
	rot = { 0.0f, 0.0f, 0.0f };

	eye = Eigen::Vector3f(0.0f, 0.0f, 3.0f);
	target = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
	up = Eigen::Vector3f(0.0f, 1.0f, 0.0f);

	//fox::gfx::look_at(eye, target, up, V);

	// initialize some defaults
	color = Eigen::Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
	/*
	La = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
	Ls = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
	Ld = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
	*/
	rot = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
	trans = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
	scale = 1.0f;
	//light_pos = Eigen::Vector4f(eye[0], eye[1], eye[2], 1.0f);
	M = Eigen::Matrix4f::Identity();
	//fox::gfx::perspective(65.0f, (float)win_w / (float)win_h, 0.01f, 40.0f, P);

	MV = V * M;

	normal_matrix(0, 0) = MV(0, 0);
	normal_matrix(0, 1) = MV(0, 1);
	normal_matrix(0, 2) = MV(0, 2);

	normal_matrix(1, 0) = MV(1, 0);
	normal_matrix(1, 1) = MV(1, 1);
	normal_matrix(1, 2) = MV(1, 2);

	normal_matrix(2, 0) = MV(2, 0);
	normal_matrix(2, 1) = MV(2, 1);
	normal_matrix(2, 2) = MV(2, 2);

	MVP = P * MV;

	resize(win_w, win_h);

	print_opengl_error();

	load_shaders();

	print_opengl_error();

	vertex.resize(3 * 3);
	int i = 0;
	vertex =
	{
		1.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	glGenBuffers(1, &fast_vertex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, fast_vertex_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 3,
		vertex.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	print_opengl_error();

	
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

void gfx::process_gui_events(SDL_Event& event)
{
	ImGui_ImplSDL2_ProcessEvent(&event);
}

void gfx::resize(int win_w, int win_h)
{
	this->win_w = win_w;
	this->win_h = win_h;

	glViewport(0, 0, win_w, win_h);
	//fox::gfx::perspective(65.0f, (float)win_w / (float)win_h, 0.01f, 40.0f, P);

	MVP = P * MV;
	/*
	if(sp && sp.get()->id)
	{
		glUseProgram(sp.get()->id);
		int u = glGetUniformLocation(sp.get()->id, "MVP");
		glUniformMatrix4fv(u, 1, GL_FALSE, MVP.data());
	}
	*/
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

	glClear(GL_COLOR_BUFFER_BIT);// | GL_DEPTH_BUFFER_BIT);

	//float dt = update_counter->update();
	//rot[1] = rot_vel * dt;
	M = M * Eigen::AngleAxisf(rot[0] / 180.0f * (float)M_PI,
		Eigen::Vector3f::UnitX());
	M = M * Eigen::AngleAxisf(rot[1] / 180.0f * (float)M_PI,
		Eigen::Vector3f::UnitY());
	M = M * Eigen::AngleAxisf(rot[2] / 180.0f * (float)M_PI,
		Eigen::Vector3f::UnitZ());

	MV = V * M;

	normal_matrix(0, 0) = MV(0, 0);
	normal_matrix(0, 1) = MV(0, 1);
	normal_matrix(0, 2) = MV(0, 2);

	normal_matrix(1, 0) = MV(1, 0);
	normal_matrix(1, 1) = MV(1, 1);
	normal_matrix(1, 2) = MV(1, 2);

	normal_matrix(2, 0) = MV(2, 0);
	normal_matrix(2, 1) = MV(2, 1);
	normal_matrix(2, 2) = MV(2, 2);

	MVP = P * MV;

	/*
	glUseProgram(sp.get()->id);

	int u = glGetUniformLocation(sp.get()->id, "MVP");
	glUniformMatrix4fv(u, 1, GL_FALSE, MVP.data());

	glBindBuffer(GL_ARRAY_BUFFER, fast_vertex_vbo);
	glEnableVertexAttribArray(sp.get()->vertex_location);
	glVertexAttribPointer(sp.get()->vertex_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_POINTS, 0, 3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	frames++;

	render_time += fps_counter.get()->update_double();
	if(render_time >= 1.0)
	{
		framerate = frames;
		frames = 0;
		std::cout << "fps: " << framerate << std::endl;
		render_time = 0.0;
	}
	*/
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	print_opengl_error();

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		//ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
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
	print_opengl_error();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(window);
}

void gfx::load_shaders()
{
	print_opengl_error();
	/*

	std::string vert_file_name = data_root + "/shaders/3d_triangles.vert";
	std::string geom_file_name = data_root + "/shaders/3d_triangles.geom";
	std::string frag_file_name = data_root + "/shaders/3d_triangles.frag";

	std::ifstream v_f;
	std::ifstream g_f;
	std::ifstream f_f;

	v_f.open(vert_file_name, std::ios::in | std::ios::binary);
	if(!v_f.is_open())
	{
		std::cerr << "Failed to open file: " << vert_file_name << std::endl;
		exit(-1);
	}
	std::string vert_s;
	vert_s.assign(std::istreambuf_iterator<char>(v_f), std::istreambuf_iterator<char>());

	g_f.open(geom_file_name, std::ios::in | std::ios::binary);
	if(!g_f.is_open())
	{
		std::cerr << "Failed to open file: " << geom_file_name << std::endl;
		exit(-1);
	}
	std::string geom_s;
	geom_s.assign(std::istreambuf_iterator<char>(g_f), std::istreambuf_iterator<char>());

	f_f.open(frag_file_name, std::ios::in | std::ios::binary);
	if(!f_f.is_open())
	{
		std::cerr << "Failed to open file: " << frag_file_name << std::endl;
		exit(-1);
	}
	std::string frag_s;
	frag_s.assign(std::istreambuf_iterator<char>(f_f), std::istreambuf_iterator<char>());

	v_f.close();
	g_f.close();
	f_f.close();

	int length = 0, chars_written = 0;
	char* info_log;

	sp = std::unique_ptr<shader_program>(new shader_program());
	sp.get()->vertex_shader = std::unique_ptr<shader>(new shader());
	sp.get()->geometry_shader = std::unique_ptr<shader>(new shader());
	sp.get()->fragment_shader = std::unique_ptr<shader>(new shader());

	// vertex shader first
	sp.get()->vertex_shader.get()->id = glCreateShader(GL_VERTEX_SHADER);
	if(sp.get()->vertex_shader.get()->id == 0)
	{
		printf("Failed to create GL_VERTEX_SHADER!\n");
		return;
	}

	print_opengl_error();

	const char* vert_c = vert_s.c_str();
	glShaderSource(sp.get()->vertex_shader.get()->id, 1, &vert_c, NULL);
	glCompileShader(sp.get()->vertex_shader.get()->id);

	GLint success = 0;
	glGetShaderiv(sp.get()->vertex_shader.get()->id, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE)
	{
		std::cerr << "Failed to compile vertex shader!" << std::endl;
	}

	print_opengl_error();

	print_shader_info_log(sp.get()->vertex_shader.get()->id);

	sp.get()->geometry_shader.get()->id = glCreateShader(GL_GEOMETRY_SHADER);
	if(sp.get()->geometry_shader.get()->id == 0)
	{
		printf("Failed to create GL_GEOMETRY_SHADER!\n");
		return;
	}

	print_opengl_error();

	const char* geom_c = geom_s.c_str();
	glShaderSource(sp.get()->geometry_shader.get()->id, 1, &geom_c, NULL);
	glCompileShader(sp.get()->geometry_shader.get()->id);

	glGetShaderiv(sp.get()->geometry_shader.get()->id, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE)
	{
		std::cerr << "Failed to compile geometry shader!" << std::endl;
	}

	print_opengl_error();

	print_shader_info_log(sp.get()->vertex_shader.get()->id);

	sp.get()->fragment_shader.get()->id = glCreateShader(GL_FRAGMENT_SHADER);
	if(sp.get()->fragment_shader.get()->id == 0)
	{
		printf("Failed to create GL_FRAGMENT_SHADER!\n");
		return;
	}

	print_opengl_error();

	const char* frag_c = frag_s.c_str();
	glShaderSource(sp.get()->fragment_shader.get()->id, 1, &frag_c, NULL);
	glCompileShader(sp.get()->fragment_shader.get()->id);
	
	glGetShaderiv(sp.get()->fragment_shader.get()->id, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE)
	{
		std::cerr << "Failed to compile fragment shader!" << std::endl;
	}

	print_opengl_error();

	print_shader_info_log(sp.get()->fragment_shader.get()->id);

	sp.get()->id = glCreateProgram();
	if(sp.get()->id == 0)
	{
		printf("Failed at glCreateProgram()!\n");
		return;
	}

	print_opengl_error();

	glAttachShader(sp.get()->id, sp.get()->vertex_shader.get()->id);
	print_opengl_error();
	glAttachShader(sp.get()->id, sp.get()->geometry_shader.get()->id);
	print_opengl_error();
	glAttachShader(sp.get()->id, sp.get()->fragment_shader.get()->id);
	print_opengl_error();

	glLinkProgram(sp.get()->id);
	print_opengl_error();

	glGetProgramiv(sp.get()->id, GL_INFO_LOG_LENGTH, &length);
	print_opengl_error();

	// use 2 for the length because NVidia cards return a line feed always
	if(length > 4)
	{
		info_log = (char*)malloc(sizeof(char) * length);
		if(info_log == NULL)
		{
			printf("ERROR couldn't allocate %d bytes for shader program info log!\n",
				length);
			return;
		}
		else
		{
			printf("Shader program info log:\n");
		}

		glGetProgramInfoLog(sp.get()->id, length, &chars_written, info_log);

		printf("%s\n", info_log);

		free(info_log);
	}

	glUseProgram(sp.get()->id);
	print_opengl_error();
	sp.get()->vertex_location = glGetAttribLocation(sp.get()->id, "pos");
	print_opengl_error();
	//normal_location = glGetAttribLocation(sp.get()->id, "vertex_normal");
	*/
}

//------------------------------------------------------------------------------
void gfx::print_shader_info_log(GLuint shader_id)
{
	int length = 0, chars_written = 0;
	char* info_log;

	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);

	// use 2 for the length because NVidia cards return a line feed always
	if(length > 4)
	{
		info_log = (char*)malloc(sizeof(char) * length);
		if(info_log == NULL)
		{
			printf("ERROR couldn't allocate %d bytes for shader info log!\n",
				length);
			return;
		}

		glGetShaderInfoLog(shader_id, length, &chars_written, info_log);

		printf("Shader info log: %s\n", info_log);

		free(info_log);
	}
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

