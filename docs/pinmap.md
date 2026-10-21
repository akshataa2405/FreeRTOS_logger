# Pin Mapping — NUCLEO-F446RE Data Logger

| Function | Pin | Peripheral | Description | Wire / Connect To |
|---|---|---|---|---|
| **ADC Input** | PA0 | ADC1_IN0 | Analog Input Channel | Wiper of 10 kΩ pot OR loopback from PA4 |
| **DAC Loopback** | PA4 | DAC_OUT1 | Self-test Signal Generator | Connect jumper to PA0 for automated accuracy test |
| **I²C SCL** | PB8 | I2C1_SCL | MPU6050 Clock (400 kHz) | MPU6050 SCL (4.7 kΩ pull-up to 3.3V) |
| **I²C SDA** | PB9 | I2C1_SDA | MPU6050 Data | MPU6050 SDA (4.7 kΩ pull-up to 3.3V) |
| **SD SPI SCK** | PB13 | SPI2_SCK | SD Card Clock | MicroSD module SCK |
| **SD SPI MISO**| PB14 | SPI2_MISO| SD Card Data Out | MicroSD module MISO |
| **SD SPI MOSI**| PB15 | SPI2_MOSI| SD Card Data In | MicroSD module MOSI |
| **SD SPI CS**  | PB12 | GPIO Out | SD Card Chip Select | MicroSD module CS |
| **UART TX**    | PA2 | USART2_TX | Virtual COM Port (115200 8N1)| Onboard ST-LINK USB |
| **UART RX**    | PA3 | USART2_RX | Virtual COM Port (115200 8N1)| Onboard ST-LINK USB |
| **Debug CH0**  | PA8 | GPIO Out | Logic Analyzer: ADC Active | Logic Analyzer Ch 0 |
| **Debug CH1**  | PA9 | GPIO Out | Logic Analyzer: Sensor Active | Logic Analyzer Ch 1 |
| **Debug CH2**  | PA10| GPIO Out | Logic Analyzer: Logger Write | Logic Analyzer Ch 2 |
| **Debug CH3**  | PB0 | GPIO Out | Logic Analyzer: Lock Held | Logic Analyzer Ch 3 |
| **Debug CH4**  | PB1 | GPIO Out | Logic Analyzer: Record Waiting| Logic Analyzer Ch 4 |
| **Status LED** | PA5 | GPIO Out | On-board Green LED (LD2) | Blinks on successful recording |
| **User Button**| PC13| GPIO In  | Blue User Push Button (B1) | Toggle logging start / stop |
