// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

/// RMT DCC encoder
///
/// \file   rmt_dcc_encoder.c
/// \author Vincent Hamp
/// \date   08/01/2023

#include "rmt_dcc_encoder.h"
#include <esp_attr.h>
#include <esp_check.h>
#include <esp_heap_caps.h>
#include <limits.h>

#if __has_include(<esp_linux_helper.h>)
#  include <esp_linux_helper.h>
#endif

// https://github.com/espressif/esp-idf/issues/13032
#if !defined(RMT_MEM_ALLOC_CAPS)
#  if CONFIG_RMT_ISR_IRAM_SAFE || CONFIG_RMT_RECV_FUNC_IN_IRAM
#    define RMT_MEM_ALLOC_CAPS (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#  else
#    define RMT_MEM_ALLOC_CAPS MALLOC_CAP_DEFAULT
#  endif
#endif

static char const* TAG = "rmt";

/// DCC encoder
typedef struct {
  rmt_encoder_t base;
  rmt_encoder_t* copy_encoder;
  rmt_encoder_t* bytes_encoder;
  rmt_symbol_word_t cutout_symbol;
  rmt_symbol_word_t one_symbol;
  rmt_symbol_word_t zero_symbol;
  rmt_symbol_word_t end_symbol;
  size_t num_preamble_symbols;
  size_t num_symbols;
  enum { Cutout, Zimo0, Preamble, Start, Data, End } state;
  struct {
    bool zimo0 : 1;
  } flags;
} rmt_dcc_encoder_t;

/// Encode single bit
///
/// \param  dcc_encoder   DCC encoder handle
/// \param  channel       RMT TX channel handle
/// \param  ret_state     Returned current encoder state
/// \param  symbol        Symbol representing current bit
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t IRAM_ATTR rmt_encode_dcc_bit(rmt_dcc_encoder_t* dcc_encoder,
                                           rmt_channel_handle_t channel,
                                           rmt_encode_state_t* ret_state,
                                           rmt_symbol_word_t const* symbol) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = 0;
  rmt_encoder_handle_t copy_encoder = dcc_encoder->copy_encoder;
  encoded_symbols += copy_encoder->encode(
    copy_encoder, channel, symbol, sizeof(rmt_symbol_word_t), &state);
  *ret_state = state;
  return encoded_symbols;
}

/// Encode cutout
///
/// \param  dcc_encoder   DCC encoder handle
/// \param  channel       RMT TX channel handle
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t IRAM_ATTR rmt_encode_dcc_cutout(rmt_dcc_encoder_t* dcc_encoder,
                                              rmt_channel_handle_t channel,
                                              rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = 0;
  rmt_encoder_handle_t copy_encoder = dcc_encoder->copy_encoder;

  if (!dcc_encoder->cutout_symbol.duration0) state |= RMT_ENCODING_COMPLETE;
  else
    while (dcc_encoder->state == Cutout) {
      size_t const tmp = copy_encoder->encode(copy_encoder,
                                              channel,
                                              &dcc_encoder->cutout_symbol,
                                              sizeof(rmt_symbol_word_t),
                                              &state);
      encoded_symbols += tmp;
      dcc_encoder->num_symbols += tmp;
      if (state & RMT_ENCODING_COMPLETE &&
          dcc_encoder->num_symbols >= 8u / 2u) {
        dcc_encoder->num_symbols = 0u;
        dcc_encoder->state = Zimo0;
      }
      if (state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        goto out;
      }
    }

out:
  *ret_state = state;
  return encoded_symbols;
}

/// Encode ZIMO 0
///
/// \param  dcc_encoder   DCC encoder handle
/// \param  channel       RMT TX channel handle
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t IRAM_ATTR rmt_encode_dcc_zimo0(rmt_dcc_encoder_t* dcc_encoder,
                                             rmt_channel_handle_t channel,
                                             rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = 0;
  if (!dcc_encoder->flags.zimo0) state |= RMT_ENCODING_COMPLETE;
  else
    encoded_symbols += rmt_encode_dcc_bit(
      dcc_encoder, channel, &state, &dcc_encoder->zero_symbol);
  if (state & RMT_ENCODING_COMPLETE) dcc_encoder->state = Preamble;
  *ret_state = state;
  return encoded_symbols;
}

