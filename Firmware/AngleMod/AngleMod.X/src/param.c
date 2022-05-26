#include "anglemod/param.h"
#include <xc.h>

#if !defined(CLI_SIM)
#pragma config SAFEN = 0
#endif

static struct param param;

/* -------------------------------------------------------------------------- */
static struct param default_param() {
#if defined(CLI_SIM)
    struct param p = { 0 };
#else
    struct param p = {
        .enable = { 
            .normal_mode = 0x01,  /* Clamp mode */
            .angle_modifiers = 1,
            .command_inputs = 1
        },
        .cmd_seq = {
            .xythreshold = 85,
            .hysteresis = 10
        },
        .dac_clamp = {
            .xl = 85,
            .xh = 171,
            .yl = 85,
            .yh = 171
        }
    };
#endif
    return p;
}

/* -------------------------------------------------------------------------- */
void param_init(void)
{
    uint8_t* data_start = (uint8_t*)&param;
    uint8_t* data_end = (uint8_t*)&param + sizeof(param);
    
    /* Address of last row? (0x3FE0 - 0x3FFF is 32 14-bit words right? */
    /* This is where we will save the structure */
    uint16_t saf_addr = 0x3FE0;
    
    while (data_start != data_end)
    {
        NVMCON1bits.NVMREGS = 0;  /* Point to PFM (instead of config) */
        NVMADR = saf_addr;        /* Address of word to read */
        NVMCON1bits.RD = 1;       /* Initiate read cycle */
        *data_start = NVMDATL;
        
        data_start++;
        saf_addr++;
    }
    
    /* See if the data we read makes any sense. If not, load the struct with
     * default values */
    if (param.dac_clamp.xl == param.dac_clamp.xh)
    {
        param = default_param();
    }
}

/* -------------------------------------------------------------------------- */
struct param* param_get(void)
{
    return &param;
}

/* -------------------------------------------------------------------------- */
void param_save_to_nvm(void)
{
    const uint8_t* data_start = (uint8_t*)&param;
    const uint8_t* data_end = (uint8_t*)&param + sizeof(param);
    
    /* 1 row consists of 32 14-bit program memory words. */
    /* SAF spans 128 words from 0x3F80 - 0x3FFF */
    
    /* Address of last row? (0x3FE0 - 0x3FFF is 32 14-bit words right? */
    /* This is where we will save the structure */
    uint16_t saf_addr = 0x3FE0;
    
    INTCONbits.GIE = 0;
    
    NVMCON1bits.NVMREGS = 0;  /* Point to PFM (instead of config) */
    NVMADR = saf_addr;        /* Specify beginning of PFM row to erase */
    NVMCON1bits.FREE = 1;     /* Specify an erase operation */
    NVMCON1bits.WREN = 1;     /* Allow erase cycle */
    
    /* Unlock sequence */
    NVMCON2 = 0x55;
    NVMCON2 = 0xAA;
    NVMCON1bits.WR = 1;
    
    NVMCON1bits.LWLO = 1;     /* Load write latches */
    while (data_start != data_end)
    {
        NVMADR = saf_addr;
        NVMDATL = *data_start;
        NVMCON2 = 0x55;
        NVMCON2 = 0xAA;
        NVMCON1bits.WR = 1;
        
        saf_addr++;
        data_start++;
    }
    
    /* Since our struct fits into just one row, can write it outside of the loop */
    NVMCON1bits.LWLO = 0;  /* Start PFM write */
    
    NVMCON1bits.WREN = 0;  /* Disable writes */
    INTCONbits.GIE = 1;    /* Enable interrupts */
}
