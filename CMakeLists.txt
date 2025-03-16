cmake_minimum_required(VERSION 3.25.1)

project(Platformer3D LANGUAGES C CXX)

set(EXTERNAL_SOURCE_DIR $ENV{External})

set(SRC_FILES
	src/Camera.cpp
	src/main.cpp
)

set(HDR_FILES
	src/Camera.h
	src/Defines.h
)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${HDR_FILES})
add_subdirectory(${EXTERNAL_SOURCE_DIR}/cglm cglm)

target_link_libraries(${PROJECT_NAME} PRIVATE
	opengl32 
	glu32 
	gdi32
	cglm
)

target_include_directories(${PROJECT_NAME} PRIVATE

)

target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${EXTERNAL_SOURCE_DIR}/stb
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

add_compile_definitions("$<$<CONFIG:Debug>:DEBUG>")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:${PROJECT_NAME}>/res
)