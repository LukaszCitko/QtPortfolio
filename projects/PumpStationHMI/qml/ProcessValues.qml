import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root

    required property var tank
    required property var controller
    required property bool hasActiveFault
    required property bool operatorSelected

    color: "#eceeef"
    border.color: "#a5aaad"

    Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Text {
            text: "PROCESS VALUES"
            color: "#252a2d"
            font.pixelSize: 20
            font.bold: true
        }

        Grid {
            columns: 2
            columnSpacing: 12
            rowSpacing: 14

            Text {
                width: 176
                text: "Temperature"
                color: "#252a2d"
                font.pixelSize: 16
            }
            Text {
                width: 120
                text: root.tank.temperature.toFixed(1) + " °C"
                horizontalAlignment: Text.AlignRight
                color: "#252a2d"
                font.pixelSize: 20
            }

            Text {
                width: 176
                text: "Water added"
                color: "#252a2d"
                font.pixelSize: 16
            }
            Text {
                width: 120
                text: root.tank.waterVolume.toFixed(1) + " L"
                horizontalAlignment: Text.AlignRight
                color: "#252a2d"
                font.pixelSize: 20
            }

            Text {
                width: 176
                text: "Concentrate added"
                color: "#252a2d"
                font.pixelSize: 16
            }
            Text {
                width: 120
                text: root.tank.concentrateVolume.toFixed(1) + " L"
                horizontalAlignment: Text.AlignRight
                color: "#252a2d"
                font.pixelSize: 20
            }

            Text {
                width: 176
                text: "Mixture volume"
                color: "#252a2d"
                font.pixelSize: 16
            }
            Text {
                width: 120
                text: root.tank.volume.toFixed(1) + " L"
                horizontalAlignment: Text.AlignRight
                color: "#252a2d"
                font.pixelSize: 20
            }
        }
        PreCheckPanel {
            width: parent.width
            height: 184
            visible: batchController.stateText === "IDLE"
            operatorSelected: root.operatorSelected
        }
        BatchActions {
            width: parent.width
            height: 184
            controller: root.controller
            hasActiveFault: root.hasActiveFault
            visible: root.controller.stateText !== "IDLE"
                     && root.controller.stateText !== "COMPLETE"
        }
        Button {
            id: nextBatchButton
            width: parent.width
            height: 56
            visible: root.controller.stateText === "COMPLETE"
            text: "PREPARE NEXT BATCH"

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