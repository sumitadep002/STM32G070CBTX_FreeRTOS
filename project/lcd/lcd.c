/*
 * lcd.c
 *
 *  Created on: Apr 5, 2026
 *      Author: sumit
 */

// Standard hedaer files
#include "stdio.h"

// Project header files
#include "main.h"

#include "lcd.h"

// Externally defined I2C handle from main.c
extern I2C_HandleTypeDef hi2c1;

// LCD Commands
#define LCD_CMD_CLEAR_DISPLAY 0x01

// LCD Local functions declarations
static uint8_t lcd_scan();
static uint8_t lcd_compose_byte(uint8_t rs, uint8_t rw, uint8_t en, uint8_t bl, uint8_t byte_data, uint8_t nibble_type);
static uint8_t lcd_send_command(uint8_t cmd);

uint8_t lcd_init(void)
{
    // Init I2C1 before calling this function
    // Scan the bus for the LCD
    lcd_scan();
    lcd_clear();

    return 0;
}

uint8_t lcd_scan()
{
    // Check if device responds at this address
    if (HAL_I2C_IsDeviceReady(&hi2c1, LCD_ADDRESS << 1, 1, LCD_I2C_TIMEOUT_MS) == HAL_OK)
    {
#if defined(LCD_LOG_ENABLE)
        printf("LCD: Detected at 0x%02X\r\n", LCD_ADDRESS);
        return 0;
#endif
    }
#if defined(LCD_LOG_ENABLE)
    printf("LCD: Unable to detect at 0x%02X\r\n", LCD_ADDRESS);
    return 0;
#endif

    return 0xff; // Return 0xFF if LCD is not found
}

/**
 * @brief Compose a byte to send to I2C LCD via PCF8574
 *
 * @param rs          RS bit (0=command, 1=data)
 * @param rw          RW bit (0=write, 1=read)
 * @param en          EN bit (0=low, 1=high for pulse)
 * @param bl          Backlight bit (0=off, 1=on)
 * @param byte_data   Full byte to send (command or data)
 * @param nibble_type 0=high nibble, 1=low nibble
 * @return uint8_t Byte to send over I2C
 */
uint8_t lcd_compose_byte(uint8_t rs, uint8_t rw, uint8_t en, uint8_t bl, uint8_t byte_data, uint8_t nibble_type)
{
    uint8_t byte = 0;

    // Map control bits to PCF8574 pins
    byte |= (rs & 0x01) << 0; // P0 = RS
    byte |= (rw & 0x01) << 1; // P1 = RW
    byte |= (en & 0x01) << 2; // P2 = EN
    byte |= (bl & 0x01) << 3; // P3 = Backlight

    // Select nibble
    uint8_t nibble = (nibble_type == 0) ? (byte_data >> 4) & 0x0F : byte_data & 0x0F;

    // Put nibble on P4–P7
    byte |= nibble << 4;

    return byte;
}

uint8_t lcd_clear(void)
{
    return lcd_send_command(LCD_CMD_CLEAR_DISPLAY);
}

/**
 * @brief Send a command byte to the LCD
 *
 * @param cmd 8-bit command to send
 *
 * Sends the high nibble first, then low nibble, each with an EN pulse to latch.
 */
uint8_t lcd_send_command(uint8_t cmd)
{
    uint8_t i2c_byte[4];

    // --- High nibble ---
    i2c_byte[0] = lcd_compose_byte(0, 0, 1, 1, cmd, 0); // RS=0 (command), EN=1

    i2c_byte[1] = lcd_compose_byte(0, 0, 0, 1, cmd, 0); // EN=0, latch

    // --- Low nibble ---
    i2c_byte[2] = lcd_compose_byte(0, 0, 1, 1, cmd, 1); // EN=1, low nibble

    i2c_byte[3] = lcd_compose_byte(0, 0, 0, 1, cmd, 1); // EN=0, latch

    if (HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDRESS << 1, i2c_byte, 4, LCD_I2C_TIMEOUT_MS) != HAL_OK)
    {
        return 0xFF; // Return error if transmission fails
    }

    HAL_Delay(2); // Give LCD time to process command
    return 0;
}