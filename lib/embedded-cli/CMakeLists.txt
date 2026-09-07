set(EMBEDDED_CLI_SOURCES
    embedded_cli.cpp
)

add_library(embedded-cli STATIC ${EMBEDDED_CLI_SOURCES})

target_link_libraries(embedded-cli PUBLIC driver)

target_include_directories(embedded-cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
