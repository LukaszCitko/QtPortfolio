import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root

    required property var mixerDevice
    required property var tankDevice
    required property var batchDevice
    required property var waterPumpDevice
    required property bool demoOverrideEnabled
    required property var resetService
    required property var drainRequestService
    required property bool hardwareMode
    required property var controllerClient


    color: "#eceeef"
    border.color: "#a5aaad"

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.top: parent.top
        anchors.topMargin: 24
        text: "SIMULATION"
        color: "#252a2d"
        font.pixelSize: 22
        font.bold: true
    }

    Rectangle {
        width: 620
        height: 430
        anchors.centerIn: parent
        color: "#f6f7f7"
        border.color: "#a5aaad"

        Column {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 12

            Text {
                text: "M1 · MIXER"
                color: "#252a2d"
                font.pixelSize: 20
                font.bold: true
            }

            Text {
                text: root.mixerDevice.connected
                      ? "Connection: CONNECTED"
                      : "Connection: DISCONNECTED"
                color: "#252a2d"
                font.pixelSize: 16
            }

            Row {
                width: parent.width
                height: 52
                spacing: 12
                visible: !root.hardwareMode

                HmiButton {
                    id: connectButton
                    width: (parent.width - parent.spacing) / 2
                    height: parent.height
                    text: root.mixerDevice.connected
                          ? "DISCONNECT MIXER"
                          : "CONNECT MIXER"

                    enabled: !root.mixerDevice.connected
                             || !root.mixerDevice.running

                    onClicked: {
                        if (root.mixerDevice.connected)
                            root.mixerDevice.disconnectDevice()
                        else
                            root.mixerDevice.connect()
                    }

                    background: Rectangle {
                        color: connectButton.enabled ? "#c5ced2" : "#d7dadd"
                        border.color: "#858e92"
                        radius: 4
                    }

                    contentItem: Text {
                        text: connectButton.text
                        color: "#252a2d"
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                HmiButton {
                    id: demoDrainButton
                    width: (parent.width - parent.spacing) / 2
                    height: parent.height
                    visible: root.demoOverrideEnabled
                    text: "DEMO DRAIN TK1"

                    enabled: root.tankDevice.volume > 0
                             && root.batchDevice.state !== BatchController.Draining

                    onClicked: root.drainRequestService.demoRequestDrain()

                    background: Rectangle {
                        color: demoDrainButton.enabled ? "#c5ced2" : "#d7dadd"
                        border.color: "#858e92"
                        radius: 4
                    }

                    contentItem: Text {
                        text: demoDrainButton.text
                        color: demoDrainButton.enabled ? "#252a2d" : "#596368"
                        font.pixelSize: 16
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            Text {
                text: "TK1 · TEMPERATURE: "
                      + root.tankDevice.temperature.toFixed(1) + " °C"
                color: "#252a2d"
                font.pixelSize: 16
            }

            // TEMPERATURE BUTTONS
            Row {
                width: parent.width
                spacing: 8
                visible: !root.hardwareMode
                Repeater {
                    model: [25, 32, 50]

                    HmiButton {
                        id: temperaturePresetButton
                        property int presetTemperature: modelData

                        width: (parent.width - 16) / 3
                        height: 52
                        text: "SET " + presetTemperature + " °C"

                        onClicked: root.tankDevice.temperature = presetTemperature

                        background: Rectangle {
                            color: "#c5ced2"
                            border.color: "#858e92"
                            radius: 4
                        }

                        contentItem: Text {
                            text: temperaturePresetButton.text
                            color: "#252a2d"
                            font.pixelSize: 16
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            // Fault SimulationControlPanel
            Row {
                width: parent.width
                spacing: 8

                Text {
                    width: 120
                    height: 52
                    text: "P1 · WATER"
                    color: "#252a2d"
                    font.pixelSize: 16
                    verticalAlignment: Text.AlignVCenter
                }

                HmiButton {
                    id: pump1FaultButton
                    width: (parent.width - 136) / 2
                    height: 52
                    text: "SET FAULT"
                    enabled: root.hardwareMode ?
                            root.controllerClient.snapshotValid && (root.controllerClient.simulatedFaultMask & 1) === 0
                            : !root.waterPumpDevice.fault

                    onClicked: {
                        if (root.hardwareMode)
                            root.controllerClient.setSimulatedFault(1, true)
                        else
                            root.waterPumpDevice.setFault()
                    }

                    background: Rectangle {
                        radius: 4
                        color: pump1FaultButton.pressed ? "#d3d6d8" : "#f2f3f3"
                        border.color: "#858e92"
                    }

                    contentItem: Text {
                        text: pump1FaultButton.text
                        color: pump1FaultButton.enabled ? "#9d2424" : "#858e92"
                        font.pixelSize: 13
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                HmiButton {
                    id: pump1ResetButton

                    width: (parent.width - 136) / 2
                    height: 52
                    text: root.hardwareMode ? "CLEAR FAULT" : "DEMO RESET"
                    visible: root.demoOverrideEnabled
                    enabled: root.waterPumpDevice.fault

                    onClicked: {
                        if (root.hardwareMode)
                            root.controllerClient.setSimulatedFault(1, false) // 1U << 0  = 1
                        else
                            root.resetService.demoResetPump1()
                    }

                    background: Rectangle {
                        radius: 4
                        color: pump1ResetButton.enabled ? "#f2f3f3" : "#d7dadd"
                        border.color: "#858e92"
                    }

                    contentItem: Text {
                        text: pump1ResetButton.text
                        color: pump1ResetButton.enabled ? "#252a2d" : "#858e92"
                        font.pixelSize: 13
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }


            }

            Row {
                width: parent.width
                spacing: 8

                Text {
                    width: 120
                    height: 52
                    text: "M1 · MIXER"
                    color: "#252a2d"
                    font.pixelSize: 16
                    verticalAlignment: Text.AlignVCenter
                }

                HmiButton {
                    id: mixerFaultButton
                    width: (parent.width - 136) / 2
                    height: 52
                    text: "SET FAULT"
                    enabled: root.hardwareMode ?
                            root.controllerClient.snapshotValid && (root.controllerClient.simulatedFaultMask & 64) === 0
                            : !root.mixerDevice.fault

                    onClicked: {
                        if (root.hardwareMode)
                            root.controllerClient.setSimulatedFault(64, true)
                        else
                            root.mixerDevice.setFault()
                    }

                    background: Rectangle {
                        radius: 4
                        color: mixerFaultButton.pressed ? "#d3d6d8" : "#f2f3f3"
                        border.color: "#858e92"
                    }

                    contentItem: Text {
                        text: mixerFaultButton.text
                        color: mixerFaultButton.enabled ? "#9d2424" : "#858e92"
                        font.pixelSize: 13
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                HmiButton {
                    id: mixerResetButton
                    width: (parent.width - 136) / 2
                    height: 52
                    text: root.hardwareMode ? "CLEAR FAULT" : "DEMO RESET"
                    visible: root.demoOverrideEnabled
                    enabled: root.mixerDevice.fault

                    onClicked: {
                        if (root.hardwareMode)
                            root.controllerClient.setSimulatedFault(64, false)  // 1U << 6 = 64
                        else
                            root.resetService.demoResetMixer()
                    }

                    background: Rectangle {
                        radius: 4
                        color: mixerResetButton.enabled ? "#f2f3f3" : "#d7dadd"
                        border.color: "#858e92"
                    }

                    contentItem: Text {
                        text: mixerResetButton.text
                        color: mixerResetButton.enabled ? "#252a2d" : "#858e92"
                        font.pixelSize: 13
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

            }

            Text {
                text: root.hardwareMode
                      ? "Hardware fault injection through Modbus TCP."
                      : "Demo connection. No physical hardware is controlled."
                color: "#596368"
                font.pixelSize: 13
            }
        }
    }
}