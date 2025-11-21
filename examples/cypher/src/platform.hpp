#pragma once

#if defined(__EMSCRIPTEN__)
void platform();
#else
inline void platform() {}
#endif
