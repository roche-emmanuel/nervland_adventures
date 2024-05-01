set(TARGET_NAME "test_sdl_wasm")

include_directories(${SRC_DIR}/nvCore/src)

include_directories(${SDL_DIR}/include)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_definitions(-DNV_LIB_STATIC)
set(ADDITIONAL_LIBS nvCore_lite pthread)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

file(GLOB_RECURSE SOURCE_FILES "main.cpp")

set(CMAKE_EXECUTABLE_SUFFIX ".js")
# set(CMAKE_EXECUTABLE_SUFFIX ".html")

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} ${ADDITIONAL_LIBS})

set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")

set(assetFiles "")
set(linkFlags
    "--no-heap-copy --preload-file \"${PROJECT_SOURCE_DIR}/tests/test_sdl_wasm/assets/sdl.bmp@/sdl.bmp\""
)
setup_emscripten_sdl_target(${TARGET_NAME} assetFiles "" linkFlags)
