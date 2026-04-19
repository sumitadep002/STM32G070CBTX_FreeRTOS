/*
 * lora_config.h
 *
 *  Created on: Apr 12, 2026
 *      Author: sumit
 *
 * LoRa SX126x Configuration
 */

#ifndef LORA_CONFIG_H
#define LORA_CONFIG_H

#include "sx126x.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************************************
 *                                      Important Macro Defines                                       *
 ******************************************************************************************************/
/* SX126X Default SYNCWORD*/
#define LORA_DFT_SYNC_WORD 0x12

/* SX126X CONFIG ----------------------------------------------------------------*/
#define LORA_STANDBY_MODE SX126X_STANDBY_CFG_RC // Standby Mode in RC
#define LORA_REG_MODE SX126X_REG_MODE_DCDC      // Power Regulator Mode in DCDC

/* Power Amplifier Configuration -------------------------------------------------*/
#define LORA_PA_CFG_PARAMS_HP_MAX 0x07        // PA output power level, 0x00 to 0x07
#define LORA_PA_CFG_PARAMS_PA_DUTY_CYCLE 0x07 // PA output power level, 0x04 to 0x07
#define LORA_PA_CFG_PARAMS_DEVICE_SEL 0x00    // 0x00 for SX1262, 0x01 for SX1261
#define LORA_PA_CFG_PARAMS_PA_LUT 0x01        // Power Amplifier Lookup Table

/* TCXO Control Configuration -------------------------------------------------*/
#define LORA_TCXO_CTRL_VOLTAGE SX126X_TCXO_CTRL_3_3V // TCXO Control Voltage
#define LORA_TCXO_CTRL_DELAY 100                     // TCXO Control Delay

/* RF Switch Control Configuration -------------------------------------------------*/
// Set the RF switch control pins through DIO2
#define LORA_DIO2_RF_SW_CTRL 0 // Enable (1) or Disable (0)

/* Packet Configuration -------------------------------------------------*/
#define LORA_PKT_TYPE SX126X_PKT_TYPE_LORA // Packet Type
#define LORA_RF_FREQ 867000000             // RF Frequency in Hz

/* TX Configuration -------------------------------------------------*/
// Set the TX power to 22 dBm, ramp time 200 us
#define LORA_TX_POWER 22                 // TX Power
#define LORA_RAMP_TIME SX126X_RAMP_10_US // Ramp Time

/* LoRa Modulation Configuration -------------------------------------------------*/
#define LORA_MOD_PARAMS_BW SX126X_LORA_BW_125 // Bandwidth
#define LORA_MOD_PARAMS_SF SX126X_LORA_SF8    // Spreading Factor
#define LORA_MOD_PARAMS_CR SX126X_LORA_CR_4_8 // Coding Rate
#define LORA_MOD_PARAMS_LDRO 0                // Low Data Rate Optimization

/* LoRa Packet Configuration -------------------------------------------------*/
#define LORA_PKT_PARAMS_PREAMBLE_LEN_IN_SYMB 8               // Preamble Length in Symbols
#define LORA_PKT_PARAMS_HEADER_TYPE SX126X_LORA_PKT_EXPLICIT // Header Type
#define LORA_PKT_PARAMS_PLD_LEN_IN_BYTES 255                 // Payload Length in Bytes
#define LORA_PKT_PARAMS_CRC_IS_ON true                       // CRC is ON
#define LORA_PKT_PARAMS_INVERT_IQ_IS_ON false                // Invert IQ is OFF

/* CAD Configuration -------------------------------------------------*/
#define LORA_CAD_PARAMS_CAD_SYMB_NUM SX126X_CAD_16_SYMB // CAD Symbol Number
#define LORA_CAD_PARAMS_CAD_DETECT_PEAK 25              // CAD Detect Peak
#define LORA_CAD_PARAMS_CAD_DETECT_MIN 10               // CAD Detect Min
#define LORA_CAD_PARAMS_CAD_EXIT_MODE SX126X_CAD_ONLY   // CAD Exit Mode
#define LORA_CAD_PARAMS_CAD_TIMEOUT 300                 // CAD Timeout

/* RX Configuration -------------------------------------------------*/
#define LORA_RX_BOOSTED true // RX Boosted

#define LORA_CHANNEL_CLEAR_ATTEMPTS 10 // Channel Clear Attempts

#ifdef __cplusplus
}
#endif

#endif /* LORA_CONFIG_H */
