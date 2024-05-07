#include "bsp.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/time.h>
#include "stm32h7xx_hal.h"
#include "stm32h7xx_ll_cortex.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_pwr.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_tim.h"
#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_utils.h"

// LEDs
#define LD1_Pin LL_GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define LD2_Pin LL_GPIO_PIN_1
#define LD2_GPIO_Port GPIOE
#define LD3_Pin LL_GPIO_PIN_14
#define LD3_GPIO_Port GPIOB

// STLINK UART bridge
#define STLINK_RX_Pin LL_GPIO_PIN_8
#define STLINK_RX_GPIO_Port GPIOD
#define STLINK_TX_Pin LL_GPIO_PIN_9
#define STLINK_TX_GPIO_Port GPIOD

// Track pins
#define TRACK_N_PIN LL_GPIO_PIN_5
#define TRACK_P_PIN LL_GPIO_PIN_6
#define TRACK_GPIO_Port GPIOE
#define TRACK_N_BS_Pos GPIO_BSRR_BS5_Pos
#define TRACK_N_BR_Pos GPIO_BSRR_BR5_Pos
#define TRACK_P_BS_Pos GPIO_BSRR_BS6_Pos
#define TRACK_P_BR_Pos GPIO_BSRR_BR6_Pos

static uint8_t* __sbrk_heap_end = NULL;

void* _sbrk(ptrdiff_t incr) {
  extern uint8_t _end;
  extern uint8_t _estack;
  extern uint32_t _Min_Stack_Size;
  uint32_t const stack_limit = (uint32_t)&_estack - (uint32_t)&_Min_Stack_Size;
  uint8_t const* max_heap = (uint8_t*)stack_limit;
  uint8_t* prev_heap_end;

  // Initialize heap end at first call
  if (NULL == __sbrk_heap_end) { __sbrk_heap_end = &_end; }

  // Protect heap from growing into the reserved MSP stack
  if (__sbrk_heap_end + incr > max_heap) {
    errno = ENOMEM;
    return (void*)-1;
  }

  prev_heap_end = __sbrk_heap_end;
  __sbrk_heap_end += incr;

  return (void*)prev_heap_end;
}

int _gettimeofday(struct timeval* ptimeval,
                  void* ptimezone __attribute__((unused))) {
  uint32_t const tick_ms = HAL_GetTick();
  ptimeval->tv_sec = tick_ms / 1000;
  ptimeval->tv_usec = (suseconds_t)tick_ms % 1000;
  return 0;
}

int _getpid() { return 1; }

int _kill(int, int) { return -1; }

void _exit(int) {
  for (;;)
    ;
}

int _close(int) { return -1; }

int _fstat(int, struct stat*) { return 0; }

int _isatty(int) { return 1; }

int _lseek(int, int, int) { return 0; }

int _read(int, char*, int) { return -1; }

int _write(int, char* ptr, int len) {
  for (int i = 0; i < len; ++i) {
    while (!LL_USART_IsActiveFlag_TXE_TXFNF(USART3))
      ;
    LL_USART_TransmitData8(USART3, ptr[i]);
  }
  return len;
}

void SysTick_Handler() { HAL_IncTick(); }

void HAL_MspInit() { __HAL_RCC_SYSCFG_CLK_ENABLE(); }

// Initialize system core clock to 64MHz
static void init_clock() {
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
  while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1) {}
  LL_PWR_ConfigSupply(LL_PWR_LDO_SUPPLY);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);
  while (LL_PWR_IsActiveFlag_VOS() == 0) {}
  LL_RCC_HSI_Enable();

  // Wait till HSI is ready
  while (LL_RCC_HSI_IsReady() != 1) {}
  LL_RCC_HSI_SetCalibTrimming(64);
  LL_RCC_HSI_SetDivider(LL_RCC_HSI_DIV1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

  // Wait till System clock is ready
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI) {}
  LL_RCC_SetSysPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAHBPrescaler(LL_RCC_AHB_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_1);
  LL_RCC_SetAPB4Prescaler(LL_RCC_APB4_DIV_1);
  LL_SetSystemCoreClock(64000000);

  // Update the time base
  if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK) _exit(-1);
}

// Initialize LEDs
static void init_gpio(void) {
  // GPIO ports clock enable
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOB);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOE);
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOH);

  LL_GPIO_ResetOutputPin(LD1_GPIO_Port, LD1_Pin);
  LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);
  LL_GPIO_ResetOutputPin(LD3_GPIO_Port, LD3_Pin);

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = LD1_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LD1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LD2_Pin;
  LL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LD3_Pin;
  LL_GPIO_Init(LD3_GPIO_Port, &GPIO_InitStruct);
}

