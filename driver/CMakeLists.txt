set(DRIVER_SOURCE_FILES
    # adc1
    ${CMAKE_CURRENT_SOURCE_DIR}/adc1/adc1.cpp

    # i2c2
    ${CMAKE_CURRENT_SOURCE_DIR}/i2c2/i2c_stm32.cpp

    # adg728
    ${CMAKE_CURRENT_SOURCE_DIR}/adg728/adg728.cpp

    # stm32 hal
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_exti.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_pwr_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_tim_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_timebase_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_spi_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_uart_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/src/stm32f3xx_hal_i2c_ex.c
)

add_library(driver STATIC ${DRIVER_SOURCE_FILES})

target_include_directories(driver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/cmsis/device/ST/STM32F3xx/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/i2c2
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f3xx_hal_driver/inc
)

target_link_libraries(driver PUBLIC app freertos)
