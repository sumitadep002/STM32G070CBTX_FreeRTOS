/**
 * @file      sx126x_hal.c
 *
 * @brief     Hardware Abstraction Layer implementation for SX126x
 */

#include "sx126x_hal.h"
#include "main.h"
#include "cmsis_os.h"
#include "common.h"

extern SPI_HandleTypeDef hspi1;

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS PROTOTYPES --------------------------------------------
 */

/**
 * @brief Wait until radio busy pin returns to 0 (low)
 * @return 1 if busy is low, 0 if timeout
 */
static uint8_t sx126x_hal_bsy( void );

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITIONS --------------------------------------------
 */

sx126x_hal_status_t sx126x_hal_write( const void* context, const uint8_t* command, const uint16_t command_length,
                                      const uint8_t* data, const uint16_t data_length )
{
    if( sx126x_hal_bsy( ) == 0 ) return SX126X_HAL_STATUS_ERROR;

    HAL_GPIO_WritePin( LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET );

    HAL_SPI_Transmit( &hspi1, ( uint8_t* ) command, command_length, LORA_SPI_TIMEOUT_TICKS );
    HAL_SPI_Transmit( &hspi1, ( uint8_t* ) data, data_length, LORA_SPI_TIMEOUT_TICKS );

    HAL_GPIO_WritePin( LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET );

    sx126x_hal_bsy( );

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_read( const void* context, const uint8_t* command, const uint16_t command_length,
                                     uint8_t* data, const uint16_t data_length )
{
    if( sx126x_hal_bsy( ) == 0 ) return SX126X_HAL_STATUS_ERROR;

    HAL_GPIO_WritePin( LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET );

    HAL_SPI_Transmit( &hspi1, ( uint8_t* ) command, command_length, LORA_SPI_TIMEOUT_TICKS );
    HAL_SPI_Receive( &hspi1, data, data_length, LORA_SPI_TIMEOUT_TICKS );

    HAL_GPIO_WritePin( LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET );

    sx126x_hal_bsy( );

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_reset( const void* context )
{
    HAL_GPIO_WritePin( LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_RESET );
    osDelay( 20 );
    HAL_GPIO_WritePin( LORA_RST_GPIO_Port, LORA_RST_Pin, GPIO_PIN_SET );

    return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_wakeup( const void* context )
{
    HAL_GPIO_WritePin( LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET );
    osDelay( 2 ); 
    HAL_GPIO_WritePin( LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET );

    return SX126X_HAL_STATUS_OK;
}

/* --- EOF ------------------------------------------------------------------ */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITIONS -------------------------------------------
 */

static uint8_t sx126x_hal_bsy( void )
{
    uint32_t timeout = LORA_BSY_TIMEOUT_TICKS;
    while( ( HAL_GPIO_ReadPin( LORA_BSY_GPIO_Port, LORA_BSY_Pin ) == GPIO_PIN_SET ) && ( timeout > 0 ) )
    {
        osDelay( 1 );
        timeout--;
    }
    
    if( timeout == 0 )
    {
        return 0; // Still high after timeout
    }
    return 1; // Successfully low
}

