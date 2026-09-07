#pragma once

#include <cstdint>

namespace fw
{

//! @brief Driver for ADC1, 12-bit SAR mode: external input on PA1 (channel 1) and the
//! internal die temperature sensor (channel 16).
//!
//! Configures the on-chip 12-bit successive-approximation ADC with software-triggered
//! single conversions. Input range is 0 V to VDDA (~3.3 V), right-aligned 12-bit result.
class Adc1
{
public:
    //! @brief Initialize ADC1.
    //!
    //! Enables the ADC1 peripheral clock, configures PA1 as analog input, powers on
    //! ADC1, runs calibration, sets sample times, and enables the internal temperature
    //! sensor. Must be called before any Read method.
    void Initialize();

    //! @brief Trigger a single conversion on PA1 and return the 12-bit raw result.
    //!
    //! Blocks until the conversion completes or a timeout expires.
    //!
    //! @return 12-bit ADC result (0-4095), or 0 on conversion timeout.
    uint16_t ReadRaw();

    //! @brief Trigger a single conversion on PA1 and return the result in volts.
    //!
    //! @return Measured voltage on PA1 (ADC1_IN1), in volts.
    float ReadVoltageVolts();

    //! @brief Trigger a single conversion of the die temperature sensor (channel 16).
    //!
    //! @return 12-bit ADC result (0-4095), or 0 on conversion timeout.
    uint16_t ReadDieTemperatureRaw();

    //! @brief Read the die temperature via the factory TS_CAL1/TS_CAL2 calibration.
    //!
    //! @return Die temperature in degrees Celsius.
    float ReadDieTemperatureCelsius();

    //! @brief Convert a raw channel-16 reading to degrees Celsius.
    //!
    //! Linear interpolation between the factory calibration points (30 C and 110 C,
    //! measured at VDDA = 3.3 V).
    //! @param raw 12-bit ADC result from the die temperature sensor.
    //! @return Die temperature in degrees Celsius.
    static float DieTemperatureCelsiusFromRaw(uint16_t raw);

    //! @brief Print ADC1 register values to the console for diagnostics.
    void PrintDiagnostics();

private:
    //! VDDA reference voltage used for code-to-voltage conversion.
    static constexpr float kVref{3.3f};

    uint16_t ConvertChannel(uint32_t channel);
};

}  // namespace fw
