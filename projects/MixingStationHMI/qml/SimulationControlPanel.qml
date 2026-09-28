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

            Button {
                id: connectButton
                width: 250
                height: 52
                text: root.mixerDevice.connected
                      ? "DISCONNECT MIXER"
                      : "CONNECT MIXER"
                enabled: !root.mixerDevice.connected
                         || root.mixerDevice.stateText !== "RUNNING"
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

                Repeater {
                    model: [25, 60, 90]

                    Button {
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

                Button {
                    width: (parent.width - 136) / 2
                    height: 52
                    text: "SET FAULT"
                    enabled: root.waterPumpDevice.stateText !== "FAULT"
                    onClicked: root.waterPumpDevice.setFault()
                }
                Button {
                    width: (parent.width - 136) / 2
                    height: 52
                    text: "DEMO RESET"
                    visible: root.demoOverrideEnabled
                    enabled: root.waterPumpDevice.stateText === "FAULT"
                    onClicked: root.resetService.demoResetPump1()
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

                Button {
                    width: (parent.width - 136) / 2
                    height: 52
                    text: "SET FAULT"
                    enabled: root.mixerDevice.stateText !== "FAULT"
                    onClicked: root.mixerDevice.setFault()
                }
                Button {
                    width: (parent.width - 136) / 2
                    height: 52
                    text: "DEMO RESET"
                    visible: root.demoOverrideEnabled
                    enabled: root.mixerDevice.stateText === "FAULT"
                    onClicked: root.resetService.demoResetMixer()
                }

            }

            Text {
                text: "Demo connection. No physical hardware is controlled."
                color: "#596368"
                font.pixelSize: 13
            }
        }
    }
}