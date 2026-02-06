/* Includes ---------------------------------------------------------------- */
#include <stddef.h>

#include "kp_clockGenerator.h"
#include "kp_clockGenerator_registerConfig.h"
#include "kp_si5338.h"

/** @addtogroup clockGenerator
 *  @{
 */

/* Constants and macros ---------------------------------------------------- */
// clang-format off

// clang-format on

/* Private types ----------------------------------------------------------- */


/* Private variables ------------------------------------------------------- */

static SI5338_ClockGenerator_t clockGenerator_generator = {
    .i2cAddress    = SI5338_ADDRESS,
    .configRegData = &clockGenerator_registerConfig_regData[0],
    .configRegNum  = CLOCK_GENERATOR_REGISTER_CONFIG_REGS_NUM};


/* Private functions ------------------------------------------------------- */

/* Exposed API ------------------------------------------------------------- */

/**
 * @brief Load clock generator configuration from clockGenerator_registerConfig.h file
 * @param zynqInstance Number of Zynq instance which clock generator should be configured
 * @return SDK_ErrorCode_Success - success, errorCode - failure
 */
XStatus clockGenerator_loadConfig(void)
{
    XStatus errorCode = si5338_loadConfiguration(&clockGenerator_generator);

    return errorCode;
}

/**
 * @brief Enable all clock generator outputs
 * @param zynqInstance Number of Zynq instance which clock generator should be enabled
 * @return SDK_ErrorCode_Success - success, errorCode - failure
 */
XStatus clockGenerator_enableAll(void)
{
    XStatus errorCode = si5338_enableAll(&clockGenerator_generator);

    return errorCode;
}

/**
 * @brief Disable all clock generator outputs
 * @param zynqInstance Number of Zynq instance which clock generator should be disabled
 * @return SDK_ErrorCode_Success - success, errorCode - failure
 */
XStatus clockGenerator_disableAll(void)
{
    XStatus errorCode = si5338_disableAll(&clockGenerator_generator);

    return errorCode;
}

/**
 * @brief Enable specified clock generator channels
 * @param zynqInstance Number of Zynq instance which clock generator channels should be
 * enabled
 * @param outputs Outputs to be enabled described by ClockGenerator_Output_t enum
 * @return SDK_ErrorCode_Success - success, errorCode - failure
 */
XStatus clockGenerator_enable(uint8_t outputs)
{
    uint8_t errorCode = XST_SUCCESS;
    if (outputs & ClockGenerator_Output_PLL_CH3)
    {
        errorCode |= si5338_enableChannel(
            &clockGenerator_generator, SI5338_ClockGenerator_Channel_0);
    }

    if (outputs & ClockGenerator_Output_MGTH_REF_CLK_250Mhz)
    {
        errorCode |= si5338_enableChannel(
            &clockGenerator_generator, SI5338_ClockGenerator_Channel_1);
    }

    if (outputs & ClockGenerator_Output_MGTH_REF_CLK_125Mhz)
    {
        errorCode |= si5338_enableChannel(
            &clockGenerator_generator, SI5338_ClockGenerator_Channel_2);
    }

    if (outputs & ClockGenerator_Output_MGTH_REF_CLK_100Mhz)
    {
        errorCode |= si5338_enableChannel(
            &clockGenerator_generator, SI5338_ClockGenerator_Channel_3);
    }

    errorCode |= si5338_enableMasterOutputEnable(&clockGenerator_generator);

    return (XStatus)errorCode;
}

/**
 * @brief Disable specified clock generator channels
 * @param zynqInstance Number of Zynq instance which clock generator channels should be
 * disabled
 * @param outputs Outputs to be disabled described by ClockGenerator_Output_t enum
 * @return SDK_ErrorCode_Success - success, errorCode - failure
 */
XStatus clockGenerator_disable(uint8_t outputs)
{
    uint8_t errorCode = XST_SUCCESS;
    if (outputs & ClockGenerator_Output_PLL_CH3)
    {
        errorCode |= si5338_disableChannel(
            &clockGenerator_generator, SI5338_ClockGenerator_Channel_0);
    }

    if (outputs & ClockGenerator_Output_MGTH_REF_CLK_250Mhz)
    {
        errorCode |= si5338_disableChannel(
            &clockGenerator_generator, SI5338_ClockGenerator_Channel_1);
    }

    if (outputs & ClockGenerator_Output_MGTH_REF_CLK_125Mhz)
    {
        errorCode |= si5338_disableChannel(
            &clockGenerator_generator, SI5338_ClockGenerator_Channel_2);
    }

    if (outputs & ClockGenerator_Output_MGTH_REF_CLK_100Mhz)
    {
        errorCode |= si5338_disableChannel(
            &clockGenerator_generator, SI5338_ClockGenerator_Channel_3);
    }

    return (XStatus)errorCode;
}

/**
 * @}
 */

/**** END OF FILE ****/