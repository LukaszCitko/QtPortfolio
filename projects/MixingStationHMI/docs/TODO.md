# MixingStationHMI follow-up work

## Recovery and safety

- Replace the temporary QML `recoveryAllowed: true` override with Technician/Admin authorization. - move logic to backend code
- Fix the `RemoteProcessController::connected` notification used by QML and restore the connection requirement for recovery confirmation.
- Persist recovery-required information when both the ESP32 controller and HMI are restarted.
- Distinguish initial connection, HMI reconnection and controller restart in History.
- Add a physical tank-level input before allowing automatic recovery operations.
- Consider storing a diagnostic process checkpoint in ESP32 NVS without automatically restarting equipment after reboot.

## Hardware

- Add physical process-level measurement.
- Add physical pump feedback inputs.
- Decide whether physical fault inputs will be included in the next hardware release.

## Diagnostics

- Add optional network diagnostics for maintenance while retaining USB/UART as the firmware development log channel.
