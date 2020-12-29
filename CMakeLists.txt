cmake_minimum_required(VERSION 3.8)
# cmake version 3.8 is required for CXX 17
project(imgui-test)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories("imgui")
include_directories("imgui/backends")
include_directories("common-cpp")

# git submodule add ssh://foxfire@foxfire256.ddns.net:27730/nas/dev/common-cpp common-cpp
# then manually add the following to .gitmodules
# url = git@github.com:foxfire256/common-cpp.git

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

# cmake .. -G "Visual Studio 16 2019" -A x64
if(MSVC)
	set(CMAKE_PREFIX_PATH "C:\\usr\\vclibs")
	set(CMAKE_INCLUDE_PATH "C:\\usr\\vclibs\\include")
	set(CMAKE_LIBRARY_PATH "C:\\usr\\vclibs\\lib")
	
	SET(CMAKE_C_FLAGS "/MP ${CMAKE_C_FLAGS}")
	SET(CMAKE_CXX_FLAGS "/MP /Oi ${CMAKE_CXX_FLAGS}")
	
	set(Boost_USE_STATIC_LIBS OFF )
	set(Boost_USE_MULTITHREADED ON)
	set(Boost_USE_STATIC_RUNTIME OFF)
	
	set(LIBS ${LIBS} kernel32.lib)
	
	include_directories("C:\\usr\\vclibs\\include")
endif(MSVC)

if(MINGW OR MSYS OR UNIX)
	if(UNIX)
        set(OpenGL_GL_PREFERENCE GLVND)
		set(LIBS ${LIBS} -lpthread)
	elseif(MINGW OR MSYS)
		set(LIBS ${LIBS} -lmingw32)
		set(QT_DIR "c:/msys64/mingw64")
	endif(UNIX)
	
	if(UNIX)
		include_directories("/usr/include")
		include_directories("/usr/include/eigen3")
	elseif(MINGW OR MSYS)
		include_directories("c:/msys64/mingw64/include")
		include_directories("c:/msys64/mingw64/include/eigen3")
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
	imgui/imgui_draw.cpp
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
	imgui/backends/imgui_impl_opengl3.h
	imgui/backends/imgui_impl_opengl3.cpp
	imgui/backends/imgui_impl_sdl.h
	imgui/backends/imgui_impl_sdl.cpp
)

find_package(OpenGL REQUIRED)

find_package(Boost COMPONENTS program_options system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

if(BUILD_SHARED_LIBS)
	set(LIBRARY_TYPE SHARED)
else()
	set(LIBRARY_TYPE STATIC)
endif()

add_executable(${PROJECT_NAME} ${MAIN_SOURCE} ${IMGUI_SOURCE})
target_link_libraries(${PROJECT_NAME} 
	${LIBS}
	${Boost_LIBRARIES}
	${OPENGL_LIBRARY}
	GLEW::GLEW
	SDL2::SDL2main
	SDL2::SDL2)

if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif(MSVC)
