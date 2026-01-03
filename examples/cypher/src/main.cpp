#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <implot.h>
#include <cstdio>
#include "about.hpp"
#include "config.hpp"
#include "datagram_builder.hpp"
#include "datagrams.hpp"
#include "layout.hpp"
#include "menu.hpp"
#include "packet_builder.hpp"
#include "packets.hpp"
#include "platform.hpp"
#include "query.hpp"
#include "style.hpp"
#include "utility.hpp"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#  include <SDL_opengles2.h>
#else
#  include <SDL_opengl.h>
#endif

// This is all taken from the ImGui example, I have no idea what this does and
// quite frankly I don't care.
#if defined(__EMSCRIPTEN__)
#  include <examples/libs/emscripten/emscripten_mainloop_stub.h>
#endif

int main(int, char*[]) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER)) {
    printf("Error: %s\n", SDL_GetError());
    return 1;
  }

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100 (WebGL 1.0)
  constexpr auto glsl_version{"#version 100"};
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
  // GL 3.0 + GLSL 130
  constexpr auto glsl_version{"#version 130"};
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

  // From 2.0.18: Enable native IME.
#if defined(SDL_HINT_IME_SHOW_UI)
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

  // Create window with graphics context
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  auto window_flags{static_cast<SDL_WindowFlags>(
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI)};
  auto window{SDL_CreateWindow("DCCypher",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               1280,
                               800,
                               window_flags)};
  if (window == nullptr) {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    return 1;
  }

  auto gl_context{SDL_GL_CreateContext(window)};
  if (gl_context == nullptr) {
    printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
    return 1;
  }

  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  auto& io{ImGui::GetIO()};
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.IniFilename = nullptr; // Disable .ini

  // Setup ImGui/ImPlot style
  style();

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // URL query
  query();

  // Main loop
  bool done{false};
#if defined(__EMSCRIPTEN__)
  EMSCRIPTEN_MAINLOOP_BEGIN
#else
  while (!done)
#endif
  {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT) done = true;
      if (event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(window))
        done = true;
    }
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
      SDL_Delay(10);
      continue;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Setup initial layout
    layout();

    // Windows
    menu();
    config();
    packet_builder();
    datagram_builder();
    packets();
    platform();
    datagrams();
    about();

    // Rendering
    ImGui::Render();
    glViewport(0,
               0,
               static_cast<int>(io.DisplaySize.x),
               static_cast<int>(io.DisplaySize.y));
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }
#if defined(__EMSCRIPTEN__)
  EMSCRIPTEN_MAINLOOP_END;
#endif

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  ImPlot::DestroyContext();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
