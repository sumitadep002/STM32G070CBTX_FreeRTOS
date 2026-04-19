/**
 * @file      sx126x_hal.c
 *
 * @brief     Hardware Abstraction Layer implementation for SX126x
 */

#include "sx126x_hal.h"
#include "cmsis_os.h"
#include "common.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;

#define SPI_TIMEOUT_MS 2000
#define MODEM_TIMEOUT_MS 5000

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS PROTOTYPES --------------------------------------------
 */

/**
 * @brief Wait until radio busy pin returns to 0 (low) using tight polling
 * @return 1 if busy is low, 0 if timeout
 */
static uint8_t sx126x_hal_bsy(void);

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITIONS --------------------------------------------
 */

sx126x_hal_status_t sx126x_hal_write(const void *context,
                                     const uint8_t *command,
                                     const uint16_t command_length,
                                     const uint8_t *data,
                                     const uint16_t data_length) {
  if (sx126x_hal_bsy() == 0) {
    return SX126X_HAL_STATUS_ERROR;
  }

  HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET);

  if (command_length > 0) {
    if (HAL_SPI_Transmit(&hspi1, (uint8_t *)command, command_length,
                         SPI_TIMEOUT_MS) != HAL_OK) {
      HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
      return SX126X_HAL_STATUS_ERROR;
    }
  }
  if (data_length > 0) {
    if (HAL_SPI_Transmit(&hspi1, (uint8_t *)data, data_length,
                         SPI_TIMEOUT_MS) != HAL_OK) {
      HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
      return SX126X_HAL_STATUS_ERROR;
    }
  }

  HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);

  return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_read(const void *context, const uint8_t *command,
                                    const uint16_t command_length,
                                    uint8_t *data, const uint16_t data_length) {
  if (sx126x_hal_bsy() == 0) {
    return SX126X_HAL_STATUS_ERROR;
  }

  HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET);

  if (command_length > 0) {
    if (HAL_SPI_Transmit(&hspi1, (uint8_t *)command, command_length,
                         SPI_TIMEOUT_MS) != HAL_OK) {
      HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
      return SX126X_HAL_STATUS_ERROR;
    }
  }
  if (data_length > 0) {
    if (HAL_SPI_Receive(&hspi1, data, data_length, SPI_TIMEOUT_MS) != HAL_OK) {
      HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
      return SX126X_HAL_STATUS_ERROR;
    }
  }

  HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);

  return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_reset(const void *context) {
  HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_RESET);
  osDelay(2);
  HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET);
  osDelay(10);

  return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_wakeup(const void *context) {
  /*
   * Following reference logic: Wakeup includes a hardware reset
   * and a pulse on NSS to ensure the chip is reactive.
   */
  sx126x_hal_reset(context);

  HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET);
  osDelay(2);
  HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);

  return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_init(const void *context) {
  /* Ensure NSS is High (Not selected) */
  HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);

  /* Perform Hardware Reset */
  sx126x_hal_reset(context);

  /* Wait for modem to become ready */
  if (sx126x_hal_bsy() == 0) {
    return SX126X_HAL_STATUS_ERROR;
  }

  return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t
sx126x_hal_set_rf_switch_mode(const void *context,
                              sx126x_hal_rf_switch_mode_t mode) {
#if 0
  if (mode == SX126X_HAL_RF_SWITCH_TX) {
    HAL_GPIO_WritePin(LORA_TXEN_GPIO_Port, LORA_TXEN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LORA_RXEN_GPIO_Port, LORA_RXEN_Pin, GPIO_PIN_RESET);
  } else {
    HAL_GPIO_WritePin(LORA_TXEN_GPIO_Port, LORA_TXEN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LORA_RXEN_GPIO_Port, LORA_RXEN_Pin, GPIO_PIN_SET);
  }
#endif

  return SX126X_HAL_STATUS_OK;
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITIONS
 * -------------------------------------------
 */

static uint8_t sx126x_hal_bsy(void) {
  uint32_t start_time = osKernelGetTickCount();

  /*
   * Tight poll the Busy pin to ensure maximum SPI throughput.
   * Replaces osDelay(1) which was causing massive overhead.
   */
  while ((HAL_GPIO_ReadPin(LORA_BSY_GPIO_Port, LORA_BSY_Pin) == GPIO_PIN_SET)) {
    if ((osKernelGetTickCount() - start_time) > MODEM_TIMEOUT_MS) {
      return 0; // Timeout
    }

    /*
     * For very short waits, we just spin.
     * If it takes longer than 5ms, we use osDelay(1) to let other tasks run.
     */
    if ((osKernelGetTickCount() - start_time) > 5) {
      osDelay(1);
    }
  }

  return 1; // Successfully low (Ready)
}

/* --- EOF ------------------------------------------------------------------
 */
