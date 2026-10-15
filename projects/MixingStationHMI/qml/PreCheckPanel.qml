import QtQuick

import QtQuick.Controls.Basic

Item {
    id: root
    required property bool operatorSelected
    required property string operatorName
    required property var controller
    required property bool hardwareMode
    required property bool recoveryAllowed
    readonly property bool recoveryRequired: root.hardwareMode && root.controller.recoveryRequired
    readonly property bool temperatureSensorReady: root.controller.temperatureSensorReady
    readonly property bool tankEmpty: root.controller.tankEmpty
    readonly property bool devicesHealthy: root.controller.devicesHealthy
    readonly property bool mixerReady: root.controller.mixerReady
    readonly property bool valvesClosed: root.controller.valvesClosed
    readonly property var checks:
    [
        { label: "Operator selected", ok: root.operatorSelected },
        { label: "Tank empty", ok: root.tankEmpty },
        { label: "Devices without faults", ok: root.devicesHealthy },
        { label: "Temperature sensor", ok: root.temperatureSensorReady },
        { label: "Mixer connected", ok: root.mixerReady },
        { label: "All valves closed", ok: root.valvesClosed }
    ]

    Column {
        anchors.fill: parent
        spacing: 4
        visible: !root.recoveryRequired

        Text {
            text: "START CONDITIONS  "
                  + root.checks.filter(function(check) { return check.ok }).length
                  + " / " + root.checks.length
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

        HmiButton {
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
    Column {
        anchors.fill: parent
        spacing: 10
        visible: root.recoveryRequired

        Text {
            width: parent.width
            text: "RECOVERY REQUIRED"
            color: "#8f2222"
            font.pixelSize: 17
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
        }

        Text {
            width: parent.width
            text: "Controller restarted during an active run. Tank state is unknown. Starting a new batch is blocked."
            color: "#252a2d"
            font.pixelSize: 13
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        HmiButton {
            id: completeRecoveryButton
            width: parent.width
            height: 48
            text: "TANK EMPTIED MECHANICALLY · COMPLETE RECOVERY"
            enabled: root.recoveryAllowed                                    //TODO: enabled: root.recoveryAllowed && root.controller.connected
            onClicked: root.controller.completeMechanicalRecovery()

            background: Rectangle {
                color: completeRecoveryButton.enabled ? "#f7f8f8" : "#d7dadd"
                border.color: completeRecoveryButton.enabled ? "#9d2424" : "#858e92"
                border.width: 1
                radius: 4
            }

            contentItem: Text {
                text: completeRecoveryButton.text
                color: completeRecoveryButton.enabled ? "#8f2222" : "#858e92"
                font.pixelSize: 15
                font.bold: true
                fontSizeMode: Text.Fit
                minimumPixelSize: 11
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Text {
            width: parent.width
            text: root.recoveryAllowed ? "Confirm only after TK1 has been physically emptied." : "TECHNICIAN OR ADMIN AUTHORIZATION REQUIRED"
            color: root.recoveryAllowed ? "#596368" : "#8f2222"
            font.pixelSize: 12
            font.bold: !root.recoveryAllowed
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }
    }
}