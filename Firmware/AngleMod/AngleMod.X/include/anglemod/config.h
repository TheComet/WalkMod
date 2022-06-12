/*!
 * @file param.h
 * @author TheComet
 */

#ifndef CONFIG_H
#define	CONFIG_H

#include <stdint.h>

enum normal_mode
{
    NORMAL_MODE_OFF,
    NORMAL_MODE_CLAMP,
    NORMAL_MODE_QUANTIZE
};

enum quantize_mode
{
    QUANTIZE_4_CARDINAL,
    QUANTIZE_4_DIAGONAL,
    QUANTIZE_8_EVEN,
    QUANTIZE_8_UTILTS,
    QUANTIZE_12,
};

struct config
{
    uint8_t magic;

    union {
        struct {
            unsigned cardinal_angles : 8;
            unsigned diagonal_angles : 8;
            unsigned special_angles  : 8;
            unsigned normal_mode     : 2;  /* 00 = OFF, 01 = clamp, 10 = quantize */
            unsigned _padding        : 6;
        };
        uint8_t bytes[4];
    } enable;

    /*!
     * @brief Sets the threshold and hysteresis parameters which determines how
     * joystick angles are converted into states.
     * 
     * The joystick position is converted into one of 9 "states" depending on where
     * it is located on a 3x3 grid. The threshold parameter controls how far the
     * joystick needs to move before it is no longer neutral, and the hysteresis
     * parameter controls the gap to insert at the threshold position, which helps
     * eliminate noise.
     * 
     * The maximum allowable value for threshold is 128 minus half of the hysteresis.
     * Default value is set such that the grid is exactly a third of the full range.
     * 
     *               threshold
     *                |<-->|
     *                |    |
     *          | |       | |
     *      NW  | |   N   | |  NE
     *    ______|_|_______|_|______ 
     *    ______|_|_______|_|______ < hystersis
     *          | |       | |        
     *      W   | | Neut. | |  E
     *    ______|_|_______|_|______
     *    ______|_|_______|_|______
     *          | |       | |
     *      SW  | |   S   | |  SE
     *          | |       | |
     */
    struct {
        uint8_t xythreshold;
        uint8_t hysteresis;
    } joy;
    
    struct {
        uint8_t xy[2];
    } dac_clamp;

    struct {
        uint8_t mode;
    } dac_quantize;

    struct {
        uint8_t xy[2];
    } angles[24];

    uint8_t _padding[6];
};

void config_load_from_nvm(void);
void config_set_defaults(void);
uint8_t config_save_to_nvm(void);

struct config* config_get(void);

#endif	/* CONFIG_H */
