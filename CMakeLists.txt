cmake_minimum_required(VERSION 3.16)

project(Platformer3D LANGUAGES C CXX)

file(GLOB SOURCE_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

target_link_libraries(${PROJECT_NAME} opengl32 glu32 gdi32)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

add_compile_definitions("$<$<CONFIG:Debug>:DEBUG>")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/textures $<TARGET_FILE_DIR:${PROJECT_NAME}>/textures
	)