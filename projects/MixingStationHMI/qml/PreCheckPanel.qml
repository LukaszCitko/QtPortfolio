import QtQuick

import QtQuick.Controls.Basic

Item {
    id: root
    required property bool operatorSelected
    required property string operatorName
    required property var controller

    readonly property bool tankEmpty: root.controller.tankEmpty
    readonly property bool devicesHealthy: root.controller.devicesHealthy
    readonly property bool mixerReady: root.controller.mixerReady
    readonly property bool valvesClosed: root.controller.valvesClosed
    readonly property var checks:
    [
        { label: "Operator selected", ok: root.operatorSelected },
        { label: "Tank empty", ok: root.tankEmpty },
        { label: "Devices without faults", ok: root.devicesHealthy },
        { label: "Mixer connected", ok: root.mixerReady },
        { label: "All valves closed", ok: root.valvesClosed }
    ]

    Column {
        anchors.fill: parent
        spacing: 4

        Text {
            text: "START CONDITIONS  "
                  + root.checks.filter(function(check) { return check.ok }).length
                  + " / 5"
            color: "#252a2d"
            font.pixelSize: 15
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
                    font.pixelSize: 13
                }

                Text {
                    width: root.width - 220
                    text: modelData.ok ? "OK" : "REQUIRED"
                    horizontalAlignment: Text.AlignRight
                    color: "#252a2d"
                    font.pixelSize: 13
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
            onClicked: root.controller.tryStartBatch(root.operatorName)

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