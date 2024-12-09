// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// RMT DCC encoder
///
/// \file   rmt_dcc_encoder.h
/// \author Vincent Hamp
/// \date   08/01/2023

#pragma once

#include <driver/rmt_encoder.h>

#ifdef __cplusplus
extern "C" {
#endif

/// DCC encoder configuration
typedef struct {
  /// Number of preamble bits [17, 30]
  uint8_t num_preamble;

  /// Optional duration of BiDi cutout bit 0 || [57, 61]
  uint8_t bidibit_duration;

  /// Duration of 1 bit [56, 60]
  uint8_t bit1_duration;

  /// Duration of 0 bit [97, 114]
  uint8_t bit0_duration;

  /// Duration of end bit [0, 60]
  ///
  /// This is mostly to work around the limitations of
  /// https://github.com/espressif/esp-idf/issues/13003
  uint8_t endbit_duration;

  struct {
    /// Level of the first bit
    ///
    /// The name is identical to that of Espressif's RMT driver
    /// rmt_symbol_word_t structure.
    ///
    /// ```c
    /// typedef union {
    ///     struct {
    ///         uint16_t duration0 : 15; /*!< Duration of level0 */
    ///         uint16_t level0 : 1;     /*!< Level of the first part */
    ///         uint16_t duration1 : 15; /*!< Duration of level1 */
    ///         uint16_t level1 : 1;     /*!< Level of the second part */
    ///     };
    ///     uint32_t val; /*!< Equivalent unsigned value for the RMT symbol */
    /// } rmt_symbol_word_t;
    /// ```
    bool level0 : 1;

    /// ZIMO 0
    bool zimo0 : 1;
  } flags;
} dcc_encoder_config_t;

/// Create RMT DCC encoder which encodes DCC byte stream into RMT symbols
///
/// \param  config              DCC encoder configuration
/// \param  ret_encoder         Returned encoder handle
/// \retval ESP_OK              Create RMT DCC encoder successfully
/// \retval ESP_ERR_INVALID_ARG Create RMT DCC encoder failed because of
///                             invalid argument
/// \retval ESP_ERR_NO_MEM      Create RMT DCC encoder failed because out of
///                             memory
/// \retval ESP_FAIL            Create RMT DCC encoder failed because of other
///                             error
esp_err_t rmt_new_dcc_encoder(dcc_encoder_config_t const* config,
                              rmt_encoder_handle_t* ret_encoder);

#ifdef __cplusplus
}
#endif