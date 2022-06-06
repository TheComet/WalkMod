#include "anglemod/config.h"
#include "anglemod/seq.h"
#include <xc.h>

#if !defined(CLI_SIM) && !defined(GTEST_TESTING)
#pragma config SAFEN = 0
#endif

static struct config config;

/* -------------------------------------------------------------------------- */
void config_load_from_nvm(void)
{
    uint8_t* data_start = (uint8_t*)&config;
    uint8_t* data_end = (uint8_t*)&config + sizeof(config);
    
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
    if (config.magic != 0xAA)
    {
        config_set_defaults();
    }
}

/* -------------------------------------------------------------------------- */
void config_set_defaults(void)
{
    config.enable.normal_mode = NORMAL_MODE_CLAMP;
    config.enable.cardinal_angles = 0xFF;
    config.enable.diagonal_angles = 0xFF;
    config.enable.special_angles = 0x1F;

    config.joy.xythreshold = 42;
    config.joy.hysteresis = 30;

    config.dac_clamp.xy[0] = 41;
    config.dac_clamp.xy[1] = 41;

    config.dac_quantize.mode = QUANTIZE_8_UTILTS;

#define X(name, mirrx, mirry, str, init_x, init_y) \
    config.angles[SEQ_##name].xy[0] = init_x; \
    config.angles[SEQ_##name].xy[1] = init_y;
    SEQ_LIST
#undef X
}

/* -------------------------------------------------------------------------- */
struct config* config_get(void)
{
    return &config;
}

/* -------------------------------------------------------------------------- */
void config_save_to_nvm(void)
{
    const uint8_t* data_start = (uint8_t*)&config;
    const uint8_t* data_end = (uint8_t*)&config + sizeof(config);
    
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
