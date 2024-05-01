#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <webgpu/webgpu_cpp.h>
// #include <emscripten.h>
#include <iostream>

// #include <core_common.h>

#define DEBUG_MSG(msg) std::cout << msg << std::endl;

// using namespace nv;

// References:
// cf. https://developer.chrome.com/blog/webgpu-cross-platform?hl=fr
// or https://developer.chrome.com/docs/web-platform/webgpu/build-app

wgpu::Instance instance;
wgpu::SwapChain swapChain;
wgpu::Device device;
wgpu::RenderPipeline pipeline;

const uint32_t kWidth = 1280;
const uint32_t kHeight = 720;

const char shaderCode[] = R"(
    @vertex fn vertexMain(@builtin(vertex_index) i : u32) ->
      @builtin(position) vec4f {
        const pos = array(vec2f(0, 1), vec2f(-1, -1), vec2f(1, -1));
        return vec4f(pos[i], 0, 1);
    }
    @fragment fn fragmentMain() -> @location(0) vec4f {
        return vec4f(1, 0, 0, 1);
    }
)";

void printDeviceError(WGPUErrorType errorType, const char* message,
                      void* /*unused*/) {
    const char* errorTypeName = "";
    switch (errorType) {
    case WGPUErrorType_Validation:
        errorTypeName = "Validation";
        break;
    case WGPUErrorType_OutOfMemory:
        errorTypeName = "Out of memory";
        break;
    case WGPUErrorType_Unknown:
        errorTypeName = "Unknown";
        break;
    case WGPUErrorType_DeviceLost:
        errorTypeName = "Device lost";
        break;
    default:
        DEBUG_MSG("ERROR: Should not be reached.");
        exit(1);
        return;
    }

    DEBUG_MSG("Dawn: " << errorTypeName << " error: " << message);
}

void deviceLostCallback(WGPUDeviceLostReason reason, const char* message,
                        void* /*unused*/) {
    DEBUG_MSG("Dawn Error: device lost: "
              << message << ", reason: "
              << (reason == WGPUDeviceLostReason_Destroyed ? "destroyed"
                                                           : "unknown"));
}

void deviceLogCallback(WGPULoggingType type, const char* message,
                       void* /*unused*/) {
    switch (type) {
    case WGPULoggingType_Verbose:
        DEBUG_MSG("DEBUG: Dawn: " << message);
        break;
    case WGPULoggingType_Info:
        DEBUG_MSG("INFO Dawn: " << message);
        break;
    case WGPULoggingType_Warning:
        DEBUG_MSG("WARN Dawn: " << message);
        break;
    case WGPULoggingType_Error:
        DEBUG_MSG("ERROR Dawn: " << message);
        break;
    default:
        DEBUG_MSG("INFO ???: " << message);
        break;
    }
}

static void
wgpu_compilation_info_callback(WGPUCompilationInfoRequestStatus status,
                               WGPUCompilationInfo const* compilationInfo,
                               void* /*userdata*/) {
    if (status == WGPUCompilationInfoRequestStatus_Error) {
        for (uint32_t m = 0; m < compilationInfo->messageCount; ++m) {
            WGPUCompilationMessage message = compilationInfo->messages[m];
            DEBUG_MSG("Shader compile error: lineNum: "
                      << message.lineNum << ", linePos: " << message.linePos
                      << ": " << message.message);
        }
    }
}

void GetDevice(void (*callback)(wgpu::Device)) {
    instance.RequestAdapter(
        nullptr,
        [](WGPURequestAdapterStatus status, WGPUAdapter cAdapter,
           const char* message, void* userdata) {
            if (status != WGPURequestAdapterStatus_Success) {
                DEBUG_MSG("ERROR: Cannot retrieve WebGPU adapter!");
                exit(0);
            }
            wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);

            // **Note**: in the RequestDevice call below the first argument is
            // the device
            //  descriptor: this is where we should specify the required
            //  features later.
            adapter.RequestDevice(
                nullptr,
                [](WGPURequestDeviceStatus status, WGPUDevice cDevice,
                   const char* message, void* userdata) {
                    wgpu::Device device = wgpu::Device::Acquire(cDevice);
                    // Now call the callback:
                    reinterpret_cast<void (*)(wgpu::Device)>(userdata)(device);
                },
                userdata);
        },
        reinterpret_cast<void*>(callback));
}

void SetupSwapChain(wgpu::Surface surface) {
    wgpu::SwapChainDescriptor scDesc{.usage =
                                         wgpu::TextureUsage::RenderAttachment,
                                     .format = wgpu::TextureFormat::BGRA8Unorm,
                                     .width = kWidth,
                                     .height = kHeight,
                                     .presentMode = wgpu::PresentMode::Fifo};
    swapChain = device.CreateSwapChain(surface, &scDesc);
}

