Raspberry Pi Pico Ethernet Test using CLI 
======================================

### Overview
This repository is intended to perform various tests using the CLI in the comnination of RP2040 and W5100S.

### Quick Start Firmware
* You can start this quickly with this compiled files.
  - **[Firmware download](build/examples/pico-e-cli/pico-e-cli.uf2)**(UF2 file 200KB)
  
### SPI Configuration
You can use the normal SPI method or RP2040 PIO method.   
Please note that the clock setting variables for the SPI normal method and the PIO method are different. 
Also, when resetting the SPI, it should be done after initialization (using "system reset" command).

The following is the normal SPI setting screen that does not use PIO.

```
>> w5x00 init spinormal 4
Using SPI to talk to wiznet
spi_init return = 33250000Hz
mac address 00:08:dc:12:34:56
phy config mode: manual
phy config speed: 100
phy config duplex: full
>> 
```
```
>> w5x00 init spinormal 8
Using SPI to talk to wiznet
spi_init return = 16625000Hz
mac address 00:08:dc:12:34:56
phy config mode: manual
phy config speed: 100
phy config duplex: full
>> 
```
The following screen shows SPI PIO being used and the clock being set to 16.6MHz.
```
>> w5x00 init spipio 4
Using PIO to talk to wiznet
PIO Instruction(0x0000) : 6001 1040 e000 e180 f026 4001 1045 4001 0083 
mac address 00:08:dc:12:34:56
phy config mode: manual
phy config speed: 100
phy config duplex: full
>> 
```

### PIO Code Hook
To change the PIO Code during running, I added ***wiznet_spi_pio.pio.static.h*** file and changed some code.
* wiznet_spi_pio.pio.static.h
```CPP
//static const uint16_t wiznet_spi_write_read_program_instructions[] = {
uint16_t wiznet_spi_write_read_program_instructions[] = {
            //     .wrap_target
    0x6001, //  0: out    pins, 1         side 0     
    0x1040, //  1: jmp    x--, 0          side 1     
    0xe000, //  2: set    pins, 0         side 0     
    0xe180, //  3: set    pindirs, 0      side 0 [1] 
    0xf026, //  4: set    x, 6            side 1     
    0x4001, //  5: in     pins, 1         side 0     
    0x1045, //  6: jmp    x--, 5          side 1     
    0x4001, //  7: in     pins, 1         side 0     
    0x0083, //  8: jmp    y--, 3          side 0     
            //     .wrap
};
```
The 0xe180 part of the wiznet_spi_write_read_program_instructions is related to the PIO Delay, and this can be modified with the "w5x00 piohook" command. The Instruction Code and Delay related parts were referenced from the RP2040 data sheet.

