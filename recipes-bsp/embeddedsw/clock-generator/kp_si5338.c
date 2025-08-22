/* Includes ---------------------------------------------------------------- */

#include <sleep.h>

#include "kp_si5338.h"
#include "kp_zynqI2C.h"

// clang-format off

// clang-format on

/** @addtogroup si5338
 *  @{
 */

/* Constants and macros ---------------------------------------------------- */
// clang-format off

#define SI5338_DIS_LOL_REG_ADDR                 241u
#define SI5338_DIS_LOL_SET_MASK                 0xE5
#define SI5338_DIS_LOL_RESET_MASK               0x65

#define SI5338_SOFT_RESET_REG_ADDR              246u
#define SI5338_SOFT_RESET_MASK                  0x02

#define SI5338_FCAL_OVRD_EN_REG_ADDR            49u
#define SI5338_FCAL_OVRD_EN_MASK                0x80

#define SI5338_FCAL1_REG_ADDR                   235u
#define SI5338_FCAL2_REG_ADDR                   236u
#define SI5338_FCAL3_REG_ADDR                   237u

#define SI5338_ACTIVE1_REG_ADDR                 45u
#define SI5338_ACTIVE2_REG_ADDR                 46u
#define SI5338_ACTIVE3_REG_ADDR                 47u

#define SI5338_LOS_REG_ADDR                     218u
#define SI5338_LOS_MASK                         0x04
#define SI5338_LOCK_MASK                        0x15

#define SI5338_AFTER_CONFIGURATION_DELAY_MS     25u

#define SI5338_OUTPUT_ENABLE_REG_ADDR           230u
#define SI5338_OUTPUT_ENABLE_CHANNEL0_DIS_MASK  0x01
#define SI5338_OUTPUT_ENABLE_CHANNEL1_DIS_MASK  0x02
#define SI5338_OUTPUT_ENABLE_CHANNEL2_DIS_MASK  0x04
#define SI5338_OUTPUT_ENABLE_CHANNEL3_DIS_MASK  0x08
#define SI5338_OUTPUT_ENABLE_MASTER_DIS_MASK    0x0A

// clang-format on

/* Private types ----------------------------------------------------------- */


/* Private variables ------------------------------------------------------- */

static const uint8_t si5338_channelDisableMaskTable[] = {
    SI5338_OUTPUT_ENABLE_CHANNEL0_DIS_MASK,
    SI5338_OUTPUT_ENABLE_CHANNEL1_DIS_MASK,
    SI5338_OUTPUT_ENABLE_CHANNEL2_DIS_MASK,
    SI5338_OUTPUT_ENABLE_CHANNEL3_DIS_MASK};


/* Private functions ------------------------------------------------------- */

static XStatus si5338_registerWrite(
    const SI5338_ClockGenerator_t* const clockGenerator,
    uint8_t regAddress,
    uint8_t regValue)
{
    XStatus errorCode =
        zynqI2C_writeReg(clockGenerator->i2cAddress, regAddress, regValue);

    return errorCode;
}

static XStatus si5338_registerRead(
    const SI5338_ClockGenerator_t* const clockGenerator,
    uint8_t regAddress,
    uint8_t* const regValue)
{
    XStatus errorCode =
        zynqI2C_readReg(clockGenerator->i2cAddress, regAddress, regValue);

    return errorCode;
}

static XStatus si5338_loadRegistersConfig(SI5338_ClockGenerator_t* const clockGenerator)
{
    XStatus errorCode = XST_SUCCESS;
    for (uint16_t i = 0; i < clockGenerator->configRegNum; i++)
    {
        SI5338_ClockGenerator_RegData_t current = clockGenerator->configRegData[i];

        if (current.mask != 0x00)
        {
            if (current.mask == 0xFF)
            {
                /* Do a write transaction only */
                errorCode =
                    si5338_registerWrite(clockGenerator, current.address, current.value);
            }
            else
            {
                /* Do a read-modify-write */
                uint8_t currentChipValue;
                errorCode = si5338_registerRead(
                    clockGenerator, current.address, &currentChipValue);

                if (errorCode == XST_SUCCESS)
                {
                    uint8_t clearCurrentValue = currentChipValue & (~current.mask);
                    uint8_t clearNewValue     = current.value & current.mask;
                    uint8_t combined          = clearNewValue | clearCurrentValue;
                    errorCode =
                        si5338_registerWrite(clockGenerator, current.address, combined);
                }
            }
        }

        if (errorCode != XST_SUCCESS)
        {
            xil_printf(
                "Configuration loading failed [%u, %u]\n", current.address, current.value);
            break;
        }
    }

    return errorCode;
}

