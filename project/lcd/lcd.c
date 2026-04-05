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
    if (HAL_I2C_IsDeviceReady(&hi2c1, LCD_ADDRESS << 1, 1, 10) == HAL_OK)
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