/// Encode preamble
///
/// \param  dcc_encoder   DCC encoder handle
/// \param  channel       RMT TX channel handle
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t IRAM_ATTR rmt_encode_dcc_preamble(rmt_dcc_encoder_t* dcc_encoder,
                                                rmt_channel_handle_t channel,
                                                rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = 0;
  rmt_encoder_handle_t copy_encoder = dcc_encoder->copy_encoder;

  while (dcc_encoder->state == Preamble) {
    size_t const tmp = copy_encoder->encode(copy_encoder,
                                            channel,
                                            &dcc_encoder->one_symbol,
                                            sizeof(rmt_symbol_word_t),
                                            &state);
    encoded_symbols += tmp;
    dcc_encoder->num_symbols += tmp;
    if (state & RMT_ENCODING_COMPLETE &&
        dcc_encoder->num_symbols >= dcc_encoder->num_preamble_symbols) {
      dcc_encoder->num_symbols = 0u;
      dcc_encoder->state = Start;
    }
    if (state & RMT_ENCODING_MEM_FULL) {
      state |= RMT_ENCODING_MEM_FULL;
      goto out;
    }
  }

out:
  *ret_state = state;
  return encoded_symbols;
}

/// Encode packet start
///
/// \param  dcc_encoder   DCC encoder handle
/// \param  channel       RMT TX channel handle
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t IRAM_ATTR rmt_encode_dcc_start(rmt_dcc_encoder_t* dcc_encoder,
                                             rmt_channel_handle_t channel,
                                             rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = 0;
  encoded_symbols +=
    rmt_encode_dcc_bit(dcc_encoder, channel, &state, &dcc_encoder->zero_symbol);
  if (state & RMT_ENCODING_COMPLETE) dcc_encoder->state = Data;
  *ret_state = state;
  return encoded_symbols;
}

/// Encode data
///
/// \param  dcc_encoder   DCC encoder handle
/// \param  channel       RMT TX channel handle
/// \param  primary_data  App data to be encoded into RMT symbols
/// \param  data_size     Size of primary_data, in bytes
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t IRAM_ATTR rmt_encode_dcc_data(rmt_dcc_encoder_t* dcc_encoder,
                                            rmt_channel_handle_t channel,
                                            void const* primary_data,
                                            size_t data_size,
                                            rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = 0;
  rmt_encoder_handle_t bytes_encoder = dcc_encoder->bytes_encoder;
  uint8_t const* data = (uint8_t const*)primary_data;

  size_t const tmp =
    bytes_encoder->encode(bytes_encoder,
                          channel,
                          &data[dcc_encoder->num_symbols / CHAR_BIT],
                          sizeof(uint8_t),
                          &state);
  encoded_symbols += tmp;
  dcc_encoder->num_symbols += tmp;
  if (state & RMT_ENCODING_COMPLETE &&
      dcc_encoder->num_symbols >= data_size * CHAR_BIT) {
    dcc_encoder->num_symbols = 0u;
    dcc_encoder->state = End;
  }
  if (state & RMT_ENCODING_MEM_FULL) state |= RMT_ENCODING_MEM_FULL;

  *ret_state = state;
  return encoded_symbols;
}

/// Encode packet end
///
/// \param  dcc_encoder   DCC encoder handle
/// \param  channel       RMT TX channel handle
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t IRAM_ATTR rmt_encode_dcc_end(rmt_dcc_encoder_t* dcc_encoder,
                                           rmt_channel_handle_t channel,
                                           rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0u;
  rmt_encode_state_t state = 0;
  encoded_symbols +=
    rmt_encode_dcc_bit(dcc_encoder, channel, &state, &dcc_encoder->end_symbol);
  if (state & RMT_ENCODING_COMPLETE) {
    dcc_encoder->num_symbols = 0u;
    dcc_encoder->state = Cutout;
  }
  *ret_state = state;
  return encoded_symbols;
}

