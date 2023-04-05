#include <stdio.h>
#include <string.h>

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
#include "lwip/dhcp.h"

#include "pico/wiznet_spi_pio.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PLL_SYS_KHZ (133 * 1000)

#define SOCKET_MACRAW 0
#define PORT_LWIPERF 5001

struct netif g_netif;
lwiperf_report_fn fn;

static struct dhcp g_dhcp_client;


/* WIZ5X00 PHY configuration for link speed 100MHz */
wiz_PhyConf gPhyConf = {.by = PHY_CONFBY_SW,
                        .mode = PHY_MODE_MANUAL,
                        .speed = PHY_SPEED_100,
                        .duplex = PHY_DUPLEX_FULL};


wiznet_spi_config_t g_spi_config = {
    .data_in_pin = PIN_MISO,
    .data_out_pin = PIN_MOSI,
    .cs_pin = PIN_CS,
    .clock_pin = PIN_SCK,
    .irq_pin = 21u, // sekim XXX IRQ?
    .reset_pin = PIN_RST,
    .clock_div_major = 4,
    .clock_div_minor = 0,
};

uint32_t g_clock_div;


void dump_bytes(const uint8_t *bptr, uint32_t len)
{
    unsigned int i = 0;
    for (i = 0; i < len;) {
        if ((i & 0x0f) == 0) {
            printf("\n");
        } else if ((i & 0x07) == 0) {
            printf(" ");
        }
        printf("%02x ", bptr[i++]);
    }
    printf("\n");
}

static int wiznet_pio_init(void)
{
    wiznet_spi_handle_t spi_handle;
    // spi_handle = wiznet_spi_pio_open(wiznet_default_spi_config());
    g_spi_config.clock_div_major = g_clock_div;
    spi_handle = wiznet_spi_pio_open(&g_spi_config);
    
    (*spi_handle)->reset(spi_handle);
    (*spi_handle)->set_active(spi_handle);

    reg_wizchip_spi_cbfunc((*spi_handle)->read_byte, (*spi_handle)->write_byte);
    reg_wizchip_spiburst_cbfunc((*spi_handle)->read_buffer, (*spi_handle)->write_buffer);
    reg_wizchip_cs_cbfunc((*spi_handle)->frame_start, (*spi_handle)->frame_end);

    // Check the version
    (*spi_handle)->frame_start();
    uint8_t version = getVER();
    (*spi_handle)->frame_end();

    if (version != 0x51)
    {
        panic("Failed to detect wiznet 0x%02x\n", version);
        (*spi_handle)->set_inactive();
        return -1;
    }

    // sekim XXXX ????
    // Configure buffers for fast MACRAW
    uint8_t sn_size[_WIZCHIP_SOCK_NUM_ * 2] = {0};
    sn_size[0] = _WIZCHIP_SOCK_NUM_ * 2;
    sn_size[_WIZCHIP_SOCK_NUM_] = _WIZCHIP_SOCK_NUM_ * 2;
    ctlwizchip(CW_INIT_WIZCHIP, sn_size);

    return 0;
}

void cmd_w5x00_init(char* szSPImode, char* szClockDiv)
{
    g_clock_div = (uint32_t) strtoul(szClockDiv, NULL, 10);

    if ( strcmp(szSPImode, "spinormal")==0 )
    {
        printf("Using SPI to talk to wiznet\n");
        wizchip_spi_initialize((PLL_SYS_KHZ / g_clock_div) * 1000);
        wizchip_cris_initialize();
        wizchip_reset();
        wizchip_initialize();
    }
    else if ( strcmp(szSPImode, "spipio")==0 )
    {
        printf("Using PIO to talk to wiznet\n");
        if (wiznet_pio_init() != 0)
        {
            printf("Failed to start with pio\n");
            return;
        }
    }
    else
    {
        printf("param error(spinormal/spipio) \n");
        return;
    }

    ctlwizchip(CW_SET_PHYCONF, &gPhyConf);
    ctlwizchip(CW_RESET_PHY, 0);

    uint8_t mac[6]; 
    getSHAR(mac);
    assert((mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5]) != 0);
    printf("mac address %02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    wiz_PhyConf conf_in = {0};
    ctlwizchip(CW_GET_PHYCONF, &conf_in);
    printf("phy config mode: %s\n", conf_in.mode ? "auto" : "manual");
    printf("phy config speed: %s\n", conf_in.speed ? "100" : "10");
    printf("phy config duplex: %s\n", conf_in.duplex ? "full" : "half");    
}

void cmd_w5x00_spiclock(char* szClcok)
{
    uint32_t spi_clock = (uint32_t) strtoul(szClcok, NULL, 10);
    printf("spi_init return = %dHz\r\n", spi_init(SPI_PORT, spi_clock));
}

