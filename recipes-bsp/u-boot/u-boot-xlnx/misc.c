// SPDX-License-Identifier: MIT
/*
 *  Copyright(c) 2025 KP Labs
 *
 *  Maintainer: Paweł Klisz <pklisz@kplabs.pl>
 */

#include <net.h>
#include <misc.h>
#include <env.h>
#include <asm/gpio.h>

enum pn_id_result
{
    PnId1 = 1,
    PnId2 = 2
};

static int _configure_pn_id_gpio(int pn_id_pin)
{
    if (gpio_request(pn_id_pin, "leopard-pn-id"))
    {
        printf("[Leopard] Cannot request pin\n");
        return -1;
    }

    if (gpio_direction_input(pn_id_pin))
    {
        printf("[Leopard] Cannot set pin direction\n");
        return -1;
    }

    return 0;
}

static enum pn_id_result _get_board_pn_id(int pn_id_pin)
{
    return gpio_get_value(pn_id_pin) == 0 ? PnId1 : PnId2;
}

static int _set_mac_address(enum pn_id_result pn_id)
{
    unsigned char enetaddr[6] = {0x02, 0xc0, 0xf1, 0xd7, 0xfa, 0x00};

    if (pn_id == PnId1)
    {
        enetaddr[5] = 0x31;
    }
    else
    {
        enetaddr[5] = 0x32;
    }

    printf("[Leopard] Setting MAC address to ");
    for (int i = 0; i < sizeof(enetaddr); i++)
    {
        printf("%02x", enetaddr[i]);
        if (i < sizeof(enetaddr) - 1)
        {
            printf(":");
        }
    }
    printf("\n");

    return eth_env_set_enetaddr("ethaddr", enetaddr);
}

static int _set_pn_id_env(enum pn_id_result pn_id)
{
    char buff[2];
    sprintf(buff, "%d", pn_id);
    return env_set("leopard_pn_id", buff);
}

static int _main()
{
    int pn_id_pin = 30;

    if (_configure_pn_id_gpio(pn_id_pin))
    {
        return -1;
    }

    enum pn_id_result pn_id = _get_board_pn_id(pn_id_pin);
    printf("[Leopard] Running on PN%d\n", pn_id);

    if (_set_pn_id_env(pn_id))
    {
        printf("[Leopard] Cannot set PN ID\n");
        return -1;
    }

    if (_set_mac_address(pn_id))
    {
        printf("[Leopard] Cannot set MAC address\n");
        return -1;
    }

    return 0;
}

/**
 * misc_init_r - Configure board specific parts
 *
 * This function is called after relocation and can be used to
 * set up board specific parts (see common/board_r.c)
 *
 * @return operation status
 */
int misc_init_r(void)
{
    if (_main())
    {
        printf("[Leopard] Error: Cannot configure MAC address, u-boot will pick a random one by itself\n");
    }

    return 0;
}
