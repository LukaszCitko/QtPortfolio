#pragma once

#include <cstdint>

namespace ProcessConfiguration
{

inline constexpr std::uint16_t Pump1DefaultRpm = 600;
inline constexpr std::uint16_t Pump2DefaultRpm = 400;
inline constexpr std::uint16_t Pump3DefaultRpm = 600;
inline constexpr std::uint16_t DosingMixerRpm = 200;
inline constexpr std::uint16_t MixingMixerRpm = 800;

inline constexpr std::uint16_t WaterTargetDecilitres = 700;
inline constexpr std::uint16_t ConcentrateTargetDecilitres = 300;
inline constexpr std::uint16_t MixtureTargetDecilitres = 1000;

inline constexpr std::uint16_t WaterFlowAtMaximumRpmDecilitresPerSecond = 200;
inline constexpr std::uint16_t ConcentrateFlowAtMaximumRpmDecilitresPerSecond = 200;
inline constexpr std::uint16_t ProductFlowAtMaximumRpmDecilitresPerSecond = 170;

inline constexpr std::uint16_t PumpRpmChangePerSecond = 2000;
inline constexpr std::uint16_t SimulationStepMilliseconds = 100;

}  // namespace ProcessConfiguration
