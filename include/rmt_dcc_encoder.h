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
  /// Number of preamble bits [17, 255]
  /// (will get rounded to multiple of 2)
  uint8_t num_preamble;

  /// Optional duration of cutout bit [0 | 57, 61]
  uint16_t cutoutbit_duration;

  /// Duration of 1 bit [56, 60]
  uint8_t bit1_duration;

  /// Duration of 0 bit [97, 114]
  uint8_t bit0_duration;

  /// Duration of end bit [0, 60]
  uint8_t endbit_duration;

  struct {
    /// Invert
    bool invert : 1;

    /// ZIMO 0
    bool zimo0 : 1;
  } flags;
} dcc_encoder_config_t;

/// Create RMT DCC encoder which encodes DCC byte stream into RMT symbols
///
/// \param  config              DCC encoder configuration
/// \param  ret_encoder         Returned encoder handle
/// \return ESP_OK              Create RMT DCC encoder successfully
/// \return ESP_ERR_INVALID_ARG Create RMT DCC encoder failed because of
///                             invalid argument
/// \return ESP_ERR_NO_MEM      Create RMT DCC encoder failed because out of
///                             memory
/// \return ESP_FAIL            Create RMT DCC encoder failed because of other
///                             error
esp_err_t rmt_new_dcc_encoder(dcc_encoder_config_t const* config,
                              rmt_encoder_handle_t* ret_encoder);

#ifdef __cplusplus
}
#endif