void cmd_w5x00_getver()
{
    uint8_t bufftemp[6] = {0,};
    bufftemp[0] = getVER();
    printf("Version(0x%08x) : %02x \n", VERR, bufftemp[0]);
}

void cmd_w5x00_getmac()
{
    uint8_t bufftemp[6] = {0,};
    getSHAR(bufftemp);
    printf("mac address(0x%08x) : %02x:%02x:%02x:%02x:%02x:%02x\n", SHAR, bufftemp[0], bufftemp[1], bufftemp[2], bufftemp[3], bufftemp[4], bufftemp[5]);
}

void cmd_w5x00_readbyte(char* param1)
{
    uint8_t bufftemp[6] = {0,};
    uint32_t addr = (uint32_t) strtoul(param1, NULL, 16);
    bufftemp[0] = WIZCHIP_READ(addr);
    printf("W5x00(0x%08x) : %02x \n", addr, bufftemp[0]);
}


int convertStringToByteBuffer(const char* str, unsigned char* buffer, size_t bufferLen)
{
    const char* p = str;
    unsigned int byte;
    int len = 0;

    while (*p && len < bufferLen)
    {
        char hex[3] = {*p, *(p+1), '\0'};
        if (hex[1]==0)  break;
        byte = strtoul(hex, NULL, 16);
        buffer[len] = (unsigned char)byte;
        p += 2;
        len++;
    }

    return len;
}

void cmd_w5x00_writebuff(char* param1, char* szData)
{
    uint8_t bufftemp[256] = {0,};
    uint32_t addr = (uint32_t) strtoul(param1, NULL, 16);
    uint32_t len;

    len = convertStringToByteBuffer(szData, bufftemp, sizeof(bufftemp));

    if ( len>sizeof(bufftemp) )
    {
        printf("cmd_w5x00_chipreadbuff : size error \n");
        return; 
    }
    
    WIZCHIP_WRITE_BUF(addr, bufftemp, len);
}

void cmd_w5x00_readbuff(char* param1, char* param2)
{
    uint8_t bufftemp[256] = {0,};
    uint32_t addr = (uint32_t) strtoul(param1, NULL, 16);
    uint32_t len = (uint32_t) strtoul(param2, NULL, 10);

    if ( len>sizeof(bufftemp) )
    {
        printf("cmd_w5x00_chipreadbuff : size error \n");
        return; 
    }
    
    WIZCHIP_READ_BUF(addr, bufftemp, len);
    printf("W5x00(0x%08x) : ", addr);
    dump_bytes(bufftemp, len);
}

// 주로 문제가 되는 시나리오 : Socket 버퍼에 수신 데이터가 왔을 때
/*
int32_t recv_lwip(uint8_t sn, uint8_t *buf, uint16_t len)
{
    ...........
    wiz_recv_data(sn, head, 2); // WIZCHIP_READ_BUF
    ...........
    wiz_recv_data(sn, buf, pack_len); // WIZCHIP_READ_BUF
    ...........
}
*/
void cmd_w5x00_readbuffdelay(char* param1, char* param2, char* param3)
{
    uint8_t bufftemp[256] = {0,};
    uint32_t addr = (uint32_t) strtoul(param1, NULL, 16);
    uint32_t len = (uint32_t) strtoul(param2, NULL, 10);
    uint32_t delay = (uint32_t) strtoul(param3, NULL, 10);

    if ( len>sizeof(bufftemp) )
    {
        printf("cmd_w5x00_chipreadbuff : size error \n");
        return; 
    }

    //  The below works similarly to WIZCHIP_READ_BUF()
    {
        uint8_t spi_data[3];
        uint16_t i = 0;
        WIZCHIP_CRITICAL_ENTER();
        WIZCHIP.CS._select();   //M20150601 : Moved here.

		spi_data[0] = 0x0F;
		spi_data[1] = (uint16_t)((addr+i) & 0xFF00) >>  8;
		spi_data[2] = (uint16_t)((addr+i) & 0x00FF) >>  0;
		WIZCHIP.IF.SPI._write_burst(spi_data, 3);

        for(uint32_t ii=0;ii<delay;ii++)    { __asm volatile ("nop"); }

        WIZCHIP.IF.SPI._read_burst(bufftemp, len);

        WIZCHIP.CS._deselect();    //M20150601 : Moved Here.
        WIZCHIP_CRITICAL_EXIT();  
    }

    printf("W5x00(0x%08x) : ", addr);
    dump_bytes(bufftemp, len);
}


ip_addr_t temp_ip;
ip_addr_t temp_mask;
ip_addr_t temp_gateway;

