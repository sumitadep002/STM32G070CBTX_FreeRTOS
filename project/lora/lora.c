/**
 * @file      lora.c
 * @brief     LoRa driver high-level implementation
 */

#include "lora.h"
#include "lora_config.h"
#include "sx126x_hal.h"
#include <stdio.h>

#include "cmsis_os.h"
#include "main.h"

#define LORA_EVT_IRQ 0x01

/* --- PRIVATE VARIABLES ------------------------------------------------------- */

static lora_rx_cb_t lora_rx_callback = NULL;

static osThreadId_t lora_task_handle;
static volatile bool gf_tx_done = false;

static const osThreadAttr_t lora_task_attributes = {
    .name = "lora_task",
    .priority = (osPriority_t)osPriorityNormal,
    .stack_size = 128 * 4
};

/* --- PRIVATE FUNCTIONS PROTOTYPES -------------------------------------------- */

static void lora_task_handler(void *argument);
static void lora_task_init(void);



/* --- PUBLIC FUNCTIONS DEFINITIONS -------------------------------------------- */

bool lora_init(lora_rx_cb_t rx_cb)
{
    sx126x_status_t modem_status;
    sx126x_hal_status_t hal_status;

    lora_rx_callback = rx_cb;

    /* Initialize Hardware Abstraction Layer */
    hal_status = sx126x_hal_init(NULL);
    if (hal_status != SX126X_HAL_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-HAL-INIT failed\r\n");
        return false;
    }

    /* Hardware already reset and woken up in sx126x_hal_init, 
       but we can call them again to be sure if following user template strictly */
    hal_status = sx126x_hal_reset(NULL);
    if (hal_status != SX126X_HAL_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-HAL-RST failed\r\n");
        return false;
    }

    hal_status = sx126x_hal_wakeup(NULL);
    if (hal_status != SX126X_HAL_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-HAL-WAKEUP failed\r\n");
        return false;
    }

    /* Modem Configuration */
    modem_status = sx126x_set_standby(NULL, LORA_STANDBY_MODE);
    if (modem_status != SX126X_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-SET-STANDBY failed\r\n");
        return false;
    }

    modem_status = sx126x_set_reg_mode(NULL, LORA_REG_MODE);
    if (modem_status != SX126X_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-HAL-SET-REG-MODE failed\r\n");
        return false;
    }

    sx126x_pa_cfg_params_t pa_cfg_params = {
        .hp_max        = LORA_PA_CFG_PARAMS_HP_MAX,
        .pa_duty_cycle = LORA_PA_CFG_PARAMS_PA_DUTY_CYCLE,
        .device_sel    = LORA_PA_CFG_PARAMS_DEVICE_SEL,
        .pa_lut        = LORA_PA_CFG_PARAMS_PA_LUT,
    };
    modem_status = sx126x_set_pa_cfg(NULL, &pa_cfg_params);
    if (modem_status != SX126X_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-SET-PA failed\r\n");
        return false;
    }
#if 0
    /* TCXO Configuration */
    modem_status = sx126x_set_dio3_as_tcxo_ctrl(NULL, LORA_TCXO_CTRL_VOLTAGE, LORA_TCXO_CTRL_DELAY);
    if (modem_status != SX126X_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-SET-DIO3-AS-TCXO failed\r\n");
        return false;
    }
#endif

    /* RF Switch Configuration via DIO2 */
    modem_status = sx126x_set_dio2_as_rf_sw_ctrl(NULL, LORA_DIO2_RF_SW_CTRL);
    if (modem_status != SX126X_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-SET-DIO2-AS-RF-SWITCH failed\r\n");
        return false;
    }
    /* Packet and Frequency configuration */
    modem_status = sx126x_set_pkt_type(NULL, LORA_PKT_TYPE);
    if (modem_status != SX126X_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-SET-PKT-TYPE failed\r\n");
        return false;
    }

    modem_status = sx126x_set_rf_freq(NULL, LORA_RF_FREQ);
    if (modem_status != SX126X_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-SET-RF-FREQ failed\r\n");
        return false;
    }

    modem_status = sx126x_set_tx_params(NULL, LORA_TX_POWER, LORA_RAMP_TIME);
    if (modem_status != SX126X_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-SET-TX-PARAMS failed\r\n");
        return false;
    }

    /* LoRa Modulation Parameters */
    sx126x_mod_params_lora_t sx126x_mod_params_lora = {
        .bw   = LORA_MOD_PARAMS_BW,
        .sf   = LORA_MOD_PARAMS_SF,
        .cr   = LORA_MOD_PARAMS_CR,
        .ldro = LORA_MOD_PARAMS_LDRO
    };
    modem_status = sx126x_set_lora_mod_params(NULL, &sx126x_mod_params_lora);
    if (modem_status != SX126X_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-SET-MODULATION-PARAMS failed\r\n");
        return false;
    }

    /* LoRa Packet Parameters */
    sx126x_pkt_params_lora_t sx126x_pkt_params_lora = {
        .preamble_len_in_symb = LORA_PKT_PARAMS_PREAMBLE_LEN_IN_SYMB,
        .header_type          = LORA_PKT_PARAMS_HEADER_TYPE,
        .pld_len_in_bytes     = LORA_PKT_PARAMS_PLD_LEN_IN_BYTES,
        .crc_is_on            = LORA_PKT_PARAMS_CRC_IS_ON,
        .invert_iq_is_on     = LORA_PKT_PARAMS_INVERT_IQ_IS_ON
    };
    modem_status = sx126x_set_lora_pkt_params(NULL, &sx126x_pkt_params_lora);
    if (modem_status != SX126X_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-SET-PKT-PARAMS failed\r\n");
        return false;
    }

    /* CAD Parameters */
    sx126x_cad_params_t cad_params = {
        .cad_symb_nb     = LORA_CAD_PARAMS_CAD_SYMB_NUM,
        .cad_detect_peak = LORA_CAD_PARAMS_CAD_DETECT_PEAK,
        .cad_detect_min  = LORA_CAD_PARAMS_CAD_DETECT_MIN,
        .cad_exit_mode   = LORA_CAD_PARAMS_CAD_EXIT_MODE,
        .cad_timeout     = LORA_CAD_PARAMS_CAD_TIMEOUT,
    };
    modem_status = sx126x_set_cad_params(NULL, &cad_params);
    if (modem_status != SX126X_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-SET-CAD-PARAMS failed\r\n");
        return false;
    }

    modem_status = sx126x_cfg_rx_boosted(NULL, LORA_RX_BOOSTED);
    if (modem_status != SX126X_STATUS_OK)
    {
        LORA_LOG_ERR("SX126X-CFG-RX-BOOST failed\r\n");
        return false;
    }

    /* Set default LoRa Syncword */
    if (sx126x_set_lora_sync_word(NULL, LORA_DFT_SYNC_WORD) != SX126X_STATUS_OK)
    {
        LORA_LOG_FAIL("SET-DFT-SYNC-WORD failed\r\n");
    }


    LORA_LOG_INFO("LoRa Initialization successful\r\n");

    /* Default pin state: RX Mode */
    sx126x_hal_set_rf_switch_mode(NULL, SX126X_HAL_RF_SWITCH_RX);

#if (LORA_BOARD_MODE == LORA_MODE_RX)
    lora_start_rx(SX126X_RX_CONTINUOUS);
    LORA_LOG_INFO("LoRa started in RX mode\r\n");
#endif

    lora_task_init();
    return true;
}


