#include <SDL2/SDL.h>

// #include <emscripten.h>
#include <iostream>

// #include <core_common.h>

#define DEBUG_MSG(msg) std::cout << msg << std::endl;

// using namespace nv;

struct Context {
    std::string title;
    int width, height;
    SDL_Event event;
};

#if 0

void callback(void* arg) {
    Context* context = static_cast<Context*>(arg);
    while (SDL_PollEvent(&context->event)) {
        if (context->event.type == SDL_QUIT) {
            exit(0);
        } else if (context->event.type == SDL_MOUSEBUTTONDOWN) {
            context->rect2.x -= 20;
        }
    }

    SDL_RenderClear(context->renderer);
    SDL_SetRenderDrawColor(context->renderer, 255, 255, 255, 255);
    // SDL_RenderDrawRect(renderer, &rect2);
    SDL_RenderFillRect(context->renderer, &context->rect2);
    SDL_SetRenderDrawColor(context->renderer, 9, 20, 33, 255);
    SDL_RenderCopy(context->renderer, context->logo, NULL, &context->rect);
    SDL_RenderPresent(context->renderer);
}
#endif

int main(int argc, char** argv) {

// #ifdef __EMSCRIPTEN__
#if 0
    // EM_ASM is a macro to call in-line JavaScript code.
    EM_ASM(
        // Make a directory other than '/'
        FS.mkdir('/testapp');
        // Then mount with IDBFS type
        FS.mount(IDBFS, {}, '/testapp');

        // Then sync
        FS.syncfs(
            true, function(err) {
                console.log("Done with initial FS.syncFS()!");
                assert(err == undefined);
                console.log("Checked error.");
            }););
#endif

    Context context;
    SDL_Init(SDL_INIT_EVERYTHING);

    context.title = "SDL2 with WebGPU";
    context.width = 1280;
    context.height = 720;

    SDL_Window* window =
        SDL_CreateWindow(context.title.c_str(), 50, 30, context.width,
                         context.height, SDL_WINDOW_SHOWN);

    // emscripten_set_main_loop_arg(callback, &context, -1, 1);

    // Run loop:
    DEBUG_MSG("Entering main loop...");
    bool running = true;
    while (running) {
        while (SDL_PollEvent(&context.event)) {
            if (context.event.type == SDL_QUIT) {
                DEBUG_MSG("Exiting on user request.");
                running = false;
            }
        }
    }
    DEBUG_MSG("Exiting main loop.");

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
