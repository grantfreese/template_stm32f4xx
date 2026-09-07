set(FREERTOS_SOURCES
    croutine.c
    event_groups.c
    list.c
    queue.c
    stream_buffer.c
    tasks.c
    timers.c
    portable/GCC/ARM_CM4F/port.c
    portable/MemMang/heap_4.c
    CMSIS_RTOS_V2/cmsis_os2.c
)

add_library(freertos STATIC ${FREERTOS_SOURCES})
target_link_libraries(freertos PUBLIC driver)

target_include_directories(freertos PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/portable/GCC/ARM_CM4F
    ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS_RTOS_V2
)