bool lora_transmit(uint8_t *data, uint16_t length, uint32_t timeout)
{
    gf_tx_done = false;

    /* Enable TX path */
    sx126x_hal_set_rf_switch_mode(NULL, SX126X_HAL_RF_SWITCH_TX);

    sx126x_pkt_params_lora_t sx126x_pkt_params_lora = {
        .preamble_len_in_symb = LORA_PKT_PARAMS_PREAMBLE_LEN_IN_SYMB,
        .header_type          = LORA_PKT_PARAMS_HEADER_TYPE,
        .pld_len_in_bytes     = (uint8_t)length,
        .crc_is_on            = LORA_PKT_PARAMS_CRC_IS_ON,
        .invert_iq_is_on      = LORA_PKT_PARAMS_INVERT_IQ_IS_ON
    };
    sx126x_set_lora_pkt_params(NULL, &sx126x_pkt_params_lora);

    sx126x_set_standby(NULL, LORA_STANDBY_MODE);
    sx126x_set_buffer_base_address(NULL, 0, 0);
    sx126x_write_buffer(NULL, 0, data, (uint8_t)length);
    sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
    sx126x_set_dio_irq_params(NULL,
                              SX126X_IRQ_TX_DONE | SX126X_IRQ_TIMEOUT,
                              SX126X_IRQ_TX_DONE | SX126X_IRQ_TIMEOUT,
                              SX126X_IRQ_NONE, SX126X_IRQ_NONE);
    sx126x_set_tx(NULL, timeout);

    uint32_t temp = timeout;
    while (!gf_tx_done && temp)
    {
        temp--;
        osDelay(1);
    }

    /* Return to RX path */
    sx126x_hal_set_rf_switch_mode(NULL, SX126X_HAL_RF_SWITCH_RX);

    return gf_tx_done;
}


