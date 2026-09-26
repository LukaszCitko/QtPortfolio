import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root

    required property var mixerDevice
    required property var tankDevice
    required property var batchDevice

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
        width: 420
        height: 340
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
                      ? "MIXER CONNECTED"
                      : "CONNECT MIXER"
                enabled: !root.mixerDevice.connected
                onClicked: root.mixerDevice.connect()

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

            Button {
                id: temperatureButton
                width: 250
                height: 52
                text: "SET TANK TO 60 °C"

                onClicked: root.tankDevice.temperature = 60.0

                background: Rectangle {
                    color: temperatureButton.enabled ? "#c5ced2" : "#d7dadd"
                    border.color: "#858e92"
                    radius: 4
                }

                contentItem: Text {
                    text: temperatureButton.text
                    color: "#252a2d"
                    font.pixelSize: 16
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
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