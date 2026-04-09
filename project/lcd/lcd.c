/*
 * lcd.c
 *
 * Created on: Apr 5, 2026
 * Author: sumit
 */

// Standard header files
#include <stdio.h>
#include <string.h>

// Project header files
#include "main.h"
#include "lcd.h"

// Externally defined I2C handle from main.c
extern I2C_HandleTypeDef hi2c1;

// Slave Address
#define I2C_ADDR (LCD_ADDRESS << 1)

// --- LCD Control Bytes ---
#define LCD_CTRL_CMD 0x00  // Control byte for sending a command
#define LCD_CTRL_DATA 0x40 // Control byte for sending text data

// LCD Commands
// --- Table 0: Standard Commands ---
#define LCD_CMD_CLEAR 0x01       // Clear display (Needs 2ms delay)
#define LCD_CMD_HOME 0x02        // Cursor home (Needs 2ms delay)
#define LCD_CMD_ENTRY_MODE 0x06  // Cursor auto-increments to the right
#define LCD_CMD_DISP_OFF 0x08    // Display OFF
#define LCD_CMD_DISP_ON 0x0C     // Display ON, Cursor OFF, Blink OFF
#define LCD_CMD_DISP_ON_CUR 0x0E // Display ON, Cursor ON, Blink OFF
#define LCD_CMD_DISP_ON_BLK 0x0F // Display ON, Cursor ON, Blink ON

// Cursor Positioning
#define LCD_ROW_0 0x80
#define LCD_ROW_1 0xC0

// --- Error Logging Macro ---
#if defined(LCD_LOG_ENABLE)
#define LCD_LOG_ERR(...) printf("LCD [ERR]: " __VA_ARGS__)
#else
#define LCD_LOG_ERR(...)
#endif

// LCD Presence
static uint8_t lcd_presence = 0;

// LCD Local functions declarations
static uint8_t lcd_scan();
static uint8_t lcd_compose_byte(uint8_t rs, uint8_t rw, uint8_t en, uint8_t bl, uint8_t byte_data, uint8_t nibble_type);
static uint8_t lcd_send_command(uint8_t cmd);
static uint8_t lcd_send_data(uint8_t data);
uint8_t lcd_print_string(const char *str);

uint8_t lcd_init(void)
{
    if (lcd_scan() != 0)
    {
        LCD_LOG_ERR("Initialization failed - device not found\r\n");
        return 0xFF;
    }

    HAL_Delay(10);

    lcd_presence = 1;

    lcd_send_command(0x38); // Set to 8 bit, 2 row configuration
    // --- Standard Display Configuration ---
    lcd_send_command(LCD_CMD_DISP_ON);    // Turn screen on
    lcd_send_command(LCD_CMD_ENTRY_MODE); // Set text to print left-to-right
    lcd_clear();                          // Clear memory

    return 0;
}

uint8_t lcd_scan()
{
    // Check if device responds at this address
    if (HAL_I2C_IsDeviceReady(&hi2c1, I2C_ADDR, 1, LCD_I2C_TIMEOUT_MS) == HAL_OK)
    {
        return 0;
    }

    LCD_LOG_ERR("Unable to detect at 0x%02X\r\n", LCD_ADDRESS);
    return 0xff; // Return 0xFF if LCD is not found
}

/**
 * @brief Compose a byte to send to I2C LCD via PCF8574
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
    uint8_t status = lcd_send_command(LCD_CMD_CLEAR);

    HAL_Delay(2); // This shall not be removed

    return status;
}

/**
 * @brief Send a command byte to the LCD
 */
uint8_t lcd_send_command(uint8_t cmd)
{
    uint8_t buffer[2];
    buffer[0] = LCD_CTRL_CMD; // Control byte: RS = 0
    buffer[1] = cmd;          // The actual command

    if (HAL_I2C_Master_Transmit(&hi2c1, I2C_ADDR, buffer, sizeof(buffer), LCD_I2C_TIMEOUT_MS) != HAL_OK)
    {
        LCD_LOG_ERR("I2C Transmit Error (CMD 0x%02X)\r\n", cmd);
        return 0xFF;
    }
    return 0;
}

/**
 * @brief Send a data byte to the LCD
 */
uint8_t lcd_send_data(uint8_t data)
{
    uint8_t buffer[2];
    buffer[0] = LCD_CTRL_DATA; // Control byte: RS = 1
    buffer[1] = data;          // The actual Data

    if (HAL_I2C_Master_Transmit(&hi2c1, I2C_ADDR, buffer, sizeof(buffer), LCD_I2C_TIMEOUT_MS) != HAL_OK)
    {
        LCD_LOG_ERR("I2C Transmit Error (DATA 0x%02X)\r\n", data);
        return 0xFF;
    }
    return 0;
}

uint8_t lcd_print_string(const char *str)
{
    uint8_t buffer[17];
    uint8_t index = 0;

    buffer[index++] = LCD_CTRL_DATA;

    while (*str && index < sizeof(buffer))
    {
        buffer[index++] = (uint8_t)(*str++);
    }

    if (HAL_I2C_Master_Transmit(&hi2c1, I2C_ADDR, buffer, index, LCD_I2C_TIMEOUT_MS) != HAL_OK)
    {
        LCD_LOG_ERR("I2C Transmit Error (STRING)\r\n");
        return 0xff;
    }

    return 0;
}

uint8_t lcd_msg_left(const char *str1, const char *str2)
{
    if (str1 == NULL || str2 == NULL || strlen(str1) > 16 || strlen(str2) > 16)
    {
        LCD_LOG_ERR("Msg Left Error - Invalid string lengths or NULL pointers\r\n");
        return 0xff;
    }

    if (lcd_clear() != 0)
    {
        return 0xfe;
    }

    if (lcd_send_command(LCD_ROW_0) != 0)
    {
        return 0xfd;
    }

    if (lcd_print_string(str1) != 0)
    {
        return 0xfc;
    }

    if (lcd_send_command(LCD_ROW_1) != 0)
    {
        return 0xfb;
    }

    if (lcd_print_string(str2) != 0)
    {
        return 0xfa;
    }

    return 0;
}

uint8_t lcd_msg_middle(const char *str1, const char *str2)
{
    if (str1 == NULL || str2 == NULL || strlen(str1) > 16 || strlen(str2) > 16)
    {
        LCD_LOG_ERR("Msg Middle Error - Invalid string lengths or NULL pointers\r\n");
        return 0xff;
    }

    if (lcd_clear() != 0)
    {
        return 0xfe;
    }

    if (lcd_send_command(LCD_ROW_0 + (16 - strlen(str1)) / 2) != 0)
    {
        return 0xfd;
    }

    if (lcd_print_string(str1) != 0)
    {
        return 0xfc;
    }

    if (lcd_send_command(LCD_ROW_1 + (16 - strlen(str2)) / 2) != 0)
    {
        return 0xfb;
    }

    if (lcd_print_string(str2) != 0)
    {
        return 0xfa;
    }

    return 0;
}