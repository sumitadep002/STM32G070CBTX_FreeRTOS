/**
 * @file      common.h
 * @brief     Common macros and configurations for the project
 */

#ifndef COMMON_H
#define COMMON_H

#include "cmsis_os.h"

/* --- UTILITY MACROS -------------------------------------------------------- */

/**
 * @brief Convert milliseconds to RTOS ticks
 */
#define MS2TICKS(ms) ((ms * osKernelGetTickFreq()) / 1000)

/**
 * @brief Get current kernel tick count in milliseconds (assuming 1kHz tick)
 */
#define MILLIS() osKernelGetTickCount()

/* --- LORA CONFIGURATIONS --------------------------------------------------- */

#define LORA_BSY_TIMEOUT_TICKS  MS2TICKS(1000)
#define LORA_SPI_TIMEOUT_TICKS  MS2TICKS(100)

#endif // COMMON_H
