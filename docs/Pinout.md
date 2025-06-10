| Name     | No.             | Type  | In Use | Description | Full Function List                                                                 |
|----------|-----------------|-------|--------|-------------|------------------------------------------------------------------------------------|
| GND      | 1,2,42,43,46-65 | P     |        |             | GND                                                                                |
| 3V3      | 3               | P     |        |             | Powersupply                                                                        |
| IO0      | 4               | I/O/T |        |             | RTC_GPIO0, GPIO0                                                                   |
| IO1      | 5               | I/O/T |        |             | RTC_GPIO1, GPIO1,TOUCH1,ADC1_CH0                                                   |
| IO2      | 6               | I/O/T |        |             | RTC_GPIO2, GPIO2,TOUCH2,ADC1_CH1                                                   |
| IO3      | 7               | I/O/T |        |             | RTC_GPIO3, GPIO3,TOUCH3,ADC1_CH2                                                   |
| IO4      | 8               | I/O/T |        |             | RTC_GPIO4, GPIO4,TOUCH4,ADC1_CH3                                                   |
| IO5      | 9               | I/O/T |        |             | RTC_GPIO5, GPIO5,TOUCH5,ADC1_CH4                                                   |
| IO6      | 10              | I/O/T |        |             | RTC_GPIO6, GPIO6,TOUCH6,ADC1_CH5                                                   |
| IO7      | 11              | I/O/T |        |             | RTC_GPIO7, GPIO7,TOUCH7,ADC1_CH6                                                   |
| IO8      | 12              | I/O/T |        |             | RTC_GPIO8, GPIO8,TOUCH8,ADC1_CH7,SUBSPICS1                                         |
| IO9      | 13              | I/O/T |        |             | RTC_GPIO9, GPIO9,TOUCH9,ADC1_CH8,FSPIHD,SUBSPIHD                                   |
| IO10     | 14              | I/O/T |        |             | RTC_GPIO10, GPIO10,TOUCH10,ADC1_CH9,FSPICS0,FSPIIO4, SUBSPICS0                     |
| IO11     | 15              | I/O/T |        |             | RTC_GPIO11, GPIO11,TOUCH11,ADC2_CH0,FSPID,FSPIIO5,SUBSPID                          |
| IO12     | 16              | I/O/T |        |             | RTC_GPIO12, GPIO12,TOUCH12,ADC2_CH1,FSPICLK,FSPIIO6, SUBSPICLK                     |
| IO13     | 17              | I/O/T |        |             | RTC_GPIO13, GPIO13,TOUCH13,ADC2_CH2,FSPIQ,FSPIIO7,SUBSPIQ                          |
| IO14     | 18              | I/O/T |        |             | RTC_GPIO14, GPIO14,TOUCH14,ADC2_CH3,FSPIWP,FSPIDQS, SUBSPIWP                       |
| IO15     | 19              | I/O/T |        |             | RTC_GPIO15, GPIO15,U0RTS,ADC2_CH4,XTAL_32K_P                                       |
| IO16     | 20              | I/O/T |        |             | RTC_GPIO16, GPIO16,U0CTS,ADC2_CH5,XTAL_32K_N                                       |
| IO17     | 21              | I/O/T |        |             | RTC_GPIO17, GPIO17,U1TXD,ADC2_CH6                                                  |
| IO18     | 22              | I/O/T |        |             | RTC_GPIO18, GPIO18,U1RXD,ADC2_CH7,CLK_OUT3                                         |
| IO19     | 23              | I/O/T |        |             | RTC_GPIO19,GPIO19,U1RTS,ADC2_CH8,CLK_OUT2, USB_D-                                  |
| IO20     | 24              | I/O/T |        |             | RTC_GPIO20,GPIO20,U1CTS,ADC2_CH9,CLK_OUT1, USB_D+                                  |
| IO21     | 25              | I/O/T |        |             | RTC_GPIO21, GPIO21                                                                 |
| IO26^{b} | 26              | I/O/T |        |             | SPICS1, GPIO26                                                                     |
| IO47     | 27              | I/O/T |        |             | SPICLK_P_DIFF, GPIO47,SUBSPICLK_P_DIFF                                             |
| IO33     | 28              | I/O/T |        |             | SPIIO4, GPIO33,FSPIHD,SUBSPIHD                                                     |
| IO34     | 29              | I/O/T |        |             | SPIIO5, GPIO34,FSPICS0,SUBSPICS0                                                   |
| IO48     | 30              | I/O/T |        |             | SPICLK_N_DIFF, GPIO48,SUBSPICLK_N_DIFF                                             |
| IO35     | 31              | I/O/T |        |             | SPIIO6, GPIO35,FSPID,SUBSPID                                                       |
| IO36     | 32              | I/O/T |        |             | SPIIO7, GPIO36,FSPICLK,SUBSPICLK                                                   |
| IO37     | 33              | I/O/T |        |             | SPIDQS, GPIO37,FSPIQ,SUBSPIQ                                                       |
| IO38     | 34              | I/O/T |        |             | GPIO38,FSPIWP,SUBSPIWP                                                             |
| IO39     | 35              | I/O/T |        |             | MTCK,GPIO39,CLK_OUT3,SUBSPICS1                                                     |
| IO40     | 36              | I/O/T |        |             | MTDO,GPIO40,CLK_OUT2                                                               |
| IO41     | 37              | I/O/T |        |             | MTDI,GPIO41,CLK_OUT1                                                               |
| IO42     | 38              | I/O/T |        |             | MTMS,GPIO42                                                                        |
| TXD0     | 39              | I/O/T |        |             | U0TXD,GPIO43,CLK_OUT1                                                              |
| RXD0     | 40              | I/O/T |        |             | U0RXD,GPIO44,CLK_OUT2                                                              |
| IO45     | 41              | I/O/T |        |             | GPIO45                                                                             |
| IO46     | 44              | I/O/T |        |             | GPIO46                                                                             |
| EN       | 45              | I     |        |             | High:on,enablesthechip. <br> Low:off,thechippowersoff. <br> Note:DonotleavetheENpinfloating. |

b For modules with ordering codes ending with -N4R2, IO26 connects to the embedded PSRAM and is not
available for other uses

![image](https://github.com/user-attachments/assets/749fc0e4-0d8b-4fba-8f06-0fa211cc2fcb)

