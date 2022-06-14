#include "anglemod/dac.h"
#include "anglemod/gpio.h"
#include "anglemod/config.h"
#include "anglemod/log.h"

#include "anglemod/uart.h"

static uint8_t dac_write_cmd[6] = {
    0x00, 0x00, 0x00, /* DAC0 */
    0x08, 0x00, 0x00  /* DAC1 */
};

static uint8_t log_counter = 0;

/* -------------------------------------------------------------------------- */
void dac_init(void)
{
    SSP1CON1 = 0x10;  /* CKP=1: clock polarity high when idle
                       * SSPM=0000: host mode, Fosc/4=8 MHz */
    SSP1CON1 = 0x30;  /* SSPEN=1: Enable serial port */
}

/* -------------------------------------------------------------------------- */
void dac_override_disable(void)
{
    gpio_clear_swxy();
    log_dac(0, 0, dac_write_cmd);
}

/* -------------------------------------------------------------------------- */
static void dac_buf_transfer(void)
{
    uint8_t i = 0;
    
    /* 
     * SPI bus is running at essentially the same frequency as the CPU and
     * there's no way to sleep or do other meaningful stuff while data is
     * transmitting, so we send the data in the foreground here.
     */
    
    gpio_select_dac();
    
    for (i = 0; i != 6; ++i)
    {    
        SSP1BUF = dac_write_cmd[i]; /* Byte to transfer */
        while (!SSP1STATbits.BF) {}   /* Wait for transmit complete */
        (void)SSP1BUF;                /* Reading received byte resets BF flag */
    }

    gpio_deselect_dac();

    /* Need to wait at least 96ns between latches */
    gpio_latch_dac();
    { volatile uint8_t i = 3; while (i--) {} }
    gpio_unlatch_dac();
}

/* -------------------------------------------------------------------------- */
void dac_override_clamp(const uint8_t xy[2])
{
    const struct config* c = config_get();
    
    uint8_t pending_sw = 0;
    static const uint8_t sw_bits[2] = {SWX_BIT, SWY_BIT};
    
    uint8_t* write_ptr = &dac_write_cmd[1];
    for (uint8_t i = 0; i != 2; ++i, write_ptr += 3)
    {
        uint8_t dac_value;

        uint8_t lower = (uint8_t)(128 - c->dac_clamp.xy[i]);
        uint8_t upper = (uint8_t)(128 + c->dac_clamp.xy[i]);
        
        if (xy[i] < lower)
            dac_value = lower;
        else if (xy[i] > upper)
            dac_value = upper;
        else
            continue;  /* Skip writing to DAC */
        
        /* Update 10-bit value to transfer to DAC in transmit buffer */
        write_ptr[0] = dac_value >> 6;
        write_ptr[1] = (dac_value << 2) & 0xFF;
        pending_sw |= sw_bits[i];  /* Analog switch needs to be enabled after latch */
    }
    
    /* 
     * Transfer new values (if any) to DAC outputs. The slew rate is 0.44 V/us
     * which means it takes about 7.5us worst case for the DACs to settle over
     * a 3.3V range
     */
    if (pending_sw)
        dac_buf_transfer();

    /* Enable/disable analog switches as needed */
    PORTx(SW_PORT) = (PORTx(SW_PORT) & ~(SWX_BIT | SWY_BIT)) | pending_sw;
    
    /* This routine gets called at about 500 Hz. Try to log at 20 Hz */
    if (log_counter-- == 0)
    {
        log_dac(pending_sw & SWX_BIT, pending_sw & SWY_BIT, dac_write_cmd);
        log_counter = 25;
    }
}

/* -------------------------------------------------------------------------- */
void dac_override(const uint8_t xy[2])
{
    uint8_t* write_ptr = &dac_write_cmd[1];
    for (uint8_t i = 0; i != 2; ++i)
    {
        write_ptr[0] = xy[i] >> 6;
        write_ptr[1] = (xy[i] << 2) & 0xFF;
        write_ptr += 3;
    }

    dac_buf_transfer();
    PORTx(SW_PORT) |= (SWX_BIT | SWY_BIT);
    log_dac(1, 1, dac_write_cmd);
}
