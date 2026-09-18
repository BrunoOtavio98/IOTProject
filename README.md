# STM32 IOT Base Project

This repository is a bare-metal STM32 firmware project focused on building an isolated HAL-driven IoT platform.
The current implementation is centered on STM32F407 hardware and provides abstractions for modem, GNSS, storage, and debug interfaces.

## Overview

- Core target: STM32F407-based board
- Primary modem integration: SIM7020E (via UART)
- GNSS support via NMEA parsing
- SD card access through SPI
- Debug output via UART
- RTOS task wrapping for modular peripheral management

## Architecture

The project separates board-specific initialization from device-level interfaces:

- `Core/Src/HAL/Boards/BoardInterface.h`
  - Defines the board abstraction and available modem interfaces.
- `Core/Src/HAL/Boards/STM32Board.cpp`
  - Implements STM32-specific peripheral setup, clock config, SD card, and RTOS task registration.
- `Core/Src/HAL/Devices/Communication/`
  - Contains hardware UART and SPI implementations for STM32.
- `Core/Src/HAL/Devices/IOT/Modem/`
  - Contains a SIM7020E modem driver layered on top of a modem interface.
- `Core/Src/HAL/Devices/Position/`
  - Contains GNSS task handling and NMEA parsing logic.
- `Core/Src/HAL/Storage/`
  - Contains SD card storage support.
- `Core/Src/HAL/DebugController/`
  - Contains debug logging and debug interface abstractions.

## Current Status

- The project is being developed with an emphasis on isolating STM HAL dependencies from application logic.
- The board implementation currently initializes UART for debug output and configures SD card SPI.
- Modem initialization is supported in the board layer, but modem UART setup is currently commented out and can be enabled.
- GNSS support is architected with a dedicated task and NMEA parser, though some board wiring is currently disabled for development.

## Building and Testing

A helper script is included to build and run unit tests using CMake and GoogleTest.

```bash
./build_test.sh      # Build and run tests
./build_test.sh b    # Configure and build tests only
./build_test.sh r    # Run tests only
```

### Requirements

- CMake 3.7+
- A C++ compiler compatible with `-std=c++14`
- Git and network access for `FetchContent` to download GoogleTest

### How it builds

- `CMakeLists.txt` configures a static library named `IOT_LIB` from `Core/Src/HAL` sources.
- `Test/CMakeLists.txt` collects unit tests from `Test/unit` and links them against `IOT_LIB`.
- The project uses `FetchContent` to download GoogleTest for host-side unit testing.

## Project Layout

- `Core/Inc/` and `Core/Src/`
  - Firmware source and header files
- `Core/Src/HAL/`
  - Hardware abstraction layer implementation
- `Debug/`
  - Build artifacts and compiler outputs
- `Drivers/`, `Middlewares/`
  - STM HAL drivers and FreeRTOS middleware
- `Test/`
  - Unit test configuration and mocks

## Notes

- The current CMake setup is primarily intended for unit testing on host or native environment.
- Cross-compilation for the STM32 target is not fully configured in this repository yet.
- The design supports future expansion to additional devices such as GNSS or IMU modules.

## Usage

- `Core/Src/main.cpp` contains the embedded firmware entry point.
- The application creates a `STM32Board`, initializes peripherals, and starts the RTOS scheduler.

## Contact

If you extend this repository, keep board-specific HAL code isolated from application logic to preserve portability across STM32 variants and IoT devices.
