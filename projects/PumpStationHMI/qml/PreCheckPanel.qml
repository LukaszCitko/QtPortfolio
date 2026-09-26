import QtQuick

import QtQuick.Controls.Basic

Item {
    id: root
    required property bool operatorSelected
    readonly property bool tankEmpty: mixingTank.volume === 0
    readonly property bool devicesHealthy:
        pump.stateText !== "FAULT"
        && pump2.stateText !== "FAULT"
        && pump3.stateText !== "FAULT"
        && mixer.stateText !== "FAULT"
        && valve1.stateText !== "FAULT"
        && valve2.stateText !== "FAULT"
        && valve3.stateText !== "FAULT"
        && valve4.stateText !== "FAULT"
        && mixingTank.stateText !== "FAULT"

    readonly property bool mixerReady:
        mixer.connected && mixer.stateText !== "FAULT"

    readonly property bool valvesClosed:
        valve1.stateText === "CLOSED"
        && valve2.stateText === "CLOSED"
        && valve3.stateText === "CLOSED"
        && valve4.stateText === "CLOSED"

    readonly property var checks: [
        { label: "Operator selected", ok: root.operatorSelected },
        { label: "Tank empty", ok: tankEmpty },
        { label: "Devices without faults", ok: devicesHealthy },
        { label: "Mixer connected", ok: mixerReady },
        { label: "All valves closed", ok: valvesClosed }
    ]

    Column {
        anchors.fill: parent
        spacing: 4

        Text {
            text: "START CONDITIONS  "
                  + root.checks.filter(function(check) { return check.ok }).length
                  + " / 5"
            color: "#252a2d"
            font.pixelSize: 14
            font.bold: true
        }

        Repeater {
            model: root.checks

            Row {
                width: root.width
                height: 18

                Text {
                    width: 220
                    text: modelData.label
                    color: "#252a2d"
                    font.pixelSize: 12
                }

                Text {
                    width: root.width - 220
                    text: modelData.ok ? "OK" : "REQUIRED"
                    horizontalAlignment: Text.AlignRight
                    color: "#252a2d"
                    font.pixelSize: 12
                    font.bold: true
                }
            }
        }

        Button {
            id: startButton
            width: root.width
            height: 42
            text: "START BATCH"
            enabled: root.checks.every(function(check) { return check.ok })
            onClicked: batchController.tryStartBatch(operatorSession.operatorName)

            background: Rectangle {
                color: startButton.enabled ? "#f7f8f8" : "#d7dadd"
                border.color: "#858e92"
                border.width: 1
                radius: 4
            }

            contentItem: Text {
                text: startButton.text
                color: "#41494d"
                font.pixelSize: 16
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}