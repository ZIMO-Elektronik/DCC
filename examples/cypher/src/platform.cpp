#if defined(__EMSCRIPTEN__)
#  include "platform.hpp"
#  include <emscripten.h>
#  include <imgui.h>
#  include <string>

namespace {

std::string clipboard{};

} // namespace

void platform() {
  auto& platform_io{ImGui::GetPlatformIO()};
  platform_io.Platform_GetClipboardTextFn = [](ImGuiContext*) {
    return clipboard.c_str();
  };
  platform_io.Platform_SetClipboardTextFn = [](ImGuiContext*,
                                               char const* text) {
    clipboard = text ? text : "";
    // clang-format off
    EM_ASM({
      const txt = UTF8ToString($0);
      if (navigator.clipboard && navigator.clipboard.writeText) {
        navigator.clipboard.writeText(txt).catch(e => console.warn('Clipboard write failed:', e));
      } else {
        console.warn('Clipboard API unavailable');
      }
    }, text);
    // clang-format on 
  };

}
#endif