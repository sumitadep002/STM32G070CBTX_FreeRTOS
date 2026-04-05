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

// LCD Local functions declarations
static uint8_t lcd_scan();
static uint8_t lcd_compose_byte(uint8_t rs, uint8_t rw, uint8_t en, uint8_t bl, uint8_t byte_data, uint8_t nibble_type);

uint8_t lcd_init(void)
{
    // Init I2C1 before calling this function
    // Scan the bus for the LCD
    lcd_scan();

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