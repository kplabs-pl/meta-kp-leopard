/* Includes ---------------------------------------------------------------- */
#include "kp_zynqI2C.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "xiicps.h"
#include "xparameters.h"

/** @addtogroup ZynqI2C
 *  @{
 */

/* Constants and macros ---------------------------------------------------- */
// clang-format off

#define ZYNQ_I2C_SCLK_RATE                  50000u
#define ZYNQ_I2C_BUS_IDLE_WAIT_MAX_COUNT    10000u

// clang-format on

/* Private types ----------------------------------------------------------- */

/* Private variables ------------------------------------------------------- */

static XIicPs zynqI2C_device;

/* Private functions ------------------------------------------------------- */


/* Exposed API ------------------------------------------------------------- */

/**
 * @brief Initialize I2C device
 * @return SDK_ErrorCode_Success - success, SDK_ErrorCode_Internal - failure
 */
XStatus zynqI2C_init(void)
{
	XIicPs_Config* config = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
	if (config == NULL)
	{
		return XST_FAILURE;
	}

	XStatus status = XIicPs_CfgInitialize(&zynqI2C_device, config, config->BaseAddress);
	if (status != XST_SUCCESS)
	{
		return status;
	}

	status = XIicPs_SelfTest(&zynqI2C_device);
	if (status != XST_SUCCESS)
	{
		return status;
	}

	status = XIicPs_SetOptions(&zynqI2C_device, XIICPS_7_BIT_ADDR_OPTION);
	if (status != XST_SUCCESS)
	{
		return status;
	}

	status = XIicPs_SetSClk(&zynqI2C_device, ZYNQ_I2C_SCLK_RATE);
	if (status != XST_SUCCESS)
	{
		return status;
	}

	return XST_SUCCESS;
}

/**
 * @brief Write I2C device register
 * @param deviceAddr I2C device address
 * @param regAddr Register address
 * @param regValue Register value
 * @return SDK_ErrorCode_Success - success, SDK_ErrorCode_Internal - failure
 */
XStatus zynqI2C_writeReg(uint8_t deviceAddr, uint8_t regAddr, uint8_t regValue)
{
	XStatus errorCode = XST_FAILURE;
    uint16_t busIdleWaitCounter = 0;

    /* Wait until bus is idle. */
    while (XIicPs_BusIsBusy(&zynqI2C_device) == TRUE)
    {
        if (busIdleWaitCounter > ZYNQ_I2C_BUS_IDLE_WAIT_MAX_COUNT)
        {
            errorCode = XST_TIMEOUT;
            break;
        }
    }

    if (errorCode != XST_TIMEOUT)
    {
        uint8_t dataToWrite[] = {regAddr, regValue};
        XStatus status        = XIicPs_MasterSendPolled(
            &zynqI2C_device, dataToWrite, sizeof(dataToWrite), deviceAddr);
        if (status == XST_SUCCESS)
        {
            errorCode = XST_SUCCESS;
        }
    }

    return errorCode;
}

/**
 * @brief Read I2C device register
 * @param deviceAddr I2C device address
 * @param regAddr Register address
 * @param regValue Register value
 * @return SDK_ErrorCode_Success - success, SDK_ErrorCode_Failure - failure
 */
XStatus zynqI2C_readReg(uint8_t deviceAddr, uint8_t regAddr, uint8_t* regValue)
{
	XStatus errorCode = XST_FAILURE;

    uint16_t busIdleWaitCounter = 0;

    /* Wait until bus is idle. */
    while (XIicPs_BusIsBusy(&zynqI2C_device) == TRUE)
    {
        if (busIdleWaitCounter > ZYNQ_I2C_BUS_IDLE_WAIT_MAX_COUNT)
        {
            errorCode = XST_TIMEOUT;
            break;
        }
    }

    if (errorCode != XST_TIMEOUT)
    {
        XStatus status = XIicPs_MasterSendPolled(
            &zynqI2C_device, &regAddr, sizeof(regAddr), deviceAddr);
        if (status == XST_SUCCESS)
        {
            /* Wait until bus is idle. */
            busIdleWaitCounter = 0;
            while (XIicPs_BusIsBusy(&zynqI2C_device) == TRUE)
            {
                if (busIdleWaitCounter > ZYNQ_I2C_BUS_IDLE_WAIT_MAX_COUNT)
                {
                    errorCode = XST_TIMEOUT;
                    break;
                }
            }

            if (errorCode != XST_TIMEOUT)
            {
                status = XIicPs_MasterRecvPolled(
                    &zynqI2C_device, regValue, sizeof(*regValue), deviceAddr);
                if (status == XST_SUCCESS)
                {
                    errorCode = XST_SUCCESS;
                }
            }
        }
    }

    return errorCode;
}


/** @}
 */

/**** END OF FILE ****/