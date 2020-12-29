#ifndef GFX_HPP
#define GFX_HPP

#define _USE_MATH_DEFINES
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

//#include "shader_program.hpp"
//#include "fox/counter.hpp"

class gfx
{
public:
	void init(int win_w, int win_h);
	void deinit();
	void resize(int win_w, int win_h);
	void render();
	void process_gui_events(SDL_Event &event);

private:
	void load_shaders();
	void print_shader_info_log(GLuint shader_id);

	int win_w;
	int win_h;

	//std::unique_ptr<fox::counter> fps_counter;
	double render_time;
	int frames, framerate;

	SDL_Window* window;
	SDL_GLContext context;
	// an empty vertex array object to bind to
	uint32_t default_vao;
	//std::unique_ptr<shader_program> sp;

	Eigen::Vector3f eye, target, up;
	Eigen::Affine3f V;
	Eigen::Projective3f P;
	// model matrix (specific to the model instance)
	Eigen::Projective3f MVP;
	Eigen::Affine3f M, MV;
	// TODO: should this be a pointer from the parent?
	// TODO: should this be Affine3f ?
	Eigen::Matrix3f normal_matrix;
	// more shader uniforms
	Eigen::Vector4f light_pos, color;
	Eigen::Vector3f La, Ls, Ld;
	Eigen::Vector3f rot, trans;
	float scale;

	Eigen::Vector3f Ka, Ks, Kd;
	float shininess;

	GLuint fast_vertex_vbo;
	GLuint fast_normal_vbo;

	std::vector<float> vertex;
};

#endif
