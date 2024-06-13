# DCC

[![build](https://github.com/ZIMO-Elektronik/DCC/actions/workflows/build.yml/badge.svg)](https://github.com/ZIMO-Elektronik/DCC/actions/workflows/build.yml) [![tests](https://github.com/ZIMO-Elektronik/DCC/actions/workflows/tests.yml/badge.svg)](https://github.com/ZIMO-Elektronik/DCC/actions/workflows/tests.yml)

<img src="https://github.com/ZIMO-Elektronik/DCC/raw/master/data/images/logo.gif" align="right"/>

DCC is an acronym for [Digital Command Control](https://en.wikipedia.org/wiki/Digital_Command_Control), a standardized protocol for controlling digital model railways. This C++ library of the same name contains platform-independent code to either decode (decoder) or generate (command station) a DCC signal on the track. For both cases, a typical microcontroller timer with microsecond precision is sufficient for implementing a receiver or transmitter class. Also included, but not platform-independent, is an encoder for the [ESP32 RMT](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/rmt.html) peripherals.

The implementation provided here is used in the following products:
- [ZIMO MN decoders](https://www.zimo.at/web2010/products/mn-nicht-sound-decoder_EN.htm)
- [ZIMO small-](https://www.zimo.at/web2010/products/ms-sound-decoder_EN.htm) and [large-scale MS decoders](https://www.zimo.at/web2010/products/ms-sound-decoder-grossbahn_EN.htm)

<details>
  <summary>Table of contents</summary>
  <ol>
    <li><a href="#protocol">Protocol</a></li>
    <li><a href="#features">Features</a></li>
    <li><a href="#getting-started">Getting started</a></li>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
        <li><a href="#build">Build</a></li>
      </ul>
    <li><a href="#usage">Usage</a></li>
      <ul>
        <li><a href="#receiver-1">Receiver</a></li>
        <li><a href="#transmitter-1">Transmitter</a></li>
        <li><a href="#esp32-rmt-encoder-1">ESP32 RMT encoder</a></li>
      </ul>
  </ol>
</details>

## Protocol
The DCC protocol is defined by various standards published by the [National Model Railroad Association (NMRA)](https://www.nmra.org/) and the [RailCommunity](https://www.vhdm.at/). The standards are mostly consistent and we have attempted to match the English and German standards in the table below. However, if you can read German, we recommend that you stick to the RCN standards as they are updated more frequently.

| NMRA (English)                                                                                                                                                                          | RailCommunity (German)                                                                                                   |
| --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
| [S-9.1 Electrical Standards for Digital Command Control](https://www.nmra.org/sites/default/files/standards/sandrp/pdf/s-9.1_electrical_standards_for_digital_command_control_2021.pdf) | [RCN-210 DCC - Protokoll Bit - Übertragung](https://normen.railcommunity.de/RCN-210.pdf)                                 |
| [S-9.2 Communications Standards For Digital Command Control, All Scales](https://www.nmra.org/sites/default/files/s-92-2004-07.pdf)                                                     | [RCN-211 DCC - Protokoll Paketstruktur, Adressbereiche und globale Befehle](https://normen.railcommunity.de/RCN-211.pdf) |
| [S-9.2.1 DCC Extended Packet Formats](https://www.nmra.org/sites/default/files/standards/sandrp/pdf/s-9.2.1_dcc_extended_packet_formats.pdf)                                            | [RCN-212 DCC - Protokoll Betriebsbefehle für Fahrzeugdecoder](https://normen.railcommunity.de/RCN-212.pdf)               |
| [S-9.2.1 DCC Extended Packet Formats](https://www.nmra.org/sites/default/files/standards/sandrp/pdf/s-9.2.1_dcc_extended_packet_formats.pdf)                                            | [RCN-213 DCC - Protokoll Betriebsbefehle für Zubehördecoder](https://normen.railcommunity.de/RCN-213.pdf)                |
| [S-9.2.1 DCC Extended Packet Formats](https://www.nmra.org/sites/default/files/standards/sandrp/pdf/s-9.2.1_dcc_extended_packet_formats.pdf)                                            | [RCN-214 DCC - Protokoll Konfigurationsbefehle](https://normen.railcommunity.de/RCN-214.pdf)                             |
| [S-9.2.3 Service Mode For Digital Command Control, All Scales](https://www.nmra.org/sites/default/files/standards/sandrp/pdf/S-9.2.3_2012_07.pdf)                                       | [RCN-216 DCC - Protokoll Programmierumgebung](https://normen.railcommunity.de/RCN-216.pdf)                               |
| [S-9.3.2 Communications Standard for Digital Command Control Basic Decoder Transmission](https://www.nmra.org/sites/default/files/s-9.3.2_2012_12_10.pdf)                               | [RCN-217 RailCom DCC-Rückmeldeprotokol](https://normen.railcommunity.de/RCN-217.pdf)                                     |
| [S-9.2.1.1 Advanced Extended Packet Formats](https://www.nmra.org/sites/default/files/standards/sandrp/pdf/s-9.2.1.1_advanced_extended_packet_formats.pdf)                              | [RCN-218 DCC - Protokoll DCC-A - Automatische Anmeldung](https://normen.railcommunity.de/RCN-218.pdf)                    |
| [S-9.2.2 Configuration Variables For Digital Command Control, All Scales](https://www.nmra.org/sites/default/files/standards/sandrp/pdf/s-9.2.2_decoder_cvs_2012.07.pdf)                | [RCN-225 DCC - Protokoll Konfigurationsvariablen](https://normen.railcommunity.de/RCN-225.pdf)                           |

## Features
- Platform-independent (apart from the ESP32 RMT encoder)
- Standard-compliant decoding within the bit duration tolerances
- Support for BiDi (RailCom), a bidirectional extension to the DCC protocol

### Receiver
- Configures itself based on its CV values
- Supports user-defined BiDi datagrams
- Supported instructions
  - Multi-function decoders
    - Decoder control
      - Digital decoder reset :negative_squared_cross_mark:
      - Hard reset :negative_squared_cross_mark:
      - Factory test :negative_squared_cross_mark:
      - Set advanced addressing :negative_squared_cross_mark:
      - Decoder acknowledgement request :negative_squared_cross_mark:
    - Consist control
      - Set consist address :ballot_box_with_check:
    - Advanced operations
      - Speed, direction and function :negative_squared_cross_mark:
      - Analog function group :negative_squared_cross_mark:
      - Special operating modes :ballot_box_with_check:
      - 128 speed step control :ballot_box_with_check:
    - Speed and direction
      - Basic speed and direction :ballot_box_with_check:
    - Function groups
      - F0-F4 :ballot_box_with_check:
      - F9-F12 :ballot_box_with_check:
      - F5-F8 :ballot_box_with_check:
    - Feature expansion
      - Binary state control long form :negative_squared_cross_mark:
      - Time and date :negative_squared_cross_mark:
      - System time :negative_squared_cross_mark:
      - Command station properties identifier :negative_squared_cross_mark:
      - F29-F36 :negative_squared_cross_mark:
      - F37-F44 :negative_squared_cross_mark:
      - F45-F52 :negative_squared_cross_mark:
      - F53-F60 :negative_squared_cross_mark:
      - F61-F68 :negative_squared_cross_mark:
      - Binary state control short form :negative_squared_cross_mark:
      - F13-F20 :ballot_box_with_check:
      - F21-F28 :ballot_box_with_check:
    - CV access
      - Long form :ballot_box_with_check:
      - Short form :ballot_box_with_check:
  - Accessory decoders
    - Currently not supported :negative_squared_cross_mark:

### Transmitter
- Configurable preamble, bit durations and BiDi cutout
- Supports user-defined packets and transmission of raw bytes

### ESP32 RMT encoder
- Configurable preamble, bit durations and BiDi cutout
- Only supports transmission of raw bytes

## Getting started
### Prerequisites
- C++23 compatible compiler
- [CMake](https://cmake.org/) ( >= 3.25 )
- Optional
  - for building [ESP32](https://www.espressif.com/en/products/socs/esp32) [RMT](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/rmt.html) encoder example
    - [ESP-IDF](https://github.com/espressif/esp-idf) ( >= 5.0.3 )
  - for building [STM32](https://www.st.com/en/microcontrollers-microprocessors/stm32-32-bit-arm-cortex-mcus.html) example
    - [arm-none-eabi-gcc](https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases) ( >= 12.2.0 )

### Installation
This library is meant to be consumed with CMake,

```cmake
# Either by including it with CPM
cpmaddpackage("gh:ZIMO-Elektronik/DCC@0.34.0")

# or the FetchContent module
FetchContent_Declare(
  DCC
  GIT_REPOSITORY "https://github.com/ZIMO-Elektronik/DCC"
  GIT_TAG v0.34.0)

target_link_libraries(YourTarget PRIVATE DCC::DCC)
```

or, on [ESP32 platforms](https://www.espressif.com/en/products/socs/esp32), with the [IDF Component Manager](https://docs.espressif.com/projects/idf-component-manager/en/latest/) by adding it to a `idf_component.yml` file.
```yaml
dependencies:
  zimo-elektronik/dcc:
    version: "0.34.0"
```

A number of [options](CMakeLists.txt) are provided to configure various sizes such as the receiver deque length or the maximum packet length. When RAM becomes scarce, deque lengths can be reduced. On the other hand, if the processing of the commands is too slow and cannot be done every few milliseconds, it can make sense to lengthen the deques and batch process several commands at once. Otherwise, we recommend sticking with the defaults.
```cmake
set(DCC_RX_DEQUE_SIZE
    8u
    CACHE STRING "" FORCE)
```

### Build
The library itself is header-only, so technically it can't be built. However, if run as top-level CMake project then, depending on the target platform, different examples can be built.

#### Host
On host platforms a [REPL](https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop) example allows a handful of commands to be sent from a simulated command station running in one thread to a simulated decoder running in another.
```sh
cmake -Bbuild
cmake --build build --target DCCRepl
```

Available commands can be listed by using `help`.
```sh
./build/examples/repl/DCCRepl
dcc> help
Commands available:
 - help
        This help message
 - exit
        Quit the session
 - address <Address [0-16383] [default:3]>
        Set address all commands are sent to
 - direction_speed <Direction [1 forward, 0 backward]> <Speed [0-127]>
        Set direction and speed
 - f4-f0 <State [0b00000-0b11111]>
        Functions F4-F0
 - f8-f5 <State [0b00000-0b11111]>
        Functions F8-F5
 - read_cv_byte <CV address [0-1023]>
        Read CV byte
 - write_cv_byte <CV address [0-1023]> <CV value [0-255]>
        Write CV byte
 - read_cv_bit <CV address [0-1023]> <Bit> <Bit position [0-7]>
        Read CV bit
 - write_cv_bit <CV address [0-1023]> <Bit> <Bit position [0-7]>
        Write CV bit
```

Set speed level 10 in the reverse direction by sending an "advanced operations speed packet".
```sh
dcc> direction_speed 0 10
dcc> Read CV byte 28==2
dcc> Address 3: set direction backward
dcc> Address 3: set speed 18
```

#### ESP32
On [ESP32 platforms](https://www.espressif.com/en/products/socs/esp32) examples from the [examples](https://github.com/ZIMO-Elektronik/DCC/raw/master/examples) subfolder can be built directly using the [IDF Frontend](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/tools/idf-py.html).

```sh
idf.py create-project-from-example "zimo-elektronik/dcc^0.34.0:esp32"
```

#### STM32
An example that runs on [STM32](https://www.st.com/en/microcontrollers-microprocessors/stm32-32-bit-arm-cortex-mcus.html) platforms is a decoder and command station pair for a [NUCLEO-H743ZI](https://www.st.com/en/evaluation-tools/nucleo-h743zi.html) development board.

```sh
cmake -Bbuild -GNinja -DARCH="-mcpu=cortex-m7 -mfloat-abi=hard" -DCMAKE_TOOLCHAIN_FILE=CMakeModules/cmake/toolchain-arm-none-eabi-gcc.cmake
cmake --build build --target DCCStm32Decoder DCCStm32CommandStation
```

<img src="https://github.com/ZIMO-Elektronik/DCC/raw/master/data/images/NUCLEO-H743ZI.png" width="50%" align="right"/>

This example builds two firmwares, one for the decoder (`DCCStm32Decoder.hex`) and one for the command station (`DCCStm32CommandStation.hex`). Both files must be flashed onto a development board each (e.g. with the [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html)).

Since this example simulates real transmission over a track, it is also necessary to connect the two PE5 pins (N track) and the two PE5 pins (P track) with each other. The command station uses the pins as outputs to send a DCC signal, the decoder uses the pins as inputs to receive the same signal again. The development board with command station firmware can be recognized by the permanently lit red LED.

During ongoing operation, the following steps are repeated in an endless loop:
1.  Accelerate loco "3" to speed step 42 in forward direction
2.  Turn on green LED
3.  Wait for 2s
4.  Set function F3 on loco "3"
5.  Turn on yellow LED
6.  Wait for 2s
7.  Stop loco "3"
8.  Turn off green LED
9.  Wait for 2s
10. Clear function F3 on loco "3"
11. Turn off yellow LED
12. Wait for 2s

There is also a virtual com port (baud rate 115200) on the micro USB plug (CN1) through which the sent/received commands can be monitored.

## Usage
### Receiver
To create a receiver (decoder) class it is necessary to derive from `dcc::rx::CrtpBase`. As the name suggest this class relies on [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern) to implement static polymorphism. The template argument of the base is checked with a concept called [Decoder](include/dcc/rx/decoder.hpp). This concept verifies that the following methods can be called from the base. The friend declarations are only necessary if the methods the base needs to call are not public.
```cpp
#include <dcc/dcc.hpp>

struct Decoder : dcc::rx::CrtpBase<Decoder> {
  friend dcc::rx::CrtpBase<Decoder>;

private:
  // Set direction (1 forward, 0 backward)
  void direction(uint32_t addr, bool dir);

  // Set speed [-1, 255] (regardless of CV settings)
  void speed(uint32_t addr, int32_t speed);

  // Set function inputs
  void function(uint32_t addr, uint32_t mask, uint32_t state);

  // Enter or exit service mode
  void serviceModeHook(bool service_mode);

  // Generate current pulse as service ACK
  void serviceAck();

  // Transmit BiDi
  void transmitBiDi(std::span<uint8_t const> bytes);

  // Read CV
  uint8_t readCv(uint32_t cv_addr, uint8_t byte = 0u);

  // Write CV
  uint8_t writeCv(uint32_t cv_addr, uint8_t byte);

  // Read CV bit
  bool readCv(uint32_t cv_addr, bool bit, uint32_t pos);

  // Write CV bit
  bool writeCv(uint32_t cv_addr, bool bit, uint32_t pos);
};
```

Implementing the [Decoder](include/dcc/rx/decoder.hpp) concept alone is not enough to get a working receiver though. The following points are still necessary:
1. After instantiating the class, the `init` method must be called. This triggers the actual configuration and results in a series of CV read calls. Things like the primary address, the number of speed steps or whether BiDi is enabled is determined. These things are intentionally not done in the constructor in case the class is instantiated globally and the CVs aren't available at that point.
    ```cpp
    // Initializing the decoder is mandatory
    decoder.init();
    ```

2. The DCC signal on the track must be used as input. At the receiving end, decoding is done by measuring the time between two consecutive zero crossings of the signal. Typically this is done using the capture/compare unit of a hardware timer. The timer triggers a hardware interrupt in which the captured value must be read and passed to the `receive` method. `receive` expects a time in **microseconds**.
    ```cpp
    // Timer interrupt handler
    void isr() {
      auto const ccr{TIM->CCR};  // Read capture/compare register
      decoder.receive(ccr);      // Pass captured value in µs
    }
    ```

3. In order to keep the time in handler mode (interrupt context) as short as possible, received packets (with the exception of [RCN-218](https://normen.railcommunity.de/RCN-218.pdf) ones) are not executed immediately. For received packets to be executed, the `execute` method must be called periodically. This could either be done either inside a super-loop or, as in the snippet below, in an RTOS task.
    ```cpp
    // RTOS task
    void task(void*) {
      for (;;) {
        decoder.execute();
        vTaskDelay(pdMS_TO_TICKS(5u));
      }
    }
    ```

#### Optional
There are various optional methods that can be implemented if required. One of them are asynchronous CV methods that contain a callback as the last parameter. These methods allow to return immediately and execute the callback at a later point in time. Another addition can enable or disable high-current BiDi if the corresponding bit is set in CV29. And last but not least, the east-west direction according to [RCN-212](https://normen.railcommunity.de/RCN-212.pdf) is supported.
```cpp
  // Read CV asynchronously
  void readCv(uint32_t cv_addr, uint8_t byte, std::function<void(uint8_t)> cb);

  // Read CV bit asynchronously
  void readCv(uint32_t cv_addr,
              bool bit,
              uint32_t pos,
              std::function<void(bool)> cb);

  // Write CV asynchronously
  void
  writeCv(uint32_t cv_addr, uint8_t byte, std::function<void(uint8_t)> cb);

  // Write CV bit asynchronously
  void writeCv(uint32_t cv_addr,
               bool bit,
               uint32_t pos,
               std::function<void(bool)> cb);

  // High-current BiDi
  void highCurrentBiDi(bool high_current);

  // Set east-west direction
  void eastWestDirection(uint32_t cv_addr, std::optional<bool> dir);
```

### Transmitter
As before for the receiver, for the transmitter (command station) we need to derive from a class, this time from `dcc::tx::CrtpBase`. The template argument of the base is checked with a concept called [CommandStation](include/dcc/tx/command_station.hpp).
```cpp
#include <dcc/dcc.hpp>

struct CommandStation : dcc::tx::CrtpBase<CommandStation> {
  friend dcc::tx::CrtpBase<CommandStation>;

private:
  // Write track outputs
  void trackOutputs(bool N, bool P);

  // BiDi start
  void biDiStart();

  // BiDi channel 1
  void biDiChannel1();

  // BiDi channel 2
  void biDiChannel2();

  // BiDi end
  void biDiEnd();
};
```

Again implementing the [CommandStation](include/dcc/tx/command_station.hpp) concept isn't sufficient:
1. After we have instantiated the class we can configure the track signal by calling the `init` method. The method takes `Config` as a parameter and lets us set the number of preamble bits, the bit durations and whether a BiDi cutout should be generated. This step is optional, if `init` it is not called, then default settings are used.
    ```cpp
    // Initializing the command station is optional
    command_station.init({.num_preamble = 17u,
                          .bit1_duration = 58u,
                          .bit0_duration = 100u,
                          .bidi = true});
    ```

2. The DCC signal must be generated as output. A transmitter usually uses an H-bridge for this, in which the left and right sides are switched at dedicated times. The switching times are best maintained with a hardware timer interrupt. The times between the interrupts, i.e. the periods, correspond to the return value of the `transmit` method. Each time a new period is returned, the hardware timer must be reloaded with it. In order to comply with the standard timings, it is advisable to assign this interrupt a very high priority.
    ```cpp
    // Timer interrupt handler
    void isr() {
      auto const arr{command_station.transmit()};  // Get next timer period
      TIM->ARR = arr;                              // Set timer period register
    }
    ```

### ESP32 RMT encoder
Similar to the other encoders of the [ESP-IDF](https://github.com/espressif/esp-idf) framework, the RMT encoder has only one function to create a new instance. For more information on how to use the encoder please refer to the [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/rmt.html) or the RMT example.
```c
#include <rmt_dcc_encoder.h>

dcc_encoder_config_t encoder_config{.num_preamble = 17u,
                                    .bidibit_duration = 60u,
                                    .bit1_duration = 58u,
                                    .bit0_duration = 100u,
                                    .endbit_duration = 58u - 24u,
                                    .flags{.invert = false, .zimo0 = true}};
rmt_encoder_handle_t* encoder;
ESP_ERROR_CHECK(rmt_new_dcc_encoder(&encoder_config, &encoder));
```

The following members of `dcc_encoder_config_t` may require some explanation.

#### BiDi bit duration
This duration may be set to values between 57-61 to enable the generation of BiDi cutout bits prior to the next preamble. These four cutout bits would be sent in the background if the cutout was not active. The following graphic from [RCN-217](https://normen.railcommunity.de/RCN-217.pdf) visualizes these bits with a dashed line.
![BiDi cutout](https://github.com/ZIMO-Elektronik/DCC/raw/master/data/images/bidibit_duration.png)

#### End bit duration
Mainly due to a workaround of [esp-idf #13003](https://github.com/espressif/esp-idf/issues/13003) the end bit duration can be adjusted independently of the bit1 duration. This allows the RMT transmission complete callback to be executed at the right time.

#### Flags
- invert  
  Boolean value which corresponds to the level of the first half bit.

- zimo0  
  Transmit 0-bit prior to preamble.