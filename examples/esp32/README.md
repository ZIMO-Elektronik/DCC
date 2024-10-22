| Supported Targets | ESP32 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

# DCC RMT Example
This example shows how to set up a DCC RMT encoder to send packets using the RMT peripheral. After configuring the hardware, an idle packet is sent in an endless loop.

## How to Use Example
### Hardware Required
* A development board with any supported Espressif SOC chip (see `Supported Targets` table above)
* A USB cable for Power supply and programming

The GPIO number used in this example can be changed according to your board, by the macro `RMT_GPIO_NUM` defined in the [source file](main/app_main.cpp).

### Build and Flash
Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.