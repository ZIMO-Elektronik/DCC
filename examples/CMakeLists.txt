if(ESP_PLATFORM)
  # ESP32 example gets automatically included by component manager
  # https://docs.espressif.com/projects/idf-component-manager/en/latest/reference/manifest_file.html#examples
elseif(CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
  add_subdirectory(repl)
else()
  add_subdirectory(stm32)
endif()
