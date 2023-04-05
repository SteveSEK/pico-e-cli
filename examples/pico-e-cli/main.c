#include <stdio.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "socket.h"
#include "w5x00_spi.h"
#include "w5x00_lwip.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"

#include "lwip/apps/lwiperf.h"
#include "lwip/etharp.h"

#include "picocli.h"

#define PLL_SYS_KHZ (133 * 1000)
//#define PLL_SYS_KHZ (144 * 1000)

#define SOCKET_MACRAW 0
#define PORT_LWIPERF 5001

extern uint8_t mac[6];
static ip_addr_t g_ip;
static ip_addr_t g_mask;
static ip_addr_t g_gateway;


struct netif g_netif2;
lwiperf_report_fn fn2;

wiz_PhyConf gPhyConf2 = {.by = PHY_CONFBY_SW,
                        .mode = PHY_MODE_MANUAL,
                        .speed = PHY_SPEED_100,
                        .duplex = PHY_DUPLEX_FULL};


static void set_clock_khz(void);

#if 1
int main_xx()
{
    /* Initialize */
    int8_t retval = 0;
    uint8_t *pack = malloc(ETHERNET_MTU);
    uint16_t pack_len = 0;
    struct pbuf *p = NULL;

    // Initialize network configuration
    IP4_ADDR(&g_ip, 192, 168, 10, 201);
    IP4_ADDR(&g_mask, 255, 255, 255, 0);
    IP4_ADDR(&g_gateway, 192, 168, 10, 1);

    set_clock_khz();

    // Initialize stdio after the clock change
    stdio_init_all();

    sleep_ms(1000 * 3); // wait for 3 seconds

    //wizchip_spi_initialize((PLL_SYS_KHZ / 4) * 1000);
    wizchip_spi_initialize((PLL_SYS_KHZ / 4) * 500);
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    // Set ethernet chip MAC address
    setSHAR(mac);
    ctlwizchip(CW_SET_PHYCONF, &gPhyConf2);
    ctlwizchip(CW_RESET_PHY, 0);

    // Initialize LWIP in NO_SYS mode
    lwip_init();

    netif_add(&g_netif2, &g_ip, &g_mask, &g_gateway, NULL, netif_initialize, netif_input);
    g_netif2.name[0] = 'e';
    g_netif2.name[1] = '0';

    // Assign callbacks for link and status
    netif_set_link_callback(&g_netif2, netif_link_callback);
    netif_set_status_callback(&g_netif2, netif_status_callback);

    // MACRAW socket open
    retval = socket(SOCKET_MACRAW, Sn_MR_MACRAW, PORT_LWIPERF, 0x00);

    if (retval < 0)
        printf(" MACRAW socket open failed\n");

    // Set the default interface and bring it up
    netif_set_link_up(&g_netif2);
    netif_set_up(&g_netif2);

    // Start lwiperf server
    lwiperf_start_tcp_server_default(fn2, NULL);

    /* Infinite loop */
    while (1)
    {
        getsockopt(SOCKET_MACRAW, SO_RECVBUF, &pack_len);
        if (pack_len > 0)
        {
            pack_len = recv_lwip(SOCKET_MACRAW, (uint8_t *)pack, pack_len);
            if (pack_len)
            {
                p = pbuf_alloc(PBUF_RAW, pack_len, PBUF_POOL);
                pbuf_take(p, pack, pack_len);
            }
            else
                printf(" No packet received\n");

            if (pack_len && p != NULL)
            {
                LINK_STATS_INC(link.recv);
                if (g_netif2.input(p, &g_netif2) != ERR_OK)
                    pbuf_free(p);
            }
        }
        /* Cyclic lwIP timers check */
        sys_check_timeouts();
    }
}
#endif

static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}


int main()
{
    set_clock_khz();
    stdio_init_all();

    sleep_ms(1000 * 3); // wait for 3 seconds

    picocli_startup(1);
    picocli_loop();

    return 0;
}