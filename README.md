# Microplate-Reader-Firmware

## Developing Environment

This project uses [Arduino IDE](https://www.arduino.cc/en/software) or visual studio code with Arduino addon, with [stm32duino](https://github.com/stm32duino/Arduino_Core_STM32) package. Please follow the stm32duino instructions to install.

Also, install [TMCStepper](https://www.arduino.cc/reference/en/libraries/tmcstepper/) library.

### Hardware Selection

- STM32F103C8 Blue Pill
  - Any MCU with USART, SPI, 8 ADC channels and 8 PWM channels will satisfy the project requirements.
- [TMC5130A-BOB](https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/tmc5130a-bob.html)
- ST-Link
  - This is only to program the Blue Pill through stm32duino.

### STM32duino Configuration

Tools => Board => STM32 Based MCU Boards => Generic STM32F1 series

Board part number => Bluepill F103C8

USB Support => CDC (generic "serial" supersedes U(S)ART)

U(S)ART Support => Enabled (generic "serial")

USB Speed => Low/Full Speed

Leave the rest as default.

## Upload

You are ready to go! Click "upload" to compile and upload the firmware.
