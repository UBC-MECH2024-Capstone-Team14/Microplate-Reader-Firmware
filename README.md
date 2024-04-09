# Microplate-Reader-Firmware

This is the firmware subsystem of the [Microplate-Reader](https://github.com/UBC-MECH2024-Capstone-Team14/Microplate-Reader) project.

## Developing Environment

This project uses [Arduino IDE](https://www.arduino.cc/en/software) or visual studio code with [Arduino extension](https://marketplace.visualstudio.com/items?itemName=vsciot-vscode.vscode-arduino), with [stm32duino](https://github.com/stm32duino/Arduino_Core_STM32) package. Please follow the stm32duino instructions to install.

Also, install [TMCStepper](https://www.arduino.cc/reference/en/libraries/tmcstepper/) Arduino library.

You will also need an ST-Link to program the Blue Pill through stm32duino.

### Hardware Selection

- STM32F103C8 Blue Pill
  - Any MCU with USART, SPI, 8 ADC channels and 8 PWM channels will satisfy the project requirements.
- [TMC5130A-BOB](https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/tmc5130a-bob.html)
- LED driver using PWM
- Photodiode amplifier to convert current to voltage signal

#### Pin Connection

Blue Pill pins must be connected as in [Microplate_Reader_Firmware.ino](./Microplate_Reader_Firmware/Microplate_Reader_Firmware.ino).

```C
// TMC5130 SPI Pins
#define CS_PIN PB12  // Chip select
#define SW_MOSI PB15 // Software Master Out Slave In (MOSI)
#define SW_MISO PB14 // Software Master In Slave Out (MISO)
#define SW_SCK PB13  // Software Slave Clock (SCK)

// ADC Pins, connected to photodiode amplifier outputs (0V - 3.3V)
#define CHANNEL_1_ADC_PIN PA1
#define CHANNEL_2_ADC_PIN PA2
#define CHANNEL_3_ADC_PIN PA3
#define CHANNEL_4_ADC_PIN PA4
#define CHANNEL_5_ADC_PIN PA5
#define CHANNEL_6_ADC_PIN PA6
#define CHANNEL_7_ADC_PIN PA7
#define CHANNEL_8_ADC_PIN PB0

// LED Driver PWM Pins
#define CHANNEL_1_PWM_PIN PB9
#define CHANNEL_2_PWM_PIN PB8
#define CHANNEL_3_PWM_PIN PB7
#define CHANNEL_4_PWM_PIN PB6
#define CHANNEL_5_PWM_PIN PA10
#define CHANNEL_6_PWM_PIN PA9
#define CHANNEL_7_PWM_PIN PA8
#define CHANNEL_8_PWM_PIN PA0
```

- The Blue Pill, photodiode amplifier and LED driver must share a common ground.
- The Blue Pill is powered by a USB cable.
- The TMC5130 power rails should be powered separately. Its VCC can share the Blue Pill 3.3V/5V.
  - TMC5130 power rails must be powered first, and then power VCC.
- TMC5130 REFL must be connected to an active-high limit switch. This is the home position of the linear stage.

### STM32duino Configuration

Tools => Board => STM32 Based MCU Boards => Generic STM32F1 series

Board part number => Bluepill F103C8

USB Support => CDC (generic "serial" supersedes U(S)ART)

U(S)ART Support => Enabled (generic "serial")

USB Speed => Low/Full Speed

Leave the rest as default.

## Upload

You are ready to go! Click "upload" to compile and upload the firmware.
