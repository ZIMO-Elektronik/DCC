#include "platform.hpp"
#include <imgui.h>
#include <string>
#include "query.hpp"

#if defined(__EMSCRIPTEN__)
#  include <emscripten.h>
#else
#  define EMSCRIPTEN_KEEPALIVE
#endif

namespace {

std::string clipboard{};

} // namespace

extern "C" {
EMSCRIPTEN_KEEPALIVE
void from_clipboard(char const* text) {
  clipboard = text ? text : "";
  from_query(clipboard.c_str());
}
EMSCRIPTEN_KEEPALIVE
char const* to_clipboard() {
  clipboard = to_query();
  return clipboard.c_str();
}
}

#if defined(__EMSCRIPTEN__)
// clang-format off
EM_JS(void, installClipboardKeyListener, (), {
    document.addEventListener('keydown', e => {
        // Ctrl+C
        if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'c') {
          e.preventDefault();
          const textPtr = _to_clipboard();
          const text = UTF8ToString(textPtr);
          navigator.clipboard.writeText(text)
            .then(() => console.log("Copied to clipboard:", text))
            .catch(err => console.error("Clipboard write failed:", err));
        }
        // Ctrl+V
        if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'v') {
            e.preventDefault();
            navigator.clipboard.readText().then(text => {
              const len = lengthBytesUTF8(text) + 1;
              const ptr = _malloc(len);
              stringToUTF8(text, ptr, len);
              _from_clipboard(ptr);
              _free(ptr);
            }).catch(e => console.error("Clipboard read failed:", e));
        }
    });
});
// clang-format on
#endif

void platform() {
#if defined(__EMSCRIPTEN__)
  static bool initialized{};
  if (initialized) return;
  initialized = true;
  installClipboardKeyListener();
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
#else
  // Ctrl+C
  if (auto& io{ImGui::GetIO()}; io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C))
    ImGui::SetClipboardText(to_clipboard());
  // Ctrl+V
  else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_V))
    from_clipboard(ImGui::GetClipboardText());
#endif
}