/// Encode the user data into RMT symbols and write into RMT memory
///
/// \param  encoder       Encoder handle
/// \param  channel       RMT TX channel handle
/// \param  primary_data  App data to be encoded into RMT symbols
/// \param  data_size     Size of primary_data, in bytes
/// \param  ret_state     Returned current encoder state
/// \return Number of RMT symbols that the primary data has been encoded into
static size_t IRAM_ATTR rmt_encode_dcc(rmt_encoder_t* encoder,
                                       rmt_channel_handle_t channel,
                                       void const* primary_data,
                                       size_t data_size,
                                       rmt_encode_state_t* ret_state) {
  size_t encoded_symbols = 0;
  rmt_encode_state_t state = 0;
  rmt_encode_state_t session_state = 0;
  rmt_dcc_encoder_t* dcc_encoder =
    __containerof(encoder, rmt_dcc_encoder_t, base);

  switch (dcc_encoder->state) {
    case Cutout:
      encoded_symbols +=
        rmt_encode_dcc_cutout(dcc_encoder, channel, &session_state);
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        goto out;
      }
      // fallthrough

    case Zimo0:
      encoded_symbols +=
        rmt_encode_dcc_zimo0(dcc_encoder, channel, &session_state);
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        goto out;
      }
      // fallthrough

    case Preamble:
      encoded_symbols +=
        rmt_encode_dcc_preamble(dcc_encoder, channel, &session_state);
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        goto out;
      }
      // fallthrough

    case Start:
    start:
      encoded_symbols +=
        rmt_encode_dcc_start(dcc_encoder, channel, &session_state);
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        goto out;
      }
      // fallthrough

    case Data:
      encoded_symbols += rmt_encode_dcc_data(
        dcc_encoder, channel, primary_data, data_size, &session_state);
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        goto out;
      }
      if (dcc_encoder->state < End) goto start;
      // fallthrough

    case End:
      encoded_symbols +=
        rmt_encode_dcc_end(dcc_encoder, channel, &session_state);
      if (session_state & RMT_ENCODING_COMPLETE) state |= RMT_ENCODING_COMPLETE;
      if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        goto out;
      }
      // fallthrough
  }

out:
  *ret_state = state;
  return encoded_symbols;
}

/// Delete RMT DCC encoder
///
/// \param  encoder             RMT encoder handle
/// \return ESP_OK              Delete RMT DCC encoder successfully
/// \return ESP_ERR_INVALID_ARG Delete RMT DCC encoder failed because of invalid
///                             argument
/// \return ESP_FAIL            Delete RMT DCC encoder failed because of other
///                             error
static esp_err_t rmt_del_dcc_encoder(rmt_encoder_t* encoder) {
  rmt_dcc_encoder_t* dcc_encoder =
    __containerof(encoder, rmt_dcc_encoder_t, base);
  rmt_del_encoder(dcc_encoder->copy_encoder);
  rmt_del_encoder(dcc_encoder->bytes_encoder);
  free(dcc_encoder);
  return ESP_OK;
}

