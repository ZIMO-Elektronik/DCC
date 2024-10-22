#include <driver/rmt_tx.h>
#include <esp_log.h>
#include <esp_task.h>
#include <rmt_dcc_encoder.h>
#include <array>
#include <dcc/dcc.hpp>

#define RMT_GPIO_NUM GPIO_NUM_21

extern "C" void app_main() {
  printf("DCC RMT encoder example");

  // Setup RMT on RMT_GPIO_NUM
  rmt_tx_channel_config_t chan_config{.gpio_num = RMT_GPIO_NUM,
                                      .clk_src = RMT_CLK_SRC_DEFAULT,
                                      .resolution_hz = 1'000'000u,  // 1MHz
                                      .mem_block_symbols =
                                        SOC_RMT_CHANNELS_PER_GROUP *
                                        SOC_RMT_MEM_WORDS_PER_CHANNEL,
                                      .trans_queue_depth = 2uz};
  rmt_channel_handle_t rmt_channel{};
  ESP_ERROR_CHECK(rmt_new_tx_channel(&chan_config, &rmt_channel));
  ESP_ERROR_CHECK(rmt_enable(rmt_channel));

  // New DCC encoder
  dcc_encoder_config_t encoder_config{.num_preamble = DCC_TX_MIN_PREAMBLE_BITS,
                                      .bidibit_duration = 60u,
                                      .bit1_duration = 58u,
                                      .bit0_duration = 100u,
                                      .endbit_duration = 58u - 24u,
                                      .flags{.invert = false, .zimo0 = true}};
  rmt_encoder_handle_t rmt_encoder{};
  ESP_ERROR_CHECK(rmt_new_dcc_encoder(&encoder_config, &rmt_encoder));

  auto idle_packet{dcc::make_idle_packet()};
  rmt_transmit_config_t rmt_transmit_config{};
  for (;;)
    ESP_ERROR_CHECK(rmt_transmit(rmt_channel,
                                 rmt_encoder,
                                 data(idle_packet),
                                 size(idle_packet),
                                 &rmt_transmit_config));
}
