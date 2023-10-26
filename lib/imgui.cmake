add_library(
	imgui
	STATIC
	${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_demo.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_draw.cpp
	${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_widgets.cpp
  ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_tables.cpp
  ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_glfw.cpp
  ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories( imgui PUBLIC glfw/include ${CMAKE_CURRENT_LIST_DIR}/imgui ${CMAKE_CURRENT_LIST_DIR}/imgui/backends )
set_target_properties( imgui PROPERTIES FOLDER "imgui" )