/// Reset RMT DCC encoder
///
/// \param  encoder             RMT encoder handle
/// \return ESP_OK              Reset RMT DCC encoder successfully
/// \return ESP_ERR_INVALID_ARG Reset RMT DCC encoder failed because of invalid
///                             argument
/// \return ESP_FAIL            Reset RMT DCC encoder failed because of other
///                             error
static esp_err_t rmt_dcc_encoder_reset(rmt_encoder_t* encoder) {
  rmt_dcc_encoder_t* dcc_encoder =
    __containerof(encoder, rmt_dcc_encoder_t, base);
  rmt_encoder_reset(dcc_encoder->copy_encoder);
  rmt_encoder_reset(dcc_encoder->bytes_encoder);
  dcc_encoder->num_symbols = 0u;
  dcc_encoder->state = Zimo0;
  return ESP_OK;
}

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
                              rmt_encoder_handle_t* ret_encoder) {
  esp_err_t ret = ESP_OK;
  rmt_dcc_encoder_t* dcc_encoder = NULL;
  ESP_GOTO_ON_FALSE(config && ret_encoder &&                    //
                      config->num_preamble >= 17u &&            //
                      (!config->cutoutbit_duration ||           //
                       (config->cutoutbit_duration >= 57u &&    //
                        config->cutoutbit_duration <= 61u)) &&  //
                      config->bit1_duration >= 56u &&           //
                      config->bit1_duration <= 60u &&           //
                      config->bit0_duration >= 97u &&           //
                      config->bit0_duration <= 114u &&          //
                      config->endbit_duration <= 60u,           //
                    ESP_ERR_INVALID_ARG,
                    err,
                    TAG,
                    "invalid argument");
  dcc_encoder =
    heap_caps_calloc(1, sizeof(rmt_dcc_encoder_t), RMT_MEM_ALLOC_CAPS);
  ESP_GOTO_ON_FALSE(
    dcc_encoder, ESP_ERR_NO_MEM, err, TAG, "no mem for dcc encoder");

  dcc_encoder->base.encode = rmt_encode_dcc;
  dcc_encoder->base.del = rmt_del_dcc_encoder;
  dcc_encoder->base.reset = rmt_dcc_encoder_reset;

  rmt_copy_encoder_config_t copy_encoder_config = {};
  ESP_GOTO_ON_ERROR(
    rmt_new_copy_encoder(&copy_encoder_config, &dcc_encoder->copy_encoder),
    err,
    TAG,
    "create copy encoder failed");

  // Number of preamble symbols
  dcc_encoder->num_preamble_symbols = config->num_preamble;

  // Setup RMT symbols
  if (config->cutoutbit_duration)
    dcc_encoder->cutout_symbol = (rmt_symbol_word_t){
      .duration0 = config->cutoutbit_duration,
      .level0 = 0u ^ config->flags.invert,
      .duration1 = config->cutoutbit_duration,
      .level1 = 1u ^ config->flags.invert,
    };
  dcc_encoder->one_symbol = (rmt_symbol_word_t){
    .duration0 = config->bit1_duration,
    .level0 = 0u ^ config->flags.invert,
    .duration1 = config->bit1_duration,
    .level1 = 1u ^ config->flags.invert,
  };
  dcc_encoder->zero_symbol = (rmt_symbol_word_t){
    .duration0 = config->bit0_duration,
    .level0 = 0u ^ config->flags.invert,
    .duration1 = config->bit0_duration,
    .level1 = 1u ^ config->flags.invert,
  };
  dcc_encoder->end_symbol = (rmt_symbol_word_t){
    .duration0 = config->bit1_duration,
    .level0 = 0u ^ config->flags.invert,
    .duration1 =
      config->endbit_duration ? config->endbit_duration : config->bit1_duration,
    .level1 = 1u ^ config->flags.invert,
  };

  // Initial state
  dcc_encoder->state = Zimo0;

  // Flags
  dcc_encoder->flags.zimo0 = config->flags.zimo0;

  rmt_bytes_encoder_config_t bytes_encoder_config = {
    .bit1 = dcc_encoder->one_symbol,
    .bit0 = dcc_encoder->zero_symbol,
    .flags.msb_first = true,
  };
  ESP_GOTO_ON_ERROR(
    rmt_new_bytes_encoder(&bytes_encoder_config, &dcc_encoder->bytes_encoder),
    err,
    TAG,
    "create bytes encoder failed");

  *ret_encoder = &dcc_encoder->base;
  return ESP_OK;
err:
  if (dcc_encoder) {
    if (dcc_encoder->bytes_encoder) rmt_del_encoder(dcc_encoder->bytes_encoder);
    if (dcc_encoder->copy_encoder) rmt_del_encoder(dcc_encoder->copy_encoder);
    free(dcc_encoder);
  }
  return ret;
}