#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <implot.h>
#include <cstdio>
#include <dcc/dcc.hpp>
#include "about.hpp"
#include "config.hpp"
#include "datagram_builder.hpp"
#include "datagrams.hpp"
#include "layout.hpp"
#include "menu.hpp"
#include "packet_builder.hpp"
#include "packets.hpp"
#include "platform.hpp"
#include "state.hpp"
#include "style.hpp"
#include "utility.hpp"

//
#if defined(IMGUI_IMPL_OPENGL_ES2)
#  include <SDL_opengles2.h>
#else
#  include <SDL_opengl.h>
#endif

// This example can also compile and run with Emscripten! See
// 'Makefile.emscripten' for details.
#if defined(__EMSCRIPTEN__)
#  include <examples/libs/emscripten/emscripten_mainloop_stub.h>
#endif

//
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

  // Setup platform IO
  platform();

  // Our state
  State state{.windows = {.show_config = true,
                          .show_packet_builder = true,
                          .show_datagram_builder = true,
                          .show_packets = true,
                          .show_datagrams = true,
                          .show_about = false}};

  // Decoder Control
  state.packets.push_back(
    {.bytes = dcc::make_reset_packet(random_basic_loco())});
  state.packets.push_back(
    {.bytes = dcc::make_hard_reset_packet(random_extended_loco())});
  state.packets.push_back(
    {.bytes = dcc::make_factory_test_packet(random_basic_loco(), true)});
  state.packets.push_back({.bytes = dcc::make_set_advanced_addressing_packet(
                             random_extended_loco(), true)});
  state.packets.push_back(
    {.bytes = dcc::make_ack_request_packet(random_basic_loco())});

  // Consist Control
  state.packets.push_back({.bytes = dcc::make_set_consist_address_packet(
                             random_extended_loco(), 1u << 7u | 42u)});

  // Advanced operations
  state.packets.push_back(
    {.bytes = dcc::make_speed_direction_and_functions_packet(
       random_basic_loco(), 1u << 7u | 0u, 0b0000'0001u)});
  state.packets.push_back(
    {.bytes = dcc::make_speed_direction_and_functions_packet(
       random_extended_loco(), 0u << 7u | 1u, 0b0000'0001u, 0b0000'0011u)});
  state.packets.push_back(
    {.bytes =
       dcc::make_speed_direction_and_functions_packet(random_basic_loco(),
                                                      1u << 7u | 100u,
                                                      0b0000'0001u,
                                                      0b0000'0011u,
                                                      0b0000'0111u)});
  state.packets.push_back({.bytes = dcc::make_analog_function_group_packet(
                             random_extended_loco(),
                             random_interval<uint8_t>(0u, 255u),
                             random_interval<uint8_t>(0u, 255u))});

  // Speed & direction
  state.packets.push_back({.bytes = dcc::make_speed_and_direction_packet(
                             random_extended_loco(), 1u << 5u | 23u)});

  // Function group
  state.packets.push_back(
    {.bytes = dcc::make_f0_f4_packet(random_extended_loco(), 0b0001'0000u)});
  state.packets.push_back(
    {.bytes = dcc::make_f9_f12_packet(random_basic_loco(), 0b0000'0110u)});
  state.packets.push_back(
    {.bytes = dcc::make_f5_f8_packet(random_extended_loco(), 0b0000'1001u)});

  // Feature expansion
  state.packets.push_back(
    {.bytes = dcc::make_f29_f36_packet(random_extended_loco(), 0b1000'0000u)});
  state.packets.push_back(
    {.bytes = dcc::make_f37_f44_packet(random_basic_loco(), 0b0100'0000u)});
  state.packets.push_back(
    {.bytes = dcc::make_f45_f52_packet(random_extended_loco(), 0b0010'0000u)});
  state.packets.push_back(
    {.bytes = dcc::make_f53_f60_packet(random_basic_loco(), 0b0001'0000u)});
  state.packets.push_back(
    {.bytes = dcc::make_f61_f68_packet(random_extended_loco(), 0b0000'1000u)});
  state.packets.push_back(
    {.bytes = dcc::make_f13_f20_packet(random_basic_loco(), 0b0000'0100u)});
  state.packets.push_back(
    {.bytes = dcc::make_f21_f28_packet(random_extended_loco(), 0b0000'0010u)});

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

    //
    menu(state);
    config(state);
    packet_builder(state);
    datagram_builder(state);
    packets(state);
    datagrams(state);
    about(state);

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
