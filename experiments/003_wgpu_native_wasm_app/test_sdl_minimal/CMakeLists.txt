set(TARGET_NAME "test_sdl_minimal")
set(TARGET_DIR "./")

# include_directories(${SRC_DIR}/nvCore/src)

include_directories(${SDL_DIR}/include)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# add_definitions(-DNV_LIB_STATIC)

file(GLOB_RECURSE SOURCE_FILES "main.cpp")
set(ADDITIONAL_LIBS ${SDL_LIBS} ${SDLMAIN_LIBS})

if(IS_EMSCRIPTEN)
  set(ADDITIONAL_LIBS ${ADDITIONAL_LIBS} pthread)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

  set(CMAKE_EXECUTABLE_SUFFIX ".js")
  # set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} ${ADDITIONAL_LIBS})

set_target_properties(${TARGET_NAME} PROPERTIES PREFIX "")

if(IS_EMSCRIPTEN)
  set(assetFiles "")
  set(linkFlags "")
  # set(linkFlags "--no-heap-copy --preload-file
  # \"${PROJECT_SOURCE_DIR}/tests/test_sdl_wasm/assets/sdl.bmp@/sdl.bmp\""
  # )
  setup_emscripten_sdl_target(${TARGET_NAME} assetFiles "" linkFlags)
else()
  install(
    TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION ${TARGET_DIR}
    LIBRARY DESTINATION ${TARGET_DIR})
endif()
