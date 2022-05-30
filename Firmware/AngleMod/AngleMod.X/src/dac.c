#include "anglemod/dac.h"
#include "anglemod/gpio.h"
#include "anglemod/param.h"
#include "anglemod/log.h"

static uint8_t dac01_write_buf[6] = {
    0x00, 0x00, 0x00, /* DAC0 */
    0x01, 0x00, 0x00  /* DAC1 */
};

/* -------------------------------------------------------------------------- */
void dac_init(void)
{
    SSP1CON1 = 0x10;  /* CKP=1: clock polarity high when idle
                       * SSPM=0000: host mode, Fosc/4=8 MHz */
    SSP1CON1 = 0x30;  /* SSPEN=1: Enable serial port after configuration */
}

/* -------------------------------------------------------------------------- */
void dac_set_clamp_threshold(uint8_t x, uint8_t y)
{
    struct param* p = param_get();
    
    if (x < 128)
    {
        p->dac_clamp.xl = (uint8_t)(128 - x);
        p->dac_clamp.xh = (uint8_t)(128 + x);
    }
    else
    {
        p->dac_clamp.xl = 0;
        p->dac_clamp.xh = 255;
    }
    
    if (y < 128)
    {
        p->dac_clamp.yl = (uint8_t)(128 - y);
        p->dac_clamp.yh = (uint8_t)(128 + y);
    }
    else
    {
        p->dac_clamp.yl = 0;
        p->dac_clamp.yh = 255;
    }
}

/* -------------------------------------------------------------------------- */
void dac_override_disable(void)
{
    gpio_clear_swxy();
    log_dac(0, 0, dac01_write_buf);
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
        SSP1BUF = dac01_write_buf[i]; /* Byte to transfer */
        while (SSP1STATbits.BF) {}    /* Wait for transmit complete */
        (void)SSP1BUF;                /* Reading received byte resets BF flag */
    }
    
    gpio_deselect_dac();

    /* Need to wait at least 96ns between latches. We're waiting 7.5us here */
    gpio_latch_dac();
    { volatile uint8_t i = 240; while (i--) {} }
    gpio_unlatch_dac();
}

/* -------------------------------------------------------------------------- */
void dac_override_clamp(uint8_t x, uint8_t y)
{
    const struct param* p = param_get();
    
    uint8_t pending_sw = 0;
    
    if (x < p->dac_clamp.xl)
    {
        /* Update 12-bit value to transfer to DAC in transmit buffer */
        dac01_write_buf[1] = p->dac_clamp.xl >> 4;
        dac01_write_buf[2] = (p->dac_clamp.xl << 4) & 0xFF;
        pending_sw = SWX_BIT;  /* Analog switch needs to be enabled after latch */
    }
    else if (x > p->dac_clamp.xh)
    {
        /* Update 12-bit value to transfer to DAC in transmit buffer */
        dac01_write_buf[1] = p->dac_clamp.xh >> 4;
        dac01_write_buf[2] = (p->dac_clamp.xh << 4) & 0xFF;
        pending_sw = SWX_BIT;  /* Analog switch needs to be enabled after latch */
    }
    
    if (y < p->dac_clamp.yl)
    {
        /* Update 12-bit value to transfer to DAC in transmit buffer */
        dac01_write_buf[4] = p->dac_clamp.yl >> 4;
        dac01_write_buf[5] = (p->dac_clamp.yl << 4) & 0xFF;
        pending_sw |= SWY_BIT;  /* Analog switch needs to be enabled after latch */
    }
    else if (y > p->dac_clamp.yh)
    {
        /* Update 12-bit value to transfer to DAC in transmit buffer */
        dac01_write_buf[4] = p->dac_clamp.yh >> 4;
        dac01_write_buf[5] = (p->dac_clamp.yh << 4) & 0xFF;
        pending_sw |= SWY_BIT;  /* Analog switch needs to be enabled after latch */
    }
    
    /* 
     * Transfer new values (if any) to DAC outputs. The slew rate is 0.44 V/us
     * which means it takes about 7.5us worst case for the DACs to settle over
     * a 3.3V range
     */
    if (pending_sw)
    {
        dac_buf_transfer();
        
        /* Enable/disable analog switches as needed */
        SW_PORT = (SW_PORT & ~pending_sw) | pending_sw;
    }
    
    log_dac(pending_sw & SWX_BIT, pending_sw & SWY_BIT, dac01_write_buf);
}

/* -------------------------------------------------------------------------- */
void dac_override_sequence(enum cmd_seq seq)
{
    const struct param* p = param_get();

    uint8_t x = p->angles[seq - 1].x;
    uint8_t y = p->angles[seq - 1].y;
    dac01_write_buf[1] = x >> 4;
    dac01_write_buf[2] = (x << 4) & 0xFF;
    dac01_write_buf[4] = y >> 4;
    dac01_write_buf[5] = (y << 4) & 0xFF;

    dac_buf_transfer();
    log_dac(1, 1, dac01_write_buf);
}