```
>> w5x00 piohook e080 // set    pindirs, 0      side 0 [0] 
..........
>> w5x00 piohook e180 // set    pindirs, 0      side 0 [1] 
..........
>> w5x00 piohook e480 // set    pindirs, 0      side 0 [4] 
..........
```
![image](https://user-images.githubusercontent.com/2126804/230025606-4772484b-5868-4cad-ad44-138823cb310d.png)

### SPI Data Ready Time
The following is about the Data Ready Time of W5x00, but when using the IoLibrary, a sufficient delay time is typically established.
![image](https://user-images.githubusercontent.com/2126804/230268105-cda04a0a-f41f-4940-8479-c4fb21245c6b.png)

### Test Function
Below is the function used in the test. To test the delay (nop) between the (A) and (B) operations in the W5x00 chip, an option to adjust the delay was added.
* testcommand.c
```CPP
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
        WIZCHIP.IF.SPI._write_burst(spi_data, 3); // ===> (A)

        for(uint32_t ii=0;ii<delay;ii++)    { __asm volatile ("nop"); }

        WIZCHIP.IF.SPI._read_burst(bufftemp, len); // ===> (B)

        WIZCHIP.CS._deselect();    //M20150601 : Moved Here.
        WIZCHIP_CRITICAL_EXIT();  
    }

    printf("W5x00(0x%08x) : ", addr);
    dump_bytes(bufftemp, len);
}

```

### SPI Normal(not PIO), SPI Clock 16.6MHz, SPI R/W Delay 100
![image](https://user-images.githubusercontent.com/2126804/230020530-8e207293-7fc0-4b39-bd98-64ff3faafac8.png)
![image](https://user-images.githubusercontent.com/2126804/230020923-f6efa3d6-4626-4a27-b4ee-276c17877912.png)

### SPI PIO, SPI Clock 16.6MHz, SPI R/W Delay 100, PIO Delay 0 (0xe080)
![image](https://user-images.githubusercontent.com/2126804/230314287-780fce07-b201-439b-9c12-65650ca79a31.png)

### SPI PIO, SPI Clock 16.6MHz, SPI R/W Delay 100, PIO Delay 4 (0xe480)
![image](https://user-images.githubusercontent.com/2126804/230314349-cdfd8edb-5c70-433d-b489-4ef234f8bfa6.png)

### Test Example
Below is the log of reading the internal registers of the W5x00 chip and conducting an iPerf Throughput test using Normal SPI at 32MHz.
```
#################### Pico Terminal ####################
>> 
>> w5x00 init spinormal 4
Using SPI to talk to wiznet
spi_init return = 33250000Hz
mac address 00:08:dc:55:66:77
phy config mode: manual
phy config speed: 100
phy config duplex: full
>> w5x00 getver
Version(0x00000080) : 51 
>> w5x00 getmac
mac address(0x00000009) : 00:08:dc:55:66:77
>> w5x00 readbuff 0 256
W5x00(0x00000000) : 
03 00 00 00 00 00 00 00  00 00 08 dc 55 66 77 00 
00 00 00 00 00 00 00 07  d0 07 03 03 00 00 00 00 
00 00 00 00 00 00 00 00  28 00 00 00 00 00 00 00 
40 00 00 00 00 00 00 00  00 00 ff ff 01 81 0a 00 
a1 00 21 00 00 01 00 40  00 00 00 00 00 07 d0 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 0c 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
51 00 71 ca 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
>> w5x00 lwipinit 192.168.10.201 255.255.255.0 192.168.10.1
netif link status changed up
netif status changed 192.168.10.201
>> w5x00 iperf
>> 
```
![image](https://user-images.githubusercontent.com/2126804/230248436-cc7e97f1-7fb7-4472-b074-6f287fef17d8.png)
```
>> w5x00 init spipio 2
Using PIO to talk to wiznet
PIO Instruction(0x0000) : 6001 1040 e000 e180 f026 4001 1045 4001 0083 
mac address 00:08:dc:55:66:77
phy config mode: manual
phy config speed: 100
phy config duplex: full
>> w5x00 readbuff 09 6
W5x00(0x00000009) : 
00 08 dc 55 66 77 
>> w5x00 readbuff 0 256
W5x00(0x00000000) : 
03 00 00 00 00 00 00 00  00 00 08 dc 55 66 77 00 
00 00 00 00 00 00 00 07  d0 07 03 03 00 00 00 00 
00 00 00 00 00 00 00 00  28 00 00 00 00 00 00 00 
40 00 00 00 00 00 00 00  00 00 ff ff 01 81 0a 00 
a1 00 21 00 00 01 00 40  00 00 00 00 00 07 d0 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 0c 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
51 00 85 5b 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00 
>> w5x00 lwipinit 192.168.10.201 255.255.255.0 192.168.10.1
netif link status changed up
netif status changed 192.168.10.201
>> w5x00 iperf
>> 
```

### Misc CLI Command
You can conduct additional tests using the following commands.
```
system reset

w5x00 init spinormal 4
w5x00 init spinormal 8
w5x00 init spinormal 128

w5x00 piohook e080
w5x00 piohook e180
w5x00 piohook e480
w5x00 init spipio 2
w5x00 init spipio 4
w5x00 init spipio 8

w5x00 readbuff 09 6
w5x00 readbuffdelay 09 6 0
w5x00 readbuffdelay 09 6 100

w5x00 writebuff 09 0008dc556677

w5x00 getmac
w5x00 getver

w5x00 lwipinit 192.168.10.201 255.255.255.0 192.168.10.1
w5x00 iperf
```