static uint8_t si5338_channelDisableMask(SI5338_ClockGenerator_Channel_t channel)
{
    uint8_t channelDisableMask = 0;
    if (channel < sizeof(si5338_channelDisableMaskTable))
    {
        channelDisableMask = si5338_channelDisableMaskTable[channel];
    }

    return channelDisableMask;
}


/* Exposed API ------------------------------------------------------------- */

/**
 * @brief Load config and initialize Si5338 clock generator
 * @param clockGenerator Pointer to clock generator structure
 * @return XST_SUCCESS - success, errorCode - failure
 */
XStatus si5338_loadConfiguration(SI5338_ClockGenerator_t* const clockGenerator)
{
    XStatus errorCode = XST_FAILURE;

    do
    {
        /* Disable outputs */
        errorCode = si5338_disableAll(clockGenerator);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        /* Pause LOL */
        errorCode = si5338_registerWrite(
            clockGenerator, SI5338_DIS_LOL_REG_ADDR, SI5338_DIS_LOL_SET_MASK);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        /* Load registers configuration */
        errorCode = si5338_loadRegistersConfig(clockGenerator);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        /* Check LOS alarm for the xtal input */
        uint8_t regValue = UINT8_MAX;
        do
        {
            errorCode = si5338_registerRead(clockGenerator, SI5338_LOS_REG_ADDR, &regValue);
            if (errorCode != XST_SUCCESS)
            {
                break;
            }

            regValue &= SI5338_LOS_MASK;

        } while (regValue != 0);

        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        /* Configure PLL for locking: FCAL_OVRD_EN = 0 */
        errorCode =
            si5338_registerRead(clockGenerator, SI5338_FCAL_OVRD_EN_REG_ADDR, &regValue);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        errorCode = si5338_registerWrite(
            clockGenerator,
            SI5338_FCAL_OVRD_EN_REG_ADDR,
            regValue & (~SI5338_FCAL_OVRD_EN_MASK));
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        /* Initiate Locking of PLL - soft reset */
        errorCode = si5338_registerWrite(
            clockGenerator, SI5338_SOFT_RESET_REG_ADDR, SI5338_SOFT_RESET_MASK);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        /* Restart LOL: DIS_LOL = 0 */
        errorCode = si5338_registerWrite(
            clockGenerator, SI5338_DIS_LOL_REG_ADDR, SI5338_DIS_LOL_RESET_MASK);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        /* Wait for Si5338 to be ready after calibration */
        usleep(SI5338_AFTER_CONFIGURATION_DELAY_MS * 1000u);

        do
        {
            /* Make sure the device locked by checking PLL_LOL and SYS_CAL */
            errorCode = si5338_registerRead(clockGenerator, SI5338_LOS_REG_ADDR, &regValue);
            if (errorCode != XST_SUCCESS)
            {
                break;
            }

            regValue &= SI5338_LOCK_MASK;

        } while (regValue != 0);

        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        /* Copy FCAL values to active registers */
        errorCode = si5338_registerRead(clockGenerator, SI5338_FCAL1_REG_ADDR, &regValue);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        errorCode = si5338_registerWrite(clockGenerator, SI5338_ACTIVE1_REG_ADDR, regValue);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        errorCode = si5338_registerRead(clockGenerator, SI5338_FCAL2_REG_ADDR, &regValue);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        errorCode = si5338_registerWrite(clockGenerator, SI5338_ACTIVE2_REG_ADDR, regValue);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        /* Clear bits 0 and 1 from 47 and combine with bits 0 and 1 from 237 */
        errorCode = si5338_registerRead(clockGenerator, SI5338_ACTIVE3_REG_ADDR, &regValue);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        uint8_t regValue2 = 255;
        errorCode = si5338_registerRead(clockGenerator, SI5338_FCAL3_REG_ADDR, &regValue2);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        errorCode = si5338_registerWrite(
            clockGenerator,
            SI5338_ACTIVE3_REG_ADDR,
            ((regValue & 0xFC) | (regValue2 & 0x03)));
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        /* Set PLL to use FCAL values */
        errorCode =
            si5338_registerRead(clockGenerator, SI5338_FCAL_OVRD_EN_REG_ADDR, &regValue);
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

        errorCode = si5338_registerWrite(
            clockGenerator,
            SI5338_FCAL_OVRD_EN_REG_ADDR,
            (regValue | SI5338_FCAL_OVRD_EN_MASK));
        if (errorCode != XST_SUCCESS)
        {
            break;
        }

    } while (0);

    return errorCode;
}

/**
 * @brief Enable all si5338 outputs and master output enable
 * @param clockGenerator Pointer to clock generator structure
 * @return XST_SUCCESS - success, errorCode - failure
 */