// Initialize UART3 to 115.2k
static void init_uart(void) {
  LL_RCC_SetUSARTClockSource(LL_RCC_USART234578_CLKSOURCE_PCLK1);

  // Peripheral clock enable
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = STLINK_RX_Pin | STLINK_TX_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  LL_USART_InitTypeDef USART_InitStruct = {0};
  USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART3, &USART_InitStruct);
  LL_USART_SetTXFIFOThreshold(USART3, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_SetRXFIFOThreshold(USART3, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_DisableFIFO(USART3);
  LL_USART_ConfigAsyncMode(USART3);

  LL_USART_Enable(USART3);

  // Polling USART3 initialisation
  while ((!(LL_USART_IsActiveFlag_TEACK(USART3))) ||
         (!(LL_USART_IsActiveFlag_REACK(USART3)))) {}
}

//
static void bsp_common_init(void) {
  HAL_Init();

  init_clock();
  init_uart();
  init_gpio();
}

void bsp_init_decoder(void) {
  // Common part
  bsp_common_init();

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = TRACK_N_PIN | TRACK_P_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(TRACK_GPIO_Port, &GPIO_InitStruct);

  // Peripheral clock enable
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM15);

  // TIM15 interrupt
  NVIC_SetPriority(TIM15_IRQn,
                   NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  NVIC_EnableIRQ(TIM15_IRQn);

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  TIM_InitStruct.Prescaler = SystemCoreClock / 1000000;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 65535;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  TIM_InitStruct.RepetitionCounter = 0;
  LL_TIM_Init(TIM15, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM15);
  LL_TIM_SetClockSource(TIM15, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerOutput(TIM15, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM15);
  LL_TIM_IC_SetActiveInput(
    TIM15, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_IC_SetPrescaler(TIM15, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetFilter(TIM15, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1);
  LL_TIM_IC_SetPolarity(TIM15, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_RISING);

  // Enable CH1
  LL_TIM_CC_EnableChannel(TIM15, LL_TIM_CHANNEL_CH1);
  LL_TIM_EnableIT_CC1(TIM15);
  LL_TIM_EnableCounter(TIM15);
}

void bsp_init_command_station(void) {
  // Common part
  bsp_common_init();

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = TRACK_N_PIN | TRACK_P_PIN;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(TRACK_GPIO_Port, &GPIO_InitStruct);

  // Peripheral clock enable
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM15);

  // TIM15 interrupt
  NVIC_SetPriority(TIM15_IRQn,
                   NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  NVIC_EnableIRQ(TIM15_IRQn);

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  TIM_InitStruct.Prescaler = SystemCoreClock / 1000000;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 0;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM15, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM15);
  LL_TIM_SetClockSource(TIM15, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerOutput(TIM15, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM15);

  // Enable update
  LL_TIM_EnableIT_UPDATE(TIM15);
  LL_TIM_EnableCounter(TIM15);
}

// Handle timer interrupt for decoder
//
// Toggle input between TI1 and TI2, subtract captured value from running
// counter and clear capture/compare interrupt flag.
uint32_t bsp_decoder_irq(void) {
  // Get captured value
  uint32_t const ccr = LL_TIM_IC_GetCaptureCH1(TIM15);

  // Toggle input TI1 and TI2
  LL_TIM_CC_DisableChannel(TIM15, LL_TIM_CHANNEL_CH1);
  LL_TIM_IC_SetActiveInput(
    TIM15,
    LL_TIM_CHANNEL_CH1,
    LL_TIM_IC_GetActiveInput(TIM15, LL_TIM_CHANNEL_CH1) ==
        LL_TIM_ACTIVEINPUT_DIRECTTI
      ? LL_TIM_ACTIVEINPUT_INDIRECTTI
      : LL_TIM_ACTIVEINPUT_DIRECTTI);
  LL_TIM_CC_EnableChannel(TIM15, LL_TIM_CHANNEL_CH1);

  // Subtract captured value from running counter
  __disable_irq();
  LL_TIM_SetCounter(TIM15, LL_TIM_GetCounter(TIM15) - ccr);
  __enable_irq();

  // Clear capture/compare interrupt flag
  while (LL_TIM_IsActiveFlag_CC1(TIM15)) LL_TIM_ClearFlag_CC1(TIM15);

  return ccr;
}

// Handle timer interrupt for command station
//
// Reload ARR register of TIM15 and clear update interrupt flag.
void bsp_command_station_irq(uint32_t arr) {
  // Reload ARR register
  LL_TIM_SetAutoReload(TIM15, arr);

  // Clear update interrupt flag
  while (LL_TIM_IsActiveFlag_UPDATE(TIM15)) LL_TIM_ClearFlag_UPDATE(TIM15);
}

void bsp_write_track(bool N, bool P) {
  TRACK_GPIO_Port->BSRR = !N << TRACK_N_BR_Pos | !P << TRACK_P_BR_Pos |
                          N << TRACK_N_BS_Pos | P << TRACK_P_BS_Pos;
}

void bsp_write_green_led(bool on) {
  if (on) LL_GPIO_SetOutputPin(LD1_GPIO_Port, LD1_Pin);
  else LL_GPIO_ResetOutputPin(LD1_GPIO_Port, LD1_Pin);
}

void bsp_write_yellow_led(bool on) {
  if (on) LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);
  else LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);
}

void bsp_write_red_led(bool on) {
  if (on) LL_GPIO_SetOutputPin(LD3_GPIO_Port, LD3_Pin);
  else LL_GPIO_ResetOutputPin(LD3_GPIO_Port, LD3_Pin);
}

void bsp_delay(uint32_t ms) { HAL_Delay(ms); }