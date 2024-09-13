cmake_minimum_required(VERSION 3.15)

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
project(project-web)
else()
project(projectx)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

if(NOT CMAKE_BUILD_TYPE) 
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()


include(FetchContent)
FetchContent_Declare(
  renderer  
  GIT_REPOSITORY https://github.com/Smutekj/simple-emscripten-renderer
  GIT_TAG testing
)
FetchContent_MakeAvailable(renderer)

FetchContent_Declare(
  CDT
  GIT_REPOSITORY https://github.com/Smutekj/CDT
  GIT_TAG main 
)
FetchContent_MakeAvailable(CDT)

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
add_custom_target(
   BuildLua
   COMMAND emmake make generic 
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/lua/src/
)
else()
add_custom_target(
   BuildLua
   COMMAND make generic
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/lua/src/
)
endif()

add_library(LuaLib STATIC IMPORTED)
set_target_properties(LuaLib
PROPERTIES
  IMPORTED_LOCATION    ${CMAKE_SOURCE_DIR}/external/lua/src/liblua.a # Make sure to use absolute path here
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/external/lua/src/
  INTERFACE_COMPILE_DEFINITIONS "USING_LUA;LUA_STATIC"
)
add_dependencies(LuaLib BuildLua) # So that anyone linking against TheLib causes BuildTheLib to build first

FetchContent_Declare(
  LuaBridge
  GIT_REPOSITORY https://github.com/vinniefalco/LuaBridge
  GIT_TAG master 
)
FetchContent_MakeAvailable(LuaBridge)

# set(BOOST_INCLUDE_LIBRARIES mpl)
# set(BOOST_ENABLE_CMAKE ON)
# FetchContent_Declare(
#   boost
#   GIT_REPOSITORY https://github.com/boostorg/boost.git
#   GIT_TAG boost-1.80.0
# )
# FetchContent_MakeAvailable(boost)




find_package(Freetype REQUIRED)
# find_package(Boost REQUIRED)

if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
set(USE_FLAGS "-std=c++20 -s USE_SDL=2 -s USE_FREETYPE=1 -s FULL_ES3=1 -s OFFSCREEN_FRAMEBUFFER=1")
set(USE_FLAGS "${USE_FLAGS} -s MIN_WEBGL_VERSION=1 -s MAX_WEBGL_VERSION=2")
set(USE_FLAGS "${USE_FLAGS} -s TOTAL_STACK=32MB")
set(USE_FLAGS "${USE_FLAGS} -s NO_DISABLE_EXCEPTION_CATCHING")
set(USE_FLAGS "${USE_FLAGS} -s -sASSERTIONS=2 -sALLOW_MEMORY_GROWTH=1")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}  --embed-file ../Resources --embed-file ../external/lygia --embed-file /home/smutekj/projects/ZombieShooterEm3/Resources")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS} -I../external/lua/src --embed-file ../Resources --embed-file ../external/lygia --embed-file /home/smutekj/projects/ZombieShooterEm3/Resources")
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
include_directories(SYSTEM ${FREETYPE_INCLUDE_DIRS})
include_directories("${PROJECT_BINARY_DIR}" ${IMGUI_DIR} ${IMGUI_DIR}/backends ${BOOST_INCLUDE_DIRS} ${LUA_INCLUDE_DIRS} ${LUABRIDG3E_INCLUDE_DIRS})
add_executable(${CMAKE_PROJECT_NAME} ${SRC} )
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE LuaLib
                                           PUBLIC renderer CDT
                                            ${LUABRIDG3E_LIBRARIES}
                                            ${BOOST_LIBRARIES}
                                            ${FREETYPE_LIBRARIES}) #flecs::flecs_static

message(${CMAKE_SOURCE_DIR})