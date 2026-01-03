#include "about.hpp"
#include <imgui.h>
#include "state.hpp"
#include "zimo_rgba.hpp"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#  include <SDL_opengles2.h>
#else
#  include <SDL_opengl.h>
#endif

// About window
void about() {
  static GLuint zimo_texture{};
  if (!zimo_texture) {
    glGenTextures(1, &zimo_texture);
    glBindTexture(GL_TEXTURE_2D, zimo_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 320,
                 226,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 data(zimo_rgba));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  if (!state.windows.show_about) return;

  if (ImGui::Begin("About DCCypher",
                   &state.windows.show_about,
                   ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Image(static_cast<ImTextureID>(zimo_texture),
                 ImVec2{320.0f, 226.0f});
    ImGui::Separator();
    ImGui::Text("@Version %s", DCC_CYPHER_VERSION);
    ImGui::Text("@GitHub");
    ImGui::SameLine();
    ImGui::TextLinkOpenURL("https://github.com/ZIMO-Elektronik/DCC",
                           "https://github.com/ZIMO-Elektronik/DCC");
  }
  ImGui::End();
}
