#ifndef LIBS_CLOCKGENERATOR_CLOCKGENERATOR_H_
#define LIBS_CLOCKGENERATOR_CLOCKGENERATOR_H_

/* Includes ---------------------------------------------------------------- */

#include "xstatus.h"

/** @addtogroup I2C
 *  @{
 */

/* Constants and macros ---------------------------------------------------- */
// clang-format off

// clang-format on

/* Types ------------------------------------------------------------------- */

/**
 * @brief Clock generator output signals description
 */
typedef enum ClockGenerator_Output_e
{
    /** Zynq PLL reference clock */
    ClockGenerator_Output_PLL_CH3 = 1,
    /** MGTH reference 250Mhz clock */
    ClockGenerator_Output_MGTH_REF_CLK_250Mhz = 2,
    /** MGTH reference 125Mhz clock */
    ClockGenerator_Output_MGTH_REF_CLK_125Mhz = 4,
    /** MGTH reference 100Mhz clock */
    ClockGenerator_Output_MGTH_REF_CLK_100Mhz = 8

} ClockGenerator_Output_t;

/* Exposed API ------------------------------------------------------------- */

XStatus clockGenerator_loadConfig(void);
XStatus clockGenerator_enableAll(void);
XStatus clockGenerator_disableAll(void);
XStatus clockGenerator_enable(uint8_t outputs);
XStatus clockGenerator_disable(uint8_t outputs);


/**
 * @}
 */

/**** END OF FILE ****/


#endif /* LIBS_CLOCKGENERATOR_CLOCKGENERATOR_H_ */