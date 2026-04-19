#include "cfg_btn.h"
#include "cmsis_os.h"
#include "common.h"
#include "main.h" // For CFG_SW_Pin, CFG_SW_GPIO_Port
#include <stdio.h>

#define EVT_GPIO_PRESSED 0x01

#define USER_INPUT_TIME_15S 15000
#define USER_INPUT_TIME_12S 12000
#define USER_INPUT_TIME_9S 9000
#define USER_INPUT_TIME_6S 6000
#define USER_INPUT_TIME_3S 3000
#define USER_INPUT_TIME_1S 1000

#define USER_INPUT_TIMEOVERFLOW 16000

static cfg_btn_callback_t user_callback = NULL;
static osThreadId_t cfg_btn_task_handle;
static uint8_t block_gpio_interrupt = 0;

static const osThreadAttr_t cfg_btn_task_attributes = {
    .name = "cfg_btn_task",
    .priority = (osPriority_t)osPriorityNormal,
    .stack_size = 512 * 4};

static void cfg_btn_task_handler(void *argument) {
  uint32_t flags;
  for (;;) {
    flags = osThreadFlagsWait(EVT_GPIO_PRESSED, osFlagsWaitAny, osWaitForever);
    if (flags & EVT_GPIO_PRESSED) {
      osDelay(MS2TICKS(200)); // debounce delay
      uint32_t triggered_time = MILLIS();

      // Loop while button is still pressed
      while (GPIO_PIN_RESET == HAL_GPIO_ReadPin(CFG_SW_GPIO_Port, CFG_SW_Pin)) {
        osDelay(MS2TICKS(200));
        if (MILLIS() - triggered_time > USER_INPUT_TIMEOVERFLOW) {
          break;
        }
      }

      uint32_t input_time = MILLIS() - triggered_time;
      uint32_t timeout_to_report = 0;

      if (input_time >= USER_INPUT_TIME_15S) {
        timeout_to_report = 15000;
      } else if (input_time >= USER_INPUT_TIME_12S) {
        timeout_to_report = 12000;
      } else if (input_time >= USER_INPUT_TIME_9S) {
        timeout_to_report = 9000;
      } else if (input_time >= USER_INPUT_TIME_6S) {
        timeout_to_report = 6000;
      } else if (input_time >= USER_INPUT_TIME_3S) {
        timeout_to_report = 3000;
      } else {
        timeout_to_report = 1000;
      }

      if (user_callback != NULL) {
        user_callback(timeout_to_report);
      }

      // clear the flag once processed so we can accept new interrupts
      block_gpio_interrupt = 0;
    }
  }
}

void cfg_btn_init(cfg_btn_callback_t cb) {
  user_callback = cb;
  cfg_btn_task_handle =
      osThreadNew(cfg_btn_task_handler, NULL, &cfg_btn_task_attributes);
}

void cfg_btn_handle_interrupt(void) {
  if (block_gpio_interrupt == 0) {
    block_gpio_interrupt = 1;
    if (cfg_btn_task_handle != NULL) {
      osThreadFlagsSet(cfg_btn_task_handle, EVT_GPIO_PRESSED);
    }
  }
}