void lora_start_rx(uint32_t timeout)
{
    /* Enable RX path */
    sx126x_hal_set_rf_switch_mode(NULL, SX126X_HAL_RF_SWITCH_RX);

    sx126x_set_standby(NULL, LORA_STANDBY_MODE);
    sx126x_set_buffer_base_address(NULL, 0, 0);
    sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
    
    sx126x_set_dio_irq_params(NULL,
                              SX126X_IRQ_RX_DONE | SX126X_IRQ_TIMEOUT | SX126X_IRQ_CRC_ERROR,
                              SX126X_IRQ_RX_DONE | SX126X_IRQ_TIMEOUT | SX126X_IRQ_CRC_ERROR,
                              SX126X_IRQ_NONE, SX126X_IRQ_NONE);
    sx126x_set_rx(NULL, timeout);
}




static void lora_task_init(void)
{
    lora_task_handle = osThreadNew(lora_task_handler, NULL, &lora_task_attributes);
    if (lora_task_handle == NULL)
    {
        LORA_LOG_ERR("Failed to create LoRa task\r\n");
    }
    else
    {
        LORA_LOG_INFO("LoRa task created successfully\r\n");
    }
}


void lora_handle_interrupt(void)
{
    if (lora_task_handle != NULL)
    {
        osThreadFlagsSet(lora_task_handle, LORA_EVT_IRQ);
    }
}

static void lora_task_handler(void *argument)
{
    uint32_t flags;
    for (;;)
    {
        flags = osThreadFlagsWait(LORA_EVT_IRQ, osFlagsWaitAny, osWaitForever);
        if (flags & LORA_EVT_IRQ)
        {
            sx126x_irq_mask_t irq_status;
            if (sx126x_get_irq_status(NULL, &irq_status) == SX126X_STATUS_OK)
            {
                sx126x_clear_irq_status(NULL, SX126X_IRQ_ALL);
                
                if (irq_status & SX126X_IRQ_TX_DONE)
                {
                    LORA_LOG_INFO("TX DONE\r\n");
                    gf_tx_done = true;
                }

                
                if (irq_status & SX126X_IRQ_RX_DONE)
                {
                    sx126x_set_standby(NULL, LORA_STANDBY_MODE);

                    sx126x_rx_buffer_status_t rx_buffer_status;
                    sx126x_get_rx_buffer_status(NULL, &rx_buffer_status);
                    
                    uint8_t rx_data[256];
                    sx126x_read_buffer(NULL, rx_buffer_status.buffer_start_pointer, rx_data, rx_buffer_status.pld_len_in_bytes);
                    
                    sx126x_pkt_status_lora_t pkt_status;
                    sx126x_get_lora_pkt_status(NULL, &pkt_status);
                    
                    LORA_LOG_INFO("RX DONE: %d bytes, RSSI: %d, SNR: %d\r\n", 
                                 rx_buffer_status.pld_len_in_bytes, 
                                 pkt_status.rssi_pkt_in_dbm, 
                                 pkt_status.snr_pkt_in_db);

                    if (lora_rx_callback != NULL)
                    {
                        lora_rx_callback(rx_data, rx_buffer_status.pld_len_in_bytes, 
                                         pkt_status.rssi_pkt_in_dbm, pkt_status.snr_pkt_in_db);
                    }
                    
                    // Re-start RX if in continuous mode
                    if (LORA_BOARD_MODE == LORA_MODE_RX)
                    {
                        lora_start_rx(SX126X_RX_CONTINUOUS);
                    }
                }
                
                if (irq_status & SX126X_IRQ_TIMEOUT)
                {
                    LORA_LOG_ERR("IRQ TIMEOUT\r\n");
                }

                if (irq_status & SX126X_IRQ_CRC_ERROR)
                {
                    LORA_LOG_ERR("RX CRC ERROR\r\n");
                    if (LORA_BOARD_MODE == LORA_MODE_RX)
                    {
                        lora_start_rx(SX126X_RX_CONTINUOUS);
                    }
                }
            }
        }
    }
}




