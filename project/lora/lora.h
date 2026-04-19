/**
 * @file      lora.h
 * @brief     LoRa driver high-level interface
 */

#ifndef LORA_H
#define LORA_H

#include <stdint.h>
#include <stdbool.h>
#include "sx126x.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- PUBLIC MACROS ----------------------------------------------------------- */

#define LORA_LOG_ENABLE // Enable logging for LoRa

#if defined(LORA_LOG_ENABLE)
#define LORA_LOG_ERR(...)   printf("LORA [ERR]: " __VA_ARGS__)
#define LORA_LOG_INFO(...)  printf("LORA [INFO]: " __VA_ARGS__)
#define LORA_LOG_FAIL(...)  printf("LORA [FAIL]: " __VA_ARGS__)
#else
#define LORA_LOG_ERR(...)
#define LORA_LOG_INFO(...)
#define LORA_LOG_FAIL(...)
#endif

#define LORA_MODE_TX 0
#define LORA_MODE_RX 1

/* Set the board mode here: LORA_MODE_TX or LORA_MODE_RX */
#define LORA_BOARD_MODE LORA_MODE_TX

/* --- PUBLIC TYPES ------------------------------------------------------------ */

/**
 * @brief  LoRa receive callback type
 */
typedef void (*lora_rx_cb_t)(uint8_t *data, uint16_t len, int16_t rssi, int8_t snr);

/* --- PUBLIC FUNCTIONS PROTOTYPES --------------------------------------------- */

/**
 * @brief  Initialize LoRa modem and parameters
 * 
 * @param  rx_cb Receive callback function
 * @return true if success, false otherwise
 */
bool lora_init(lora_rx_cb_t rx_cb);
void lora_handle_interrupt(void);
bool lora_transmit(uint8_t *data, uint16_t length, uint32_t timeout);
void lora_start_rx(uint32_t timeout);






#ifdef __cplusplus
}
#endif

#endif // LORA_H
