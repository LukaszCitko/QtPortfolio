import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root

    required property var controller
    required property bool hasActiveFault

    readonly property bool canPause:
        root.controller.state === BatchController.FillingWater
        || root.controller.state === BatchController.DosingConcentrate
        || root.controller.state === BatchController.TemperatureCheck
        || root.controller.state === BatchController.Mixing
        || root.controller.state === BatchController.Transferring

    Column {
        anchors.fill: parent
        spacing: 12

        // Keeps the batch controls aligned below the process values.
        Item {
            width: parent.width
            height: 18
        }

        Text {
            visible: root.controller.stageIndex === 4
            text: root.controller.state === BatchController.ReadyForTransfer
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
            visible: root.controller.state !== BatchController.ReadyForTransfer
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
                enabled: root.controller.state === BatchController.Paused
                         && !root.hasActiveFault
                onClicked: root.controller.resume()
            }
        }

        Button {
            width: parent.width
            height: 52
            text: "START TRANSFER"
            visible: root.controller.state === BatchController.ReadyForTransfer
            enabled: visible && !root.hasActiveFault
                     && root.controller.transferAllowed
            onClicked: root.controller.startTransfer()
        }
        Text {
            visible: root.controller.state === BatchController.ReadyForTransfer
                     && !root.controller.transferAllowed
            text: "TRANSFER BLOCKED · TK1 must be 58–62 °C"
            color: "#8f2222"
            font.pixelSize: 13
        }
        Text {
            visible: root.controller.state === BatchController.Paused
                     && root.hasActiveFault
            text: "Clear the active fault before resuming."
            color: "#8f2222"
            font.pixelSize: 12
        }
    }
}