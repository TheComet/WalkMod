#include "anglemod/dac.h"
#include "anglemod/gpio.h"
#include "anglemod/param.h"

/* -------------------------------------------------------------------------- */
void dac_init(void)
{
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
}

/* -------------------------------------------------------------------------- */
void dac_override_clamp(uint8_t x, uint8_t y)
{
    const struct param* p = param_get();
    
    if (x >= p->dac_clamp.xl && x <= p->dac_clamp.xh)
        gpio_clear_swx();
    else
    {
        gpio_set_swx();
    }
    
    if (y >= p->dac_clamp.yl && y <= p->dac_clamp.yh)
        gpio_clear_swy();
    else
    {
        gpio_set_swy();
    }
}
