# Changelog

## 0.44.0
- Revert remove `DCC_RX_MIN_CV_WRITE_PACKETS` and add `DCC_STANDARD_COMPLIANCE` CMake option
- Add `DCC_RX_LOGON_DID_CV_ADDRESS`, `DCC_RX_LOGON_CID_CV_ADDRESS`, `DCC_RX_LOGON_SID_CV_ADDRESS` and `DCC_RX_LOGON_ADDRESS_CV_ADDRESS` CMake options
- Bugfix logon address de-/encoding ([#91](https://github.com/ZIMO-Elektronik/DCC/issues/91))
- Bugfix standard compliant LOGON_ENABLE ([#93](https://github.com/ZIMO-Elektronik/DCC/issues/93))

## 0.43.0
- Remove `DCC_STANDARD_COMPLIANCE` CMake option
- Add `DCC_RX_MIN_CV_WRITE_PACKETS` CMake option
- Keep track of last CV access packet for app:pom ([#90](https://github.com/ZIMO-Elektronik/DCC/issues/90))

## 0.42.1
- Bugfix wrong EStop decoding ([#86](https://github.com/ZIMO-Elektronik/DCC/issues/86))
- Bugfix ignore F0 exceptions on broadcasts ([#87](https://github.com/ZIMO-Elektronik/DCC/issues/87))

## 0.42.0
- `tx::CrtpBase` stores packets as either bytes or timings ([#57](https://github.com/ZIMO-Elektronik/DCC/issues/57))
- Add `bidi::Dissector` ([#59](https://github.com/ZIMO-Elektronik/DCC/issues/59))
- Add `DCC_STANDARD_COMPLIANCE` CMake option ([#78](https://github.com/ZIMO-Elektronik/DCC/issues/78))
- Bugfix receiver sends channel 1 replies to broadcast packets ([#60](https://github.com/ZIMO-Elektronik/DCC/issues/60))
- Bugfix CV29:0 did not work with consist ([#68](https://github.com/ZIMO-Elektronik/DCC/issues/68))
- Bugfix CV short access for CV23/24 ([#69](https://github.com/ZIMO-Elektronik/DCC/issues/69))
- Bugfix idle packet iterator not updated ([#71](https://github.com/ZIMO-Elektronik/DCC/issues/71))
- Bugfix accessory address encoding ([#74](https://github.com/ZIMO-Elektronik/DCC/issues/74))
- Bugfix standard compliant CV access ([#75](https://github.com/ZIMO-Elektronik/DCC/issues/75))
- Bugfix length check packets ([#77](https://github.com/ZIMO-Elektronik/DCC/issues/77))
- Bugfix consist control decoding ([#82](https://github.com/ZIMO-Elektronik/DCC/issues/82))

## 0.41.1
- Update to ZTL 0.21.0

## 0.41.0
- Add workaround for transmitting long consist addresses ([#53](https://github.com/ZIMO-Elektronik/DCC/issues/53))
- Rename `IdleSystem` to `Idle` address (255 no longer used for anything but idle)

## 0.40.4
- Bugfix tip-off search time corresponds to that of the first search packet ([#50](https://github.com/ZIMO-Elektronik/DCC/issues/50))
- Bugfix place `rmt_dcc_encoder_reset` in IRAM_ATTR

## 0.40.3
- Bugfix minimum CMake version set in `static_math` no longer supported by CMake 4.0.0

## 0.40.2
- Ignore `-Warray-bounds` for `decode_address`

## 0.40.1
- Add boolean return value to `bytes` and `packet` methods of `tx::CrtpBase`

## 0.40.0
- Add `size` and `capacity` methods to `tx::CrtpBase`
- Remove async CV bit operations
- Async CV operations are only used in service mode
- Add `BasicAccessory` and `ExtendedAccessory` address
- Rename `Short` to `BasicLoco` address
- Rename `Long` to `ExtendedLoco` address

## 0.39.0
- Bugfix disable LOGON_ENABLE after LOGON_SELECT ([#38](https://github.com/ZIMO-Elektronik/DCC/issues/38))

## 0.38.1
- Remove `zimo_id`
- Add `DCC_MANUFACTURER_ID` definition
- Bugfix CMake always includes tests

## 0.38.0
- Rename `rx::CrtpBase::cutoutChannel1` to `rx::CrtpBase::biDiChannel1` to match `tx`
- Rename `rx::CrtpBase::cutoutChannel2` to `rx::CrtpBase::biDiChannel2` to match `tx`

## 0.37.0
- Add `dcc::Address` overloads of `make_*_packet` functions
  - Allows to e.g. send address 100-127 as type `Long`
- Add `DCC_TX_MIN_BIDI_BIT_TIMING` definition
- Add `DCC_TX_MAX_BIDI_BIT_TIMING` definition

## 0.36.0
- Add speed, direction and functions instruction
- Bugfix no CV manipulation on consist address ([#26](https://github.com/ZIMO-Elektronik/DCC/issues/26))

## 0.35.3
- Bugfix typo in `decode_instruction` results in incorrect decoding ([#27](https://github.com/ZIMO-Elektronik/DCC/issues/27))

## 0.35.2
- Ignore `-Warray-bounds` for `decode_address`

## 0.35.1
- Bugfix POM only ever acknowledged once ([#15](https://github.com/ZIMO-Elektronik/DCC/issues/15))
- Bugfix operator precedence ([#24](https://github.com/ZIMO-Elektronik/DCC/issues/24))

## 0.35.0
- Add `dcc::Packet` and `std::span` overloads to `decode_address`
- Add `dcc::Packet` and `std::span` overloads to `decode_instruction`
- Block `rx::CrtpBase::execute` during BiDi cutout
- Bugfix POM ID0 datagrams may only be sent in BiDi cutouts following CV access commands ([#15](https://github.com/ZIMO-Elektronik/DCC/issues/15))

## 0.34.0
- Add datagram method to `rx::bidi::CrtpBase` to queue dyn (ID7) datagrams
- Replace `int32_t` direction (1, -1) with `bool` (1, 0)
- Remove decoders `emergencyStop` API and replace it with `speed(-1)`
- Add `Direction` and `Speed` enumerations
- Rename `bidi::Speed` to `bidi::Kmh` to avoid ambiguity
- Rename `tx::Config::preamble_bits` to `num_preamble` to match RMT configuration

## 0.33.1
- Add `DCC_TX_MAX_PREAMBLE_BITS` definition

## 0.33.0
- Limit number of preamble bits of RMT encoder to 30
- Rename `dcc_encoder_config_t::cutoutbit_duration` to `bidibit_duration`
- Remove optional mduEntry
- Bugfix standard compliant CV28
  - Logon must be enabled by CV28:7 and ignores CV28:1 and CV28:0
- Bugfix standard compliant RCN-217
  - Reply with active address in channel 1 (instead of just primary)
- Bugfix standard compliant RCN-218
  - Logon address is only temporary

## 0.32.0
- Individual timings for namespace `dcc::rx` and `dcc::tx`
- Move `time2bit` to namespace `dcc::rx`
- Bugfix RMT encoder [#13032](https://github.com/espressif/esp-idf/issues/13032)

## 0.31.0
- Changes to namespace `dcc::tx`
  - Rename `raw2timings` to `bytes2timings`
  - Rename `CrtpBase::raw` to `CrtpBase::bytes`

## 0.30.0
- Use `speed` instead of `notch`
- Update to ZTL 0.18.0

## 0.29.1
- Don't receive packets <=3 bytes

## 0.29.0
- Standardize tip-off search (use broadcast XF2 instead of reserved address 253)
- Standardize track voltage datagram (5V offset)

## 0.28.1
- Remove arbitrary RMT preamble limit

## 0.28.0
- More sophisticated packet
- `// [[fallthrough]];` not recognized by GCC

## 0.27.1
- Dependencies can be provided externally

## 0.27.0
- API change to transmitBiDi

## 0.26.2
- API changes to bytewise CV read

## 0.26.1
- Bugfix QoS datagram not written to deque

## 0.26.0
- API changes to bitwise CV read

## 0.25.0
- BiDi dyn datagrams get added from execute method
- Add BiDi temperature datagram (ID7, subindex 26)
- CV15/16 the values 0 can no longer activate the lock

## 0.24.1
- `Short` and `Long` addresses are considered equal if their values are
- `make_*_packet` functions
- Bugfix QoS datagram wasn't thread safe

## 0.24.0
- Rename `serviceMode` to `serviceModeHook`

## 0.23.0
- `time2bit` returns `Bit` enum instead of `std::optional` (overhead is pretty significant)

## 0.22.2
- Use CPM.cmake

## 0.22.1
- Bugfix service mode does not ignore idle packets

## 0.22.0
- Add consist control

## 0.21.4
- Top two bits of CV17 must always be set

## 0.21.3
- Update to ZTL 0.16.0

## 0.21.2
- Bugfix tip-off search is restarted if track voltage fails

## 0.21.1
- LOGON_ASSIGN CRC must be calculated before 6/8 encoding

## 0.21.0
- Mainline RCN-218
- LOGON_ENABLE does not have CRC
- LOGON_SELECT uses 6/8 encoding

## 0.20.1
- Add proprietary CV20

## 0.20.0
- Minimal implementation of (crippled) RCN-218
- Update to ZTL 0.15.0

## 0.19.0
- Update to ZTL 0.14.0
- [Semantic versioning](https://semver.org)
- Renamed concept Impl to Decoder
- Renamed namespace receive to rx

## 0.18
- Update to ZTL 0.13

## 0.17
- Update to ZTL 0.12

## 0.16
- Allow bit verify and bit write in operations mode

## 0.15
- Set CV29:5 in short form of configuration variable access

## 0.14
- Direction bits (CV29:1 and CV19:7)

## 0.13
- Receive must clear packet_end_ immediately

## 0.12
- Increase deque size to 31

## 0.11
- Non-linear scaling for 14 and 28 speed steps (minimum speed is equal with 126 speed steps)

## 0.10
- Make count variables size_t (for performance reasons)

## 0.9
- Removed CMake exports
- Update to ZTL 0.11

## 0.8
- Default paged mode index register to 1

## 0.7
- Bugfix `Short` addresses must also work in CV17/18

## 0.6
- Added register mode

## 0.5
- Removed rolling stock search
- Bugfix function group command must also respect exception for F0

## 0.4
- Stretched 0 bits are no longer mistaken for 1 bits

## 0.3
- New service mode which does not rely on preventing optimizations

## 0.2
- CV28:6 high-current BiDi support

## 0.1
- First release