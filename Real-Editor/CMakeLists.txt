cmake_minimum_required(VERSION 3.28)
project(Real-Editor)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(REAL_EMSCRIPTEN OFF CACHE BOOL "enable build for the web")

add_subdirectory(../Real-Engine/ ../Real-Engine/build/)
add_executable(${PROJECT_NAME} src/App.cpp src/EditorLayer.cpp
  src/Panels/SceneHierarchy.cpp src/Panels/Inspector.cpp
  src/HelperWindows.cpp src/EditorSerializer.cpp src/Panels/ContentBrowser.cpp
  src/Panels/AssetManagerGui.cpp src/EditorCamera.cpp src/EditorEvents/EditorEventsManager.cpp
  src/Panels/ErrorModal.cpp src/Panels/ProjectPanel.cpp src/ProjectSelectionLayer.cpp src/RecentProjects.cpp
  src/RecentProjectsSerializer.cpp)
add_compile_options(-ldl -o4)
message("idrs" ${REAL_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${REAL_INCLUDE_DIRS} src/)
target_link_libraries(${PROJECT_NAME} Real-Engine)
if(${REAL_IS_EMSCRIPTEN})
  set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html"
    LINK_FLAGS "-sUSE_GLFW=3 -sWASM=0 -sFULL_ES3 --preload-file ../res --bind -DREAL_EMSCRIPTEN --shell-file template.html"

    #ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/"
    #LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    #RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
endif()
