cmake_minimum_required(VERSION 3.28)
project(Real-Engine LANGUAGES CXX)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only")
#set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "Build the Box2D unit tests")
#set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "Build the Box2D testbed")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

option(REAL_ENABLE_ASSERT "Enables Assertion" ON)
option(REAL_ENABLE_VERIFY "Enables Verify" ON)
option(REAL_ENABLE_INSTRUMENTATION "Enables Profiling" ON)
option(REAL_IS_SHARED "builds as shared lib" OFF)
option(REAL_EMSCRIPTEN "builds for the web" OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-Wall -g -O0 -fPIC)
  set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-fPIC -Wall -g -O0)
  set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
endif()

set(REAL_SRC src/Real-Engine/Core/App.cpp src/Real-Engine/Core/Log.cpp
    src/Platforms/Linux/LinuxWindow.cpp src/Real-Engine/Core/Layers/Layer.cpp
    src/Real-Engine/Core/Layers/LayerStack.cpp src/Real-Engine/Core/Layers/GuiLayer.cpp
    src/Platforms/Linux/LinuxInput.cpp
    src/Platforms/OpenGl/OpenGlContext.cpp src/Platforms/OpenGl/OpenGlVertexBuffer.cpp
    src/Platforms/OpenGl/OpenGlIndexBuffer.cpp src/Real-Engine/Render/API/VertexBuffer.cpp
    src/Real-Engine/Render/API/IndexBuffer.cpp
    src/Real-Engine/Render/API/RendererAPI.cpp src/Real-Engine/Render/API/VertexArray.cpp
    src/Platforms/OpenGl/OpenGlVertexArray.cpp src/Real-Engine/Render/API/Shader.cpp
    src/Platforms/OpenGl/OpenGlShader.cpp src/Real-Engine/Core/Utils/FileLoader.cpp
    src/Real-Engine/Render/API/Renderer.cpp src/Real-Engine/Render/API/RenderCommands.cpp
    src/Platforms/OpenGl/OpenGlRendererAPI.cpp src/Real-Engine/Render/OrthographicCamera.cpp
    src/Platforms/Windows/WindowsInput.cpp src/Platforms/Windows/WindowsWindow.cpp
    src/Platforms/OpenGl/OpenGlTexture.cpp src/Real-Engine/Render/API/Texture.cpp
    src/Real-Engine/Core/Utils/ImageLoader.cpp src/Platforms/OpenGl/OpenGlCore.cpp
    src/Real-Engine/Render/ShaderManager.cpp src/Real-Engine/Render/OrthographicCameraController.cpp
    src/Real-Engine/Render/Renderer2D/Renderer2D.cpp src/Platforms/OpenGl/OpenGlRendererCapabilities.cpp
    src/Real-Engine/Core/Instrumentor.cpp src/Real-Engine/Render/API/RendererCapabilities.cpp
    src/Real-Engine/Render/API/FrameBuffer.cpp src/Platforms/OpenGl/OpenGlFrameBuffer.cpp
    src/Real-Engine/Scene/Scene.cpp src/Real-Engine/Scene/Entity.cpp src/Real-Engine/Scene/SceneCamera.cpp
    src/Real-Engine/Scene/SceneSerializer.cpp src/Platforms/Linux/LinuxFileDialogs.cpp
    src/Real-Engine/Core/AssetManager.cpp src/Real-Engine/Core/Math/math.cpp src/Real-Engine/Core/UUID.cpp
    src/Platforms/Windows/WindowsFileDialogs.cpp src/Platforms/OpenGl/OpenGlUtils.cpp
    src/Real-Engine/Project/Project.cpp src/Real-Engine/Project/ProjectSerializer.cpp
    src/Real-Engine/Scripting/Lua.cpp src/Real-Engine/Core/AssetManagerSerializer.cpp
    include/Real-Engine/Real-Engine.h src/Real-Engine/Core/Export.cpp
  )
