#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "port_common.h"

#include "hardware/pll.h" // don't forget to add hardware_pll to your Cmakelists.txt

void cmd_system_reset()
{
    extern void watchdog_reboot(uint32_t pc, uint32_t sp, uint32_t delay_ms);
    watchdog_reboot(0,0,0);
}



void gset_sys_clock_pll(uint32_t vco_freq, uint post_div1, uint post_div2) 
{
    if (!running_on_fpga()) 
    {
        clock_configure(clk_sys,
                        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
                        48 * MHZ,
                        48 * MHZ);

        pll_init(pll_sys, 1, vco_freq, post_div1, post_div2);
        uint32_t freq = vco_freq / (post_div1 * post_div2);

        // Configure clocks
        // CLK_REF = XOSC (12MHz) / 1 = 12MHz
        clock_configure(clk_ref,
                        CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC,
                        0, // No aux mux
                        12 * MHZ,
                        12 * MHZ);

        // CLK SYS = PLL SYS (125MHz) / 1 = 125MHz
        clock_configure(clk_sys,
                        CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX,
                        CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS,
                        freq, freq);

    //    clock_configure(clk_peri,
    //                    0, // Only AUX mux on ADC
    //                    CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_USB,
    //                    48 * MHZ,
    //                    48 * MHZ);
    }
}

static inline bool gset_sys_clock_khz(uint32_t freq_khz, bool required) 
{
    uint vco, postdiv1, postdiv2;
    if (check_sys_clock_khz(freq_khz, &vco, &postdiv1, &postdiv2)) 
    {
        gset_sys_clock_pll(vco, postdiv1, postdiv2);
        return true;
    } 
    else if (required) 
    {
        panic("System clock of %u kHz cannot be exactly achieved", freq_khz);
    }
    return false;
}


// sekim XXX cmd_system_clock not working?
void cmd_system_clock(char* param1)
{
    int nDev = (uint32_t) strtoul(param1, NULL, 10);
    if (nDev<=0)
    {
        //printf("cmd_system_clock error %d \n", nDev);
        printf("clk_sys = %ukHz\n", frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS));
        return;
    }
    //set_sys_clock_khz(nDev, true); // default 125000?
    gset_sys_clock_khz(nDev, false);
}



void system_command(int argc, char *argv[])
{
    if      ( strcmp(argv[1], "reset")==0 )         {   cmd_system_reset();    }
    else if ( strcmp(argv[1], "clock")==0 )         {   cmd_system_clock(argv[2]);    }
}