XStatus si5338_enableAll(SI5338_ClockGenerator_t* const clockGenerator)
{
    /* Enable Outputs master switch */
    return si5338_registerWrite(clockGenerator, SI5338_OUTPUT_ENABLE_REG_ADDR, 0x00);
}

/**
 * @brief Disable all si5338 outputs and master output enable
 * @param clockGenerator Pointer to clock generator structure
 * @return XST_SUCCESS - success, errorCode - failure
 */
XStatus si5338_disableAll(SI5338_ClockGenerator_t* const clockGenerator)
{
    /* Disable outputs master switch */
    return si5338_registerWrite(
        clockGenerator,
        SI5338_OUTPUT_ENABLE_REG_ADDR,
        (SI5338_OUTPUT_ENABLE_CHANNEL0_DIS_MASK | SI5338_OUTPUT_ENABLE_CHANNEL1_DIS_MASK
         | SI5338_OUTPUT_ENABLE_CHANNEL2_DIS_MASK | SI5338_OUTPUT_ENABLE_CHANNEL3_DIS_MASK
         | SI5338_OUTPUT_ENABLE_MASTER_DIS_MASK));
}

/**
 * @brief Enable master output enable with no changing of each channel output enable state
 * value, output will be enable on channels depending of their output enable state value
 * @param clockGenerator Pointer to clock generator structure
 * @return XST_SUCCESS - success, errorCode - failure
 */
XStatus si5338_enableMasterOutputEnable(SI5338_ClockGenerator_t* const clockGenerator)
{
    uint8_t regValue;
    XStatus errorCode =
        si5338_registerRead(clockGenerator, SI5338_OUTPUT_ENABLE_REG_ADDR, &regValue);
    if (errorCode == XST_SUCCESS)
    {
        errorCode = si5338_registerWrite(
            clockGenerator,
            SI5338_OUTPUT_ENABLE_REG_ADDR,
            (regValue & (~SI5338_OUTPUT_ENABLE_MASTER_DIS_MASK)));
    }

    return errorCode;
}

/**
 * @brief Disable master output enable with no changing of each channel output enable
 * state value, output on all channels will be disabled
 * @param clockGenerator Pointer to clock generator structure
 * @return XST_SUCCESS - success, errorCode - failure
 */
XStatus si5338_disableMasterOutputEnable(SI5338_ClockGenerator_t* const clockGenerator)
{
    uint8_t regValue;
    XStatus errorCode =
        si5338_registerRead(clockGenerator, SI5338_OUTPUT_ENABLE_REG_ADDR, &regValue);
    if (errorCode == XST_SUCCESS)
    {
        errorCode = si5338_registerWrite(
            clockGenerator,
            SI5338_OUTPUT_ENABLE_REG_ADDR,
            (regValue | SI5338_OUTPUT_ENABLE_MASTER_DIS_MASK));
    }

    return errorCode;
}

/**
 * @brief Set output enable of specified channel to on position,
 *        output on chosen channel will be enabled if master output enable is on
 * @param clockGenerator Pointer to clock generator structure
 * @param channel Channel to be enabled
 * @return XST_SUCCESS - success, errorCode - failure
 */
XStatus si5338_enableChannel(
    SI5338_ClockGenerator_t* const clockGenerator,
    SI5338_ClockGenerator_Channel_t channel)
{
    uint8_t regValue;
    XStatus errorCode =
        si5338_registerRead(clockGenerator, SI5338_OUTPUT_ENABLE_REG_ADDR, &regValue);
    if (errorCode == XST_SUCCESS)
    {
        uint8_t channelDisableMask = si5338_channelDisableMask(channel);
        errorCode                  = si5338_registerWrite(
            clockGenerator,
            SI5338_OUTPUT_ENABLE_REG_ADDR,
            (regValue & (~channelDisableMask)));
    }

    return errorCode;
}

/**
 * @brief Set output enable of specified channel to off position,
 *        output on chosen channel will be disabled in any situation
 * @param clockGenerator Pointer to clock generator structure
 * @param channel Channel to be disabled
 * @return XST_SUCCESS - success, errorCode - failure
 */
XStatus si5338_disableChannel(
    SI5338_ClockGenerator_t* const clockGenerator,
    SI5338_ClockGenerator_Channel_t channel)
{
    uint8_t regValue;
    XStatus errorCode =
        si5338_registerRead(clockGenerator, SI5338_OUTPUT_ENABLE_REG_ADDR, &regValue);
    if (errorCode == XST_SUCCESS)
    {
        uint8_t channelDisableMask = si5338_channelDisableMask(channel);
        errorCode                  = si5338_registerWrite(
            clockGenerator, SI5338_OUTPUT_ENABLE_REG_ADDR, (regValue | channelDisableMask));
    }

    return errorCode;
}

/**
 * @}
 */

/**** END OF FILE ****/