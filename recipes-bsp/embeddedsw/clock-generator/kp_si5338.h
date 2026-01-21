/* Define to prevent recursive inclusion ----------------------------------- */
#ifndef LIBS_CLOCKGENERATOR_SI5338_SI5338_H_
#define LIBS_CLOCKGENERATOR_SI5338_SI5338_H_

/* Includes ---------------------------------------------------------------- */

#include "xstatus.h"

/** @addtogroup I2C
 *  @{
 */

/* Constants and macros ---------------------------------------------------- */
// clang-format off

/**
 * @brief SI5338 I2C address
 */
#define SI5338_ADDRESS  0x70


// clang-format on

/* Types ------------------------------------------------------------------- */

/**
 * @brief Structure containing register configuration info
 */
typedef struct SI5338_ClockGenerator_RegData_s
{
    /** Register address */
    uint8_t address;
    /** Register value */
    uint8_t value;
    /** Register mask */
    uint8_t mask;

} SI5338_ClockGenerator_RegData_t;

/**
 * @brief SI5338 clock generator structure.
 */
typedef struct SI5338_ClockGenerator_s
{
    /** I2C clock generator address */
    const uint8_t i2cAddress;
    /** Pointer to register config data table */
    const SI5338_ClockGenerator_RegData_t* const configRegData;
    /** Number of config registers */
    uint16_t configRegNum;

} SI5338_ClockGenerator_t;

/**
 * @brief SI5338 output channel description
 */
typedef enum SI5338_ClockGenerator_Channel_e
{
    SI5338_ClockGenerator_Channel_0,
    SI5338_ClockGenerator_Channel_1,
    SI5338_ClockGenerator_Channel_2,
    SI5338_ClockGenerator_Channel_3

} SI5338_ClockGenerator_Channel_t;


/* Exposed API ------------------------------------------------------------- */

XStatus si5338_loadConfiguration(SI5338_ClockGenerator_t* const clockGenerator);
XStatus si5338_enableAll(SI5338_ClockGenerator_t* const clockGenerator);
XStatus si5338_disableAll(SI5338_ClockGenerator_t* const clockGenerator);
XStatus si5338_enableMasterOutputEnable(SI5338_ClockGenerator_t* const clockGenerator);
XStatus si5338_disableMasterOutputEnable(SI5338_ClockGenerator_t* const clockGenerator);
XStatus si5338_enableChannel(
    SI5338_ClockGenerator_t* const clockGenerator,
    SI5338_ClockGenerator_Channel_t channel);
XStatus si5338_disableChannel(
    SI5338_ClockGenerator_t* const clockGenerator,
    SI5338_ClockGenerator_Channel_t channel);


/**
 * @}
 */

#endif /* LIBS_CLOCKGENERATOR_SI5338_SI5338_H_ */

/**** END OF FILE ****/