void CreateRenderPipeline() {
    wgpu::ShaderModuleWGSLDescriptor wgslDesc{};
    wgslDesc.code = shaderCode;

    wgpu::ShaderModuleDescriptor shaderModuleDescriptor{.nextInChain =
                                                            &wgslDesc};
    DEBUG_MSG("Creating shader module.");
    wgpu::ShaderModule shaderModule =
        device.CreateShaderModule(&shaderModuleDescriptor);
    DEBUG_MSG("Done creating shader module.");
    if (shaderModule == nullptr) {
        DEBUG_MSG("ERROR: Invalid shader module!");
        exit(0);
    }
    shaderModule.GetCompilationInfo(wgpu_compilation_info_callback, nullptr);
    instance.ProcessEvents();

    wgpu::ColorTargetState colorTargetState{
        .format = wgpu::TextureFormat::BGRA8Unorm};

    wgpu::FragmentState fragmentState{.module = shaderModule,
                                      .entryPoint = "fragmentMain",
                                      .targetCount = 1,
                                      .targets = &colorTargetState};

    wgpu::RenderPipelineDescriptor descriptor{
        .vertex = {.module = shaderModule, .entryPoint = "vertexMain"},
        .fragment = &fragmentState};

    DEBUG_MSG("Creating pipeline");
    pipeline = device.CreateRenderPipeline(&descriptor);
    DEBUG_MSG("Done creating pipeline");
}

void InitGraphics(wgpu::Surface surface) {
    SetupSwapChain(surface);
    DEBUG_MSG("Creating RenderPipeline...");
    CreateRenderPipeline();
    DEBUG_MSG("Graphics initialized.");
}

void Render() {
    wgpu::RenderPassColorAttachment attachment{
        .view = swapChain.GetCurrentTextureView(),
        .loadOp = wgpu::LoadOp::Clear,
        .storeOp = wgpu::StoreOp::Store};

    wgpu::RenderPassDescriptor renderpass{.colorAttachmentCount = 1,
                                          .colorAttachments = &attachment};

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderpass);
    pass.SetPipeline(pipeline);
    pass.Draw(3);
    pass.End();
    wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);
}

auto getSurfaceDescriptor(SDL_Window* window)
    -> std::unique_ptr<wgpu::ChainedStruct> {

    SDL_SysWMinfo info;

    SDL_VERSION(&info.version);

    if (SDL_GetWindowWMInfo(window, &info) == 0) {
        DEBUG_MSG("Error: Cannot retrieve window info: " << SDL_GetError());
        exit(0);
    }

    std::unique_ptr<wgpu::ChainedStruct> res;

#if _WIN32
    if (info.subsystem != SDL_SYSWM_WINDOWS) {
        DEBUG_MSG("Error: invalid SDL subsystem on windows!");
        exit(0);
    }

    std::unique_ptr<wgpu::SurfaceDescriptorFromWindowsHWND> desc =
        std::make_unique<wgpu::SurfaceDescriptorFromWindowsHWND>();

    desc->hwnd = (void*)info.info.win.window;
    desc->hinstance = (void*)info.info.win.hinstance;
    res = std::move(desc);
#endif

#if __linux__
    if (info.subsystem != SDL_SYSWM_X11) {
        DEBUG_MSG("Error: invalid SDL subsystem on linux!");
        exit(0);
    }

    std::unique_ptr<wgpu::SurfaceDescriptorFromXlibWindow> desc =
        std::make_unique<wgpu::SurfaceDescriptorFromXlibWindow>();
    desc->display = (void*)info.info.x11.display;
    desc->window = info.info.x11.window;
    res = std::move(desc);
#endif

    return res;
}

auto createSurfaceForWindow(SDL_Window* window) -> wgpu::Surface {
    auto desc = getSurfaceDescriptor(window);
    wgpu::SurfaceDescriptor descriptor;
    descriptor.nextInChain = desc.get();
    return instance.CreateSurface(&descriptor);
}

void run() {
    DEBUG_MSG("Running app...");
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window* window = SDL_CreateWindow("SDL2 with WebGPU", 50, 30, kWidth,
                                          kHeight, SDL_WINDOW_SHOWN);

    // emscripten_set_main_loop_arg(callback, &context, -1, 1);
    // instance.ProcessEvents();

    wgpu::Surface surface = createSurfaceForWindow(window);
    if (surface == nullptr) {
        DEBUG_MSG("Error: Cannot create surface.");
        exit(0);
    }
    InitGraphics(surface);

    // Run loop:
    DEBUG_MSG("Entering main loop...");
    bool running = true;
    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                DEBUG_MSG("Exiting on user request.");
                running = false;
            }
        }

        if (!running) {
            break;
        }

        // Process this frame:
        Render();
        swapChain.Present();
        instance.ProcessEvents();
    }
    DEBUG_MSG("Exiting main loop.");

    SDL_DestroyWindow(window);
    SDL_Quit();
}

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

    // Create the webgpu instance:
    DEBUG_MSG("Creating WebGPU instance...");
    instance = wgpu::CreateInstance();
    // instance = std::make_unique<dawn::native::Instance>();
    DEBUG_MSG("WebGPU instance created.");

    // run();

    DEBUG_MSG("Retrieving device...");
    // Get the device and then start the app :
    GetDevice([](wgpu::Device dev) {
        device = dev;
        device.SetUncapturedErrorCallback(printDeviceError, nullptr);
        device.SetDeviceLostCallback(deviceLostCallback, nullptr);
        device.SetLoggingCallback(deviceLogCallback, nullptr);
        run();
    });

    DEBUG_MSG("Exiting app.");
    return 0;
}
