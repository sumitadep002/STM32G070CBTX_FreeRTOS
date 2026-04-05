/*
 * lcd.h
 *
 *  Created on: Apr 5, 2026
 *      Author: sumit
 */

#ifndef LCD_H_
#define LCD_H_

#include <stdint.h>

#define LCD_LOG_ENABLE
#define LCD_ADDRESS 0x3E
#define LCD_I2C_TIMEOUT_MS 1000

uint8_t lcd_init(void);
uint8_t lcd_clear(void);

#endif /* LCD_H_ */
