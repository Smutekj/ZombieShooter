include(FetchContent)
    
### LUA (Needs special treatment for reasons I don't understand)
if( ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    add_custom_target(
        BuildLua
        COMMAND emmake make linux 
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/external/lua/src/
    )
else()
    add_custom_target(
        BuildLua
        COMMAND make linux
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

### LUABridge 
FetchContent_Declare(
    LuaBridge
    GIT_REPOSITORY https://github.com/vinniefalco/LuaBridge
    GIT_TAG master 
)
FetchContent_MakeAvailable(LuaBridge)
            
### RENDERER
FetchContent_Declare(
  renderer  
  GIT_REPOSITORY https://github.com/Smutekj/simple-emscripten-renderer
  GIT_TAG master
)
FetchContent_MakeAvailable(renderer)

### CONSTRAINED DELAUNAY TRIANGULATION
FetchContent_Declare(
    CDT
    GIT_REPOSITORY https://github.com/Smutekj/CDT
    GIT_TAG main 
)
FetchContent_MakeAvailable(CDT)

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