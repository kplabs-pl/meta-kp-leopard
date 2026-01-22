/* Define to prevent recursive inclusion ----------------------------------- */
#ifndef LIBS_ZYNQI2C_ZYNQI2C_H_
#define LIBS_ZYNQI2C_ZYNQI2C_H_

/* Includes ---------------------------------------------------------------- */
#include <stdbool.h>
#include <stdint.h>

#include "xstatus.h"

/** @addtogroup ZynqI2C
 *  @{
 */

/* Constants and macros ---------------------------------------------------- */
// clang-format off


// clang-format on

/* Types ------------------------------------------------------------------- */


/* Exposed API ------------------------------------------------------------- */

XStatus zynqI2C_init(void);
XStatus zynqI2C_writeReg(uint8_t deviceAddr, uint8_t regAddr, uint8_t regValue);
XStatus zynqI2C_readReg(uint8_t deviceAddr, uint8_t regAddr, uint8_t* regValue);


/**
 * @}
 */

#endif /* LIBS_ZYNQI2C_ZYNQI2C_H_ */

/**** END OF FILE ****/