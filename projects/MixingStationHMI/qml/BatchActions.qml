import QtQuick
import QtQuick.Controls.Basic

Item {
    id: root

    required property var controller
    required property bool hasActiveFault
    required property bool operatorSelected

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
            HmiButton {
                id: pauseButton
                width: (parent.width - 12) / 2
                height: 52
                text: "PAUSE"
                enabled: root.operatorSelected && root.canPause
                onClicked: root.controller.pause()
                background: Rectangle {
                    radius: 4
                    color: pauseButton.enabled ? "#f7f8f8" : "#c5cace"
                    border.color: pauseButton.enabled ? "#596368" : "#a5aaad"
                }

                contentItem: Text {
                    text: pauseButton.text
                    color: pauseButton.enabled ? "#252a2d" : "#879195"
                    font.pixelSize: 16
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            HmiButton {
                id: resumeButton
                width: (parent.width - 12) / 2
                height: 52
                text: "RESUME"
                enabled: root.operatorSelected
                         && root.controller.state === BatchController.Paused
                         && !root.hasActiveFault
                onClicked: root.controller.resume()
                background: Rectangle {
                    radius: 4
                    color: resumeButton.enabled ? "#f7f8f8" : "#c5cace"
                    border.color: resumeButton.enabled ? "#596368" : "#a5aaad"
                }

                contentItem: Text {
                    text: resumeButton.text
                    color: resumeButton.enabled ? "#252a2d" : "#879195"
                    font.pixelSize: 16
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        HmiButton {
            width: parent.width
            height: 52
            text: "START TRANSFER"
            visible: root.controller.state === BatchController.ReadyForTransfer
            enabled: root.operatorSelected
                     && visible
                     && !root.hasActiveFault
                     && root.controller.transferAllowed
            onClicked: root.controller.startTransfer()
        }
        Text {
            visible: root.controller.state === BatchController.ReadyForTransfer
                     && !root.controller.transferAllowed
            text: "TRANSFER BLOCKED · TK1 must be 30–36 °C"
            color: "#8f2222"
            font.pixelSize: 13
        }
        Text {
            visible: !root.operatorSelected
            width: parent.width
            text: "OPERATOR LOGIN REQUIRED TO CONTROL ACTIVE BATCH"
            color: "#8f2222"
            font.pixelSize: 12
            font.bold: true
            wrapMode: Text.WordWrap
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