# MixingStationHMI

[![MixingStationHMI CI](https://github.com/LukaszCitko/QtPortfolio/actions/workflows/mixingstation-ci.yml/badge.svg?branch=main)](https://github.com/LukaszCitko/QtPortfolio/actions/workflows/mixingstation-ci.yml)

A touch-oriented batch mixing station HMI built with Qt 6.8, C++ and QML. This portfolio project simulates process equipment and uses a 1280 × 800 interface inspired by high-performance HMI principles: neutral equipment graphics, clear process values and colour reserved for conditions that need attention.

![Mixing stage in the process view](docs/screenshots/cropped/mixing.png)

## What the demo covers

- **Batch sequence:** pre-start checks, a 70 L water target, a 30 L concentrate target, temperature check at 58–62 °C, 15 seconds of mixing, and operator-initiated transfer. The batch target is 100 L in a 120 L physical tank.
- **Equipment and faults:** P1–P3 pumps, V1–V4 valves, TK1 tank and M1 mixer. The Equipment view shows status and pump RPM settings. Supported P1 and M1 faults can be injected from Simulation and reset according to the selected role.
- **Process visibility:** current values, stage progress, alarms and event messages, live trends, and run history with events and saved trend samples.
- **Persistence and roles:** SQLite stores users, runs, events and trend samples. Demo operator, technician and admin roles control batch operation, fault reset, drain approval and user management.

[Open the illustrated application guide (PDF)](docs/MixingStationHMI-Guide.pdf) · [Browse all cropped screenshots](docs/screenshots/cropped/)

## Try a batch

1. Select a demo operator with **LOGIN**. This selects a role; it does not authenticate a person.
2. Open **SIMULATION**, connect M1 and set TK1 to **60 °C**.
3. Return to **PROCESS**. When all five start conditions pass, press **START BATCH**.
4. Watch water filling and concentrate dosing. The mixer then runs at 800 RPM for 15 seconds; after mixing it returns to 200 RPM while waiting for transfer.
5. Press **START TRANSFER**. Open **TRENDS** for live data or **HISTORY** to inspect a saved run and its historical trend.

To explore faults, inject a P1 or M1 fault in **SIMULATION**. A technician or admin can reset the supported fault from **EQUIPMENT**. The simulation panel also includes explicitly labelled **DEMO RESET** and **DEMO DRAIN TK1** overrides for an operator; their use is logged as a simulation override.

## Screens

| Process and faults | Trends and history |
| --- | --- |
| [Water filling](docs/screenshots/cropped/water-filling.png) · [Mixing](docs/screenshots/cropped/mixing.png) · [Paused on fault](docs/screenshots/cropped/fault-paused.png) · [Equipment fault](docs/screenshots/cropped/p1-fault-equipment.png) | [Live trends](docs/screenshots/cropped/live-trends.png) · [Run history](docs/screenshots/cropped/run-history.png) · [Historical trends](docs/screenshots/cropped/historical-trends.png) |

[Demo login](docs/screenshots/cropped/demo-login.png) · [Admin user management](docs/screenshots/cropped/user-management.png) · [Ready for transfer](docs/screenshots/cropped/ready-for-transfer.png)

## Build and test

Requirements: Qt **6.8** with Qt Quick, Qt SQL and Qt Test, a C++ compiler and CMake **3.16+**. Open `CMakeLists.txt` in Qt Creator and select a Qt 6.8 kit, or build from the project directory:

```sh
cmake -S . -B build/portfolio -DCMAKE_PREFIX_PATH="/path/to/Qt/6.8.0/macos"
cmake --build build/portfolio --parallel
ctest --test-dir build/portfolio --output-on-failure
```

Replace `CMAKE_PREFIX_PATH` with the location of your Qt installation. Run `appMixingStationHMI` from the selected build directory or through Qt Creator. The app creates its SQLite database in the application data directory and prints the exact path at startup.

## Architecture at a glance

- `src/`: devices, batch state machine, simulator, event coordination, role policy and services.
- `src/database/`: schema, repositories, persistence coordination and QAbstractListModel-backed views for users, runs and events.
- `qml/`: the 1280 × 800 HMI, equipment controls, trends, history, alarms and simulation UI.
- `tests/`: Qt Test coverage of process behaviour, permissions and persistence.

The C++ process objects and services expose state and actions to QML. Repositories write runs, events and trend samples to SQLite; list models read persisted data for the HMI.

## Scope of this version

This is a portfolio **simulation**, not a controller for physical equipment. Demo login has no card, chip or password authentication. No hardware I/O, physical interlocks or production safety validation are implemented. The **DEMO RESET** and **DEMO DRAIN TK1** paths are intentionally limited to the simulator and should not be treated as an equipment permission.
