Raspberry Pi Pico Ethernet CLI Test
======================================

### Overview
이 레파지토리는 RP2040과 W5100S의 구성에서 CLI를 이용하여 여러 가지 테스트를 진행하기 위함이다.

### Quick Start Firmware
* You can start this quickly with these compiled files.
  - **[Raspberry Pi Pico Ethernet CLI (UF2 file 200KB)](build/examples/pico-e-cli/pico-e-cli.uf2)**
  
### SPI 설정
SPI는 기존의 방식을 사용하거나 RP2040 PIO를 사용할 수 있다.
단, SPI Normal 방식와 PIO 방식의 Clock 설정 변수가 다른 점을 주의하세요.
그리고, SPI를 재 설정할 때에는 초기화 후에 해야 한다.
(system reset)

아래는 PIO를 사용하지 않는 SPI 설정 화면이다.
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
아래는 PIO를 사용하고 SPI Clock을 16.6MHz로 설정한 화면이다.
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
PIO Code를 Running 중에 변경하기 위해서 몇몇 코드를 수정하였다.
```
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
위의 코드에서 wiznet_spi_write_read_program_instructions의 0xe180 부분이 PIO Delay와 관련된 부분이었고, 이는 "w5x00 piohook" 명령을 통해 수정할 수 있다.
Instruction Code와 Delay 관련 부분은 RP2040의 데이터쉬트를 참고하였다.
```
w5x00 piohook e080 // set    pindirs, 0      side 0 [0] 
w5x00 piohook e180 // set    pindirs, 0      side 0 [1] 
w5x00 piohook e480 // set    pindirs, 0      side 0 [4] 
```
![image](https://user-images.githubusercontent.com/2126804/230025606-4772484b-5868-4cad-ad44-138823cb310d.png)


### SPI Normal(not PIO), SPI Clock 16.6MHz, SPI R/W Delay 100
![image](https://user-images.githubusercontent.com/2126804/230020530-8e207293-7fc0-4b39-bd98-64ff3faafac8.png)
![image](https://user-images.githubusercontent.com/2126804/230020923-f6efa3d6-4626-4a27-b4ee-276c17877912.png)

### SPI PIO, SPI Clock 16.6MHz, SPI R/W Delay 100, PIO Delay 0 (0xe080)
![image](https://user-images.githubusercontent.com/2126804/230020753-2f0ce2e0-09f5-4358-a4ca-2cab062e3d7d.png)

### SPI PIO, SPI Clock 16.6MHz, SPI R/W Delay 100, PIO Delay 4 (0xe480)
![image](https://user-images.githubusercontent.com/2126804/230021134-d6dc5685-e853-4b33-9220-a206488539b2.png)

### Misc CLI Command
아래와 같은 Command를 이용해서 추가적인 테스트를 할 수 있다.
```
system reset

w5x00 init spinormal 4
w5x00 init spinormal 8
w5x00 init spinormal 128

w5x00 piohook e080
w5x00 piohook e180
w5x00 piohook e480
w5x00 init spipio 8
w5x00 init spipio 128

w5x00 readbuff 09 6
w5x00 readbuffdelay 09 6 0
w5x00 readbuffdelay 09 6 100

w5x00 writebuff 09 0008dc556677

w5x00 getmac
w5x00 getver

w5x00 lwipinit 192.168.10.201 255.255.255.0 192.168.10.1
w5x00 iperf
```
