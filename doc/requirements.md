<PROJECT_NAME> requirements

# Status markers:
#   [ ] - Not started, no ticket written
#   [~] - Ticket written (ticket ID in parentheses, e.g. "(T16)" or "(B3)")
#   [?] - Implemented but not yet tested
#   [x] - Implemented and tested



> build
    > [?] build.firmware (T2)
        - CMake/Ninja cross-compile produces build/bin/app.elf (and app.hex) for the STM32F373 target
        - Toolchain file toolchain/arm-gcc-toolchain.cmake drives the pinned arm-none-eabi GCC 10.3-2021.10
        - The link step prints the RAM/FLASH memory usage summary

    > [?] build.packages (T2)
        - doc/install-packages.sh installs every apt and python dependency the build needs
        - The pinned ARM toolchain is documented as a separate manual install

    > [?] build.rtos (T2)
        - FreeRTOS kernel V10.0.1 (GCC/ARM_CM4F port, heap_4) boots via the CMSIS-RTOS v2 shim
        - The CLI task is created with static allocation before osKernelStart()

> cli
    > [?] cli.console (T2)
        - Interactive CLI on USART1 at 115200 baud using the embedded-cli framework
        - Infrastructure commands (clear, reset, save) are always registered
        - Optional CLI modules are compiled in via cmake/cli_features.cmake and register through
          weak-symbol registrars declared in src/cli/cli_registry.h
        - 'simple' toggles autocomplete-free, escape-sequence-free output for automated drivers
          (session-scoped; resets to fancy mode on boot)
        - The debug framework ('debug' command) gates per-module streamers with a master enable,
          per-source flags, and a budgeted print loop drained from the CLI task

> driver
    > [?] driver.gpio (T2)
        - IOPinSTM32F3 (src/platform/stm32f3xx_pin.{h,cpp}) implements the IDigitalSignal interface
          for GPIO used as digital signals, with configurable pull and speed
        - Peripheral drivers own their pins' alternate-function muxing directly via the HAL

    > [?] driver.i2c2 (T2)
        - I2C2 peripheral is configured as a master on PF6 (SCL) / PF7 (SDA)
        - Driver implements the II2C interface using the STM32 HAL
        - Standard mode (100 kHz) sourced from HSI (8 MHz I2C kernel clock)
        - Driver class lives at driver/i2c2/i2c_stm32.{h,cpp}
        - 'i2c scan/probe' CLI operates on buses registered via RegisterI2cCliBus()

    > [?] driver.adc1 (T2)
        - ADC1 (12-bit SAR ADC) driver provides polled single-conversion reads returning raw
          counts (0-4095) and voltage (0-3.3 V)

    > [?] driver.adg728 (T2)
        - Adg728 inherits from IMux<8, MuxType::kSingleEnded> and drives the switch over an
          injected II2C bus
        - GetChannels() polls hardware via I2C read; GetShadow() returns the last-written state
          without a bus round-trip

    > [?] driver.ads131m0x (T2)
        - Header-only Ads131m0x / Ads131m0xSpi templates drive the ADS131M0x delta-sigma ADC over
          an injected ISPI bus with an ISettableDigitalSignal chip select
        - Not instantiated by the template; a board project supplies the SPI bus and CS pin

> nvm
    > [?] nvm.persistence (T4)
        - Dual-slot redundant storage in the last two 2 KB flash pages (0x0803F000, 0x0803F800);
          the linker script caps FLASH at 252 KB to reserve them
        - Every write targets the inactive slot, programs and verifies it, then promotes it, so a
          partial write never corrupts the active record
        - Boot slot selection: valid magic AND highest nvm_sequence wins; if both slots are invalid
          (first boot, chip erase, kNvmMagic bump), first-boot defaults are applied and seed slot 0
        - NOTE: "NVM" is the persistent settings record (NonvolatileStorage_t) managed by
          src/nvm/nvm.{h,cpp}, not raw flash access in general

    > [?] nvm.testValue (T4)
        - The record carries a placeholder float test_value (default 1.234) proving the persistence
          path until a project stores real settings
        - 'nvm show' prints all fields plus sequence and magic validity; 'nvm get test' /
          'nvm set test <value>' read and write the in-RAM record; 'save' persists it to flash

> can
    > [?] can.fwVersion (T3)
        - Periodic FW_VERSION broadcast carrying the git-derived firmware version (PROJECT_VERSION)
        - Extended ID 0x1FA00001, DLC 5: major/minor/patch/tagdist bytes plus local and dirty
          flags, big-endian per stm32f4_template.dbc
        - Broadcast at 1 Hz from the CAN task on the 1 Mbit/s bus; 'can on|off' gates the
          broadcast (session-scoped, default on); 'debug can' streams TX/RX frames

    > [?] can.temperature (T3)
        - Periodic broadcast of the STM32 internal die temperature (ADC1 channel 16, converted with
          the factory TS_CAL1/TS_CAL2 calibration values)
        - Extended ID 0x1FA00002, DLC 4: temperature at 0.01 degC/bit (int16) plus raw ADC counts
          (uint16), big-endian per stm32f4_template.dbc
        - Sampled at 1 Hz by the CAN task even when the broadcast is gated off; latest value
          shown by the 'can' CLI command