void cmd_w5x00_lwipinit(char* szIp, char* szMask, char* szGateway)
{
    int dhcp_mode = 1;

    if ( szIp && szMask && szGateway )
    {
        if ( strlen(szIp)>0 && strlen(szMask)>0 && strlen(szGateway)>0 )
        {
            if ( ip4addr_aton(szIp,      &temp_ip)==0 )      {  printf("cmd_w5x00_lwipinit : Invalid IP address 1 \n");  return;   }
            if ( ip4addr_aton(szMask,    &temp_mask)==0 )    {  printf("cmd_w5x00_lwipinit : Invalid IP address 2 \n");  return;   }
            if ( ip4addr_aton(szGateway, &temp_gateway)==0 ) {  printf("cmd_w5x00_lwipinit : Invalid IP address 3 \n");  return;   }
            dhcp_mode = 0;
        }
    }

    lwip_init();

    netif_add(&g_netif, &temp_ip, &temp_mask, &temp_gateway, NULL, netif_initialize, netif_input);
    g_netif.name[0] = 'e';
    g_netif.name[1] = '0';

    // Assign callbacks for link and status
    netif_set_link_callback(&g_netif, netif_link_callback);
    netif_set_status_callback(&g_netif, netif_status_callback);

    int retval = socket(SOCKET_MACRAW, Sn_MR_MACRAW, PORT_LWIPERF, 0x00);

    if (retval < 0)
    {
        printf(" MACRAW socket open failed\n");
        return;
    }

    netif_set_link_up(&g_netif);
    netif_set_up(&g_netif);

    if ( dhcp_mode==1 )
    {
        printf("DHCP Started\n");
        dhcp_set_struct(&g_netif, &g_dhcp_client);
        dhcp_start(&g_netif);
    }
}

void poll_w5x00macraw(int check_linkup)
{
    uint8_t pack[ETHERNET_MTU];
    uint16_t pack_len = 0;
    struct pbuf *p = NULL;

    if ( check_linkup==1 )
    {
        if ( !netif_is_link_up(&g_netif) )
        {
            return;
        }
    }

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
            if (g_netif.input(p, &g_netif) != ERR_OK)
                pbuf_free(p);
        }
    }
    sys_check_timeouts();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int8_t g_iperf_mode = 0;
void cmd_w5x00_iperf(char* param1, char* param2)
{
    lwiperf_start_tcp_server_default(fn, NULL);

    int8_t retval = 0;
    uint8_t *pack = malloc(ETHERNET_MTU);
    uint16_t pack_len = 0;
    struct pbuf *p = NULL;

    g_iperf_mode = 1;    

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
                if (g_netif.input(p, &g_netif) != ERR_OK)
                    pbuf_free(p);
            }
        }
        sys_check_timeouts();

        if ( g_iperf_mode==0 )         break;
    }
}



uint32_t g_pio_hookinstruction = 0;
void cmd_w5x00_piohook(char* param1)
{
    g_pio_hookinstruction = (uint32_t) strtoul(param1, NULL, 16);
    printf("Changed to 0x%04x \n", g_pio_hookinstruction);
    return; 
}

void w5x00_command(int argc, char *argv[])
{
    if      ( strcmp(argv[1], "init")==0 )          {   cmd_w5x00_init(argv[2], argv[3]);       }

    else if ( strcmp(argv[1], "spiclock")==0 )      {   cmd_w5x00_spiclock(argv[2]);    }
    else if ( strcmp(argv[1], "getver")==0 )        {   cmd_w5x00_getver();     }
    else if ( strcmp(argv[1], "getmac")==0 )        {   cmd_w5x00_getmac();     }
    else if ( strcmp(argv[1], "writebuff")==0 )     {   cmd_w5x00_writebuff(argv[2], argv[3]);  }
    else if ( strcmp(argv[1], "readbyte")==0 )      {   cmd_w5x00_readbyte(argv[2]);  }
    else if ( strcmp(argv[1], "readbuff")==0 )      {   cmd_w5x00_readbuff(argv[2], argv[3]);  }
    else if ( strcmp(argv[1], "readbuffdelay")==0 ) {   cmd_w5x00_readbuffdelay(argv[2], argv[3], argv[4]);  }


    else if ( strcmp(argv[1], "iperf")==0 )         {   cmd_w5x00_iperf(argv[2], argv[3]);  }


    else if ( strcmp(argv[1], "piohook")==0 )       {   cmd_w5x00_piohook(argv[2]);  }


    else if ( strcmp(argv[1], "lwipinit")==0 )      {   cmd_w5x00_lwipinit(argv[2], argv[3], argv[4]);  }
}


