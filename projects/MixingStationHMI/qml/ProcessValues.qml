import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root

    required property bool hardwareMode
    required property bool recoveryAllowed
    required property bool operatorSelected
    required property string operatorName
    required property var tank
    required property var controller
    required property bool hasActiveFault
    required property var mixerDevice
    required property var pump1Device
    required property var pump2Device
    required property var pump3Device

    readonly property var activePumpDevice: root.pump1Device.running ?
                                                root.pump1Device : root.pump2Device.running ?
                                                root.pump2Device : root.pump3Device.running ?
                                                root.pump3Device : null

    readonly property string activePumpTag: root.pump1Device.running ? "P1" : root.pump2Device.running ? "P2" : root.pump3Device.running ? "P3" : ""

    color: "#eceeef"
    border.color: "#a5aaad"

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 14

        Grid {
            columns: 2
            columnSpacing: 12
            rowSpacing: 13

            Text {
                width: 176
                text: "Run ID"
                color: "#252a2d"
                font.pixelSize: 18
            }
            Text {
                width: 120
                text: root.controller.batchId === "" ? "—" : root.controller.batchId
                horizontalAlignment: Text.AlignRight
                color: "#252a2d"
                font.pixelSize: 18
                fontSizeMode: Text.Fit
                minimumPixelSize: 12
            }

            Text {
                width: 176
                text: (root.tank.temperature < 30.0 && root.tank.volume !== 0) ?
                        "Temperature (TOO LOW)" : root.tank.temperature > 36.0
                        ? "Temperature (TOO HIGH)": "Temperature"

                color: ((root.tank.temperature < 30.0 || root.tank.temperature > 36.0) && root.tank.volume !== 0) ? "#b63838": "#252a2d"

                font.pixelSize: 17
            }
            Text {
                width: 120
                text: root.tank.temperature.toFixed(1) + " °C"
                horizontalAlignment: Text.AlignRight
                color: "#252a2d"
                font.pixelSize: 17
            }

            Text {
                width: 176
                text: "Water added"
                color: "#252a2d"
                font.pixelSize: 17
            }
            Text {
                width: 120
                text: root.tank.waterVolume.toFixed(1) + " L"
                horizontalAlignment: Text.AlignRight
                color: "#252a2d"
                font.pixelSize: 17
            }

            Text {
                width: 176
                text: "Concentrate added"
                color: "#252a2d"
                font.pixelSize: 17
            }
            Text {
                width: 120
                text:  root.tank.concentrateVolume.toFixed(1) + " L"
                horizontalAlignment: Text.AlignRight
                color: "#252a2d"
                font.pixelSize: 17
            }

            Text {
                width: 176
                text: "Mixture volume"
                color: "#252a2d"
                font.pixelSize: 17
            }
            Text {
                width: 120
                text: root.tank.volume.toFixed(1) + " L"
                horizontalAlignment: Text.AlignRight
                color: "#252a2d"
                font.pixelSize: 18
            }


            Text {
                width: 176
                text: "Mixer speed"
                color: "#252a2d"
                font.pixelSize: 18
            }
            Text {
                width: 120
                text: root.mixerDevice.actualRpm > 0 ? Math.round(root.mixerDevice.actualRpm) + " RPM" : "—"
                horizontalAlignment: Text.AlignRight
                color: "#252a2d"
                font.pixelSize: 18
            }
            Text {
                width: 176
                text: root.activePumpDevice ? "Pump: " + root.activePumpTag + " speed" : "Pump speed"
                color: "#252a2d"
                font.pixelSize: 18
            }
            Text {
                width: 120
                text: root.activePumpDevice ? Math.round(root.activePumpDevice.actualRpm) + " RPM" : "—"
                horizontalAlignment: Text.AlignRight
                color: "#252a2d"
                font.pixelSize: 18
            }


        }

        PreCheckPanel {
            width: parent.width
            height: 184
            visible: root.controller.state === BatchController.Idle

            controller: root.controller
            operatorSelected: root.operatorSelected
            operatorName: root.operatorName
            hardwareMode: root.hardwareMode
            recoveryAllowed: root.recoveryAllowed
        }
        BatchActions {
            width: parent.width
            height: 184
            operatorSelected: root.operatorSelected
            controller: root.controller
            hasActiveFault: root.hasActiveFault
            visible: root.controller.state !== BatchController.Idle
                     && root.controller.state !== BatchController.Complete
        }
        HmiButton {
            id: nextBatchButton
            width: parent.width
            height: 56
            visible: root.controller.state === BatchController.Complete
            text: "PREPARE NEXT BATCH"
            enabled: root.operatorSelected
            onClicked: root.controller.prepareNextBatch()

            background: Rectangle {
                color: "#f7f8f8"
                border.color: "#858e92"
                radius: 4
            }

            contentItem: Text {
                text: nextBatchButton.text
                color: "#252a2d"
                font.pixelSize: 16
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}