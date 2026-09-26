import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root

    required property var controller
    required property bool hasActiveFault

    readonly property bool canPause:
        controller.stateText === "FILLING WATER"
        || controller.stateText === "DOSING CONCENTRATE"
        || controller.stateText === "TEMPERATURE CHECK"
        || controller.stateText === "MIXING"
        || controller.stateText === "TRANSFERRING"

    Column {
        anchors.fill: parent
        spacing: 12

        Text {
            text: "BATCH CONTROLS"
            color: "#252a2d"
            font.pixelSize: 14
            font.bold: true
        }
        Text {
            visible: root.controller.stageIndex === 4
            text: root.controller.stateText === "READY FOR TRANSFER"
                  ? "MIXING COMPLETE · M1 RUNNING"
                  : "MIXING REMAINING  "
                    + root.controller.mixingSecondsRemaining + " s"
            color: "#252a2d"
            font.pixelSize: 16
            font.bold: true
        }

        Row {
            width: parent.width
            spacing: 12
            visible: root.controller.stateText !== "READY FOR TRANSFER"

            Button {
                width: (parent.width - 12) / 2
                height: 52
                text: "PAUSE"
                enabled: root.canPause
                onClicked: root.controller.pause()
            }

            Button {
                width: (parent.width - 12) / 2
                height: 52
                text: "RESUME"
                enabled: root.controller.stateText === "PAUSED"
                         && !root.hasActiveFault
                onClicked: root.controller.resume()
            }
        }

        Button {
            width: parent.width
            height: 52
            text: "START TRANSFER"
            visible: root.controller.stateText === "READY FOR TRANSFER"
            enabled: visible && !root.hasActiveFault
            onClicked: root.controller.startTransfer()
        }

        Text {
            visible: root.controller.stateText === "PAUSED"
                     && root.hasActiveFault
            text: "Clear the active fault before resuming."
            color: "#8f2222"
            font.pixelSize: 12
        }
    }
}