#set(REAL_SRC_H src/Real-Engine/App.h src/Real-Engine/Log.h src/Platforms/Windows/WindowsWindow.h src/Real-Engine/Layers/Layer.h
#    src/Real-Engine/Layers/LayerStack.h src/Real-Engine/Layers/GuiLayer.h src/Platforms/Windows/WindowsInput.h
#  src/Real-Engine/Input.h src/Platforms/OpenGl/OpenGlContext.h src/Platforms/OpenGl/OpenGlVertexBuffer.h
#  src/Platforms/OpenGl/OpenGlIndexBuffer.h src/Render/API/VertexBuffer.h src/Render/API/IndexBuffer.h
#  src/Render/API/RendererAPI.h src/Render/API/VertexArray.h
#  src/Platforms/OpenGl/OpenGlVertexArray.h src/Render/API/Shader.h
#  src/Platforms/OpenGl/OpenGlShader.h src/Real-Engine/Utils/FileLoader.h
#  src/Render/API/Renderer.h src/Render/API/RenderCommands.h
#  src/Platforms/OpenGl/OpenGlRendererAPI.h
#  src/Real-Engine/Core.h src/Real-Engine/Window.h src/Platforms/Linux/LinuxInput.h Platforms/Linux/LinuxWindow.h  src/Render/OrthographicCamera.h
#)
set(REAL_PUBLIC_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/src/
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/entt/single_include/
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/yaml-cpp/include/
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/pugixml/src
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/Imguizmo
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/sol2/include
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/Lua/src
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/IconFontHeaders
  )
 set(REAL_INCLUDE_DIRS ${REAL_PUBLIC_INCLUDE_DIRS} PARENT_SCOPE)

 set(REAL_PRIVATE_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/include
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui/backends
  ${OPENGL_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/Box2D/include
  ${CMAKE_CURRENT_SOURCE_DIR}/vendor/Box2D/src
  )
if(NOT REAL_EMSCRIPTEN)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glad)
endif()
add_subdirectory(vendor/stb_image)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/pugixml)
add_subdirectory(vendor/entt)
add_subdirectory(vendor/yaml-cpp)
add_subdirectory(vendor/Box2D)
add_subdirectory(vendor/Imguizmo)
add_subdirectory(vendor/sol2)
add_subdirectory(vendor/Lua)
if(NOT REAL_EMSCRIPTEN)
  set(REAL_LIBS glm glfw imgui glad stb pugixml yaml-cpp box2d imguizmo Lua sol2)
else()
set(REAL_ADDITIONAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/../../emsdk/upstream/emscripten/cache/sysroot/include)
set(REAL_LIBS glm imgui stb pugixml yaml-cpp box2d imguizmo Lua sol2)
endif()

if(REAL_IS_SHARED)
  add_library(${PROJECT_NAME} SHARED ${REAL_SRC} ${REAL_SRC_H})
  target_compile_definitions(${PROJECT_NAME} PRIVATE REAL_BUILD_SHARED REAL_IS_SHARED)
  set(REAL_LINK_LIBS ${REAL_LIBS} PARENT_SCOPE)
else()
  set(REAL_LINK_LIBS  PARENT_SCOPE)
  add_library(${PROJECT_NAME} STATIC ${REAL_SRC} ${REAL_SRC_H})
endif(REAL_IS_SHARED)

if(REAL_ENABLE_ASSERT)
  target_compile_definitions(${PROJECT_NAME} PUBLIC REAL_ENABLE_ASSERT)
endif(REAL_ENABLE_ASSERT)

if(REAL_ENABLE_VERIFY)
  target_compile_definitions(${PROJECT_NAME} PUBLIC REAL_ENABLE_VERIFY)
endif(REAL_ENABLE_VERIFY)

if(REAL_EMSCRIPTEN)
  target_compile_definitions(${PROJECT_NAME} PUBLIC REAL_EMSCRIPTEN)
  set(REAL_IS_EMSCRIPTEN ${REAL_EMSCRIPTEN} PARENT_SCOPE)
endif()

if(REAL_ENABLE_INSTRUMENTATION)
  target_compile_definitions(${PROJECT_NAME} PUBLIC REAL_ENABLE_INSTRUMENTATION)
  set(REAL_INSTRUMENTATION ${REAL_ENABLE_INSTRUMENTATION} PARENT_SCOPE)
endif()

target_precompile_headers(${PROJECT_NAME} PUBLIC src/pch.h)

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${REAL_LIBS})

target_include_directories(${PROJECT_NAME} PRIVATE ${REAL_PRIVATE_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME} PUBLIC ${REAL_PUBLIC_INCLUDE_DIRS} ${REAL_ADDITIONAL_INCLUDE})
