cmake_minimum_required(VERSION 3.15)

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
project(project-web)
else()
project(projectx)
endif()

set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE) 
set(CMAKE_BUILD_TYPE "Release")
endif()


# add_subdirectory(external/freetype)


include(cmake/CompilerFlags.cmake)
include(cmake/External.cmake)

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
set(USE_FLAGS "-std=c++20 -s USE_SDL=2 -s FULL_ES3=1 -s OFFSCREEN_FRAMEBUFFER=1")
set(USE_FLAGS "${USE_FLAGS} -s MIN_WEBGL_VERSION=1 -s MAX_WEBGL_VERSION=2")
set(USE_FLAGS "${USE_FLAGS} -s TOTAL_STACK=32MB")
set(USE_FLAGS "${USE_FLAGS} -s NO_DISABLE_EXCEPTION_CATCHING")
set(USE_FLAGS "${USE_FLAGS} -s -sASSERTIONS=2 -sALLOW_MEMORY_GROWTH=1")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS} ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --embed-file ../scripts  --embed-file ../external/lygia ") 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --embed-file ../Resources/Shaders --embed-file ../Resources/Textures --embed-file ../Resources/Fonts")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --embed-file ../Resources/GrassTiles")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}   -L../external/lua/src/liblua.a -I ../external/lua/src")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --embed-file ../scripts  --embed-file ../external/lygia ") 
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --embed-file ../Resources/Shaders --embed-file ../Resources/Textures --embed-file ../Resources/Fonts")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --embed-file ../Resources/GrassTiles")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
set(CMAKE_EXECUTABLE_SUFFIX .html)
endif()



file(GLOB SRC
     "src/*.h"
     "src/*.cpp"
     "src/Utils/*.h"
     "src/Utils/*.cpp"
     "src/PathFinding/*.h"
     "src/PathFinding/*.cpp"
     "src/Shadows/*.h"
     "src/Shadows/*.cpp"
)

include_directories(SYSTEM external/stbimage)
include_directories(SYSTEM external/magic_enum)
include_directories(SYSTEM external)
include_directories(SYSTEM external/lua/src)
include_directories(SYSTEM external/freetype/include)
include_directories("${PROJECT_BINARY_DIR}" ${IMGUI_DIR} ${IMGUI_DIR}/backends ${BOOST_INCLUDE_DIRS} ${LUA_INCLUDE_DIRS} ${LUABRIDG3E_INCLUDE_DIRS})
add_executable(${CMAKE_PROJECT_NAME} ${SRC} )
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC LuaLib
                                            renderer CDT
                                            ${LUABRIDG3E_LIBRARIES}
                                            ${BOOST_LIBRARIES}
                                            freetype) #flecs::flecs_static

message(${CMAKE_SOURCE_DIR})