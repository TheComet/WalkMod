#include "anglemod/config.h"
#include "anglemod/seq.h"
#include <xc.h>

#define MAGIC 0xAA

static struct config config;

static const struct config default_config =
#if !defined(CLI_SIM) && !defined(GTEST_TESTING)
{
    .magic = MAGIC,
    .enable = {
        .cardinal_angles = 0xFF,
        .diagonal_angles = 0xFF,
        .special_angles = 0x1F,
        .normal_mode = NORMAL_MODE_CLAMP
    },
    .joy = {
        .xythreshold = 42,
        .hysteresis = 14
    },
    .dac_clamp = {
        .xy = {41, 41}
    },
    .dac_quantize = {
        .mode = QUANTIZE_8_UTILTS
    },
    .angles = {
#define X(name, mirrx, mirry, str, initx, inity) {initx, inity},
        SEQ_LIST
#undef X
    }
};
#else
{};
#endif

/* -------------------------------------------------------------------------- */
void config_set_defaults(void)
{
    config = default_config;
}

/* -------------------------------------------------------------------------- */
struct config* config_get(void)
{
    return &config;
}

/* -------------------------------------------------------------------------- */
void config_load_from_nvm(void)
{
    uint8_t* data_start = (uint8_t*)&config;
    uint8_t* data_end = (uint8_t*)&config + sizeof(config);

    /* 
     * Config structure is 58 bytes -> need 2 rows in SAF to store it.
     * 
     * SAF spans 128 words from 0x1F80 - 0x1FFF, where 1 row consists of 32 14-bit
     * program memory words.
     * First row is  0x1FC0-0x1FDF
     * Second row is 0x1FE0-0x1FFF
     */
    
    NVMCON1 = 0;  /* Point to PFM (instead of config) */

    uint8_t saf_addr_l = 0xC0;  /* Begin reading at 0x1FC0 */
    while (data_start != data_end)
    {
        NVMADRH = 0x1F;           /* Address of word to read (high byte) */
        NVMADRL = saf_addr_l;     /* Address of word to read (low byte) */
        NVMCON1bits.RD = 1;       /* Initiate read cycle */
        *data_start = NVMDATL;
        
        data_start++;
        saf_addr_l++;
    }
    
    /* See if the data we read makes any sense. If not, load the struct with
     * default values */
    if (config.magic != MAGIC)
        config_set_defaults();
}

/* -------------------------------------------------------------------------- */
static void write_byte(uint8_t saf_addr_l, uint8_t byte)
{
    NVMADRH = 0x1F;           /* Specify beginning of PFM row to erase (high byte) */
    NVMADRL = saf_addr_l;     /* Specify beginning of PFM row to erase (low byte) */
    NVMDATL = byte;

    /* Unlock sequence */
    NVMCON2 = 0x55;
    NVMCON2 = 0xAA;
    NVMCON1bits.WR = 1;
}

/* -------------------------------------------------------------------------- */
static void write_row(uint8_t saf_addr_l, const uint8_t* data)
{
    for (uint8_t i = 0; i != 32; ++i)
    {
        /* Last word in row? */
        if (i == 31)
            NVMCON1bits.LWLO = 0;  /* Next write command will write to PFM */

        write_byte(saf_addr_l++, *data++);
    }
}

/* -------------------------------------------------------------------------- */
uint8_t config_save_to_nvm(void)
{
    uint8_t success;
    
    /*
     * Config structure is 58 bytes -> need 2 rows in SAF to store it.
     *
     * SAF spans 128 words from 0x1F80 - 0x1FFF, where 1 row consists of 32 14-bit
     * program memory words.
     * First row is  0x1FC0-0x1FDF
     * Second row is 0x1FE0-0x1FFF
     */
    
    INTCONbits.GIE = 0;  /* Disable interrupts */
    
    /* Erase 64 bytes in SAF from 0x1FC0-0x1FFF */
    NVMCON1 = 0x14;  /* NVMREGS=0 (PFM), LWLO=0, FREE=1, WREN=1 */
    write_byte(0xC0, 0);   /* Erase 0x1FC0-0x1FDF */
    NVMCON1 = 0x14;  /* NVMREGS=0 (PFM), LWLO=0, FREE=1, WREN=1 */
    write_byte(0xE0, 0);   /* Erase 0x1FE0-0x1FFF */

    /* Write config structure */
    const uint8_t* data = (uint8_t*)&config;
    NVMCON1 = 0x24;  /* NVMREGS=0 (PFM), LWLO=1, FREE=0, WREN=1 */
    write_row(0xC0, data);
    NVMCON1 = 0x24;  /* NVMREGS=0 (PFM), LWLO=1, FREE=0, WREN=1 */
    write_row(0xE0, data + 32);
    
    success = !NVMCON1bits.WRERR;
    
    NVMCON1 = 0;           /* Disable writes */
    INTCONbits.GIE = 1;    /* Enable interrupts */
    
    return success;
}
