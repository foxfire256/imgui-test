cmake_minimum_required(VERSION 3.8)
# cmake version 3.8 is required for CXX 17
project(imgui-test)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories("imgui")
include_directories("imgui/backends")

# git submodule add git@github.com:ocornut/imgui.git imgui

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# cmake .. -G "Visual Studio 16 2019" -A x64
if(MSVC)
	set(CMAKE_PREFIX_PATH "C:\\usr\\vclibs")
	set(CMAKE_INCLUDE_PATH "C:\\usr\\vclibs\\include")
	set(CMAKE_LIBRARY_PATH "C:\\usr\\vclibs\\lib")
	
	SET(CMAKE_C_FLAGS "/MP ${CMAKE_C_FLAGS}")
	SET(CMAKE_CXX_FLAGS "/MP /Oi ${CMAKE_CXX_FLAGS}")
	
	include_directories("C:\\usr\\vclibs\\include")
endif(MSVC)

if(MINGW OR MSYS OR UNIX)
	if(UNIX)
		set(OpenGL_GL_PREFERENCE GLVND)
		set(LIBS ${LIBS} -lpthread)
	elseif(MINGW OR MSYS)
		set(LIBS ${LIBS} -lmingw32)
	endif(UNIX)
	
	if(UNIX)
		include_directories("/usr/include")
	elseif(MINGW OR MSYS)
		include_directories("c:/msys64/mingw64/include")
	endif(UNIX)
endif(MINGW OR MSYS OR UNIX)

set(MAIN_SOURCE
	main.cpp
	gfx.hpp
	gfx.cpp
)
set(IMGUI_SOURCE
	imgui/imgui.h
	imgui/imgui.cpp
	imgui/imconfig.h
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
	imgui/backends/imgui_impl_opengl3.h
	imgui/backends/imgui_impl_opengl3.cpp
	imgui/backends/imgui_impl_sdl.h
	imgui/backends/imgui_impl_sdl.cpp
)

find_package(OpenGL REQUIRED)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

add_library(imgui STATIC ${IMGUI_SOURCE})
target_link_libraries(imgui
	GLEW::GLEW
	SDL2::SDL2main
	SDL2::SDL2
)

add_executable(${PROJECT_NAME} ${MAIN_SOURCE})
target_link_libraries(${PROJECT_NAME} 
	${LIBS}
	${OPENGL_LIBRARY}
	GLEW::GLEW
	SDL2::SDL2main
	SDL2::SDL2
	imgui
)

if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif(MSVC)
