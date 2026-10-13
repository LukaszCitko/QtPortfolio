import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root

    property bool rpmControlEnabled: false
    property string highlightedTag: ""
    required property bool resetFaultAllowed
    required property var resetService
    required property var drainRequestService
    required property var drainValveDevice
    required property var waterPumpDevice
    required property var waterValveDevice
    required property var concentratePumpDevice
    required property var concentrateValveDevice
    required property var transferPumpDevice
    required property var transferValveDevice
    required property var mixerDevice
    required property var tankDevice
    required property bool operatorSelected
    required property bool canApproveDrain
    required property var batchDevice
    required property bool hardwareMode
    required property var controllerClient

    signal rpmRequested(var device, string tag)

    readonly property bool drainAvailable:
        root.operatorSelected
        && root.tankDevice.volume > 0
        && !root.tankDevice.fault
        && !root.drainValveDevice.open
        && !root.drainValveDevice.fault
        && root.batchDevice.state !== BatchController.Draining
        && (!root.hardwareMode || (root.controllerClient.snapshotValid
        && (root.batchDevice.state === BatchController.Idle || root.batchDevice.state === BatchController.Paused)))

    readonly property var devices: [
        {
            tag: "P1",
            name: "WATER PUMP",
            device: root.waterPumpDevice
        },
        {
            tag: "V1",
            name: "WATER VALVE",
            device: root.waterValveDevice
        },
        {
            tag: "P2",
            name: "CONCENTRATE PUMP",
            device: root.concentratePumpDevice
        },
        {
            tag: "V2",
            name: "CONCENTRATE VALVE",
            device: root.concentrateValveDevice
        },
        {
            tag: "P3",
            name: "TRANSFER PUMP",
            device: root.transferPumpDevice
        },
        {
            tag: "V3",
            name: "TRANSFER VALVE",
            device: root.transferValveDevice
        },
        {
            tag: "M1",
            name: "MIXER",
            device: root.mixerDevice
        },
        {
            tag: "TK1",
            name: "MIXING TANK",
            device: root.tankDevice
        },
        {
            tag: "V4",
            name: "DRAIN VALVE",
            device: root.drainValveDevice
        }
    ]
    readonly property bool valveCard:
        modelData.tag === "V1"
        || modelData.tag === "V2"
        || modelData.tag === "V3"
        || modelData.tag === "V4"

    color: "#eceeef"
    border.color: "#a5aaad"


    Text {
        id: title
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 18
        text: "EQUIPMENT STATUS"
        color: "#252a2d"
        font.pixelSize: 20
        font.bold: true
    }

    Grid {
        id: equipmentGrid
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: title.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.topMargin: 14
        anchors.bottomMargin: 20

        columns: 3
        columnSpacing: 12
        rowSpacing: 12

        Repeater {
            model: root.devices

            Rectangle {
                id: equipmentCard
                readonly property string deviceState: modelData.device.stateText
                readonly property bool fault: modelData.device.fault

                readonly property bool pumpCard:
                    modelData.tag === "P1"
                    || modelData.tag === "P2"
                    || modelData.tag === "P3"

                readonly property bool active:
                    pumpCard
                        ? modelData.device.running
                        : valveCard
                          ? modelData.device.open
                          : modelData.tag === "M1"
                            ? modelData.device.running
                            : modelData.tag === "TK1"
                              ? modelData.device.active
                              : false

                width: (equipmentGrid.width - 24) / 3
                height: (equipmentGrid.height - 24) / 3
                color: fault ? "#f4ddda" : active ? "#f7f8f8" : "#d7dadd"
                border.color: fault ? "#b63838"
                              : modelData.tag === root.highlightedTag ? "#496879"
                              : "#a5aaad"
                border.width: fault ? 2
                              : modelData.tag === root.highlightedTag ? 3
                              : 1

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 14
                    anchors.top: parent.top
                    anchors.topMargin: 10
                    text: modelData.tag + "  ·  " + modelData.name
                    color: "#252a2d"
                    font.pixelSize: 15
                    font.bold: true
                }

                Text {
                    anchors.centerIn: parent
                    text: deviceState
                    color: fault ? "#8f2222" : "#252a2d"
                    font.pixelSize: 20
                    font.bold: true

                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: 14
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 10
                    width: equipmentCard.fault
                           && (modelData.tag === "P1" || modelData.tag === "M1")
                           ? parent.width - 170
                           : parent.width - 28
                    elide: Text.ElideRight

                    text:
                        equipmentCard.fault
                          ? "FAULT ACTIVE · CHECK ALARMS"
                          : pumpCard ?
                            "TARGET " + Math.round(modelData.device.targetRpm) + " RPM · TAP TO SET"
                              : modelData.tag === "M1" ? (modelData.device.connected
                                                       ? Math.round(modelData.device.actualRpm)
                                                         + " / "
                                                         + Math.round(modelData.device.targetRpm)
                                                         + " RPM"
                                                       : "DISCONNECTED")
                              : modelData.tag === "TK1"
                                  ? "VOLUME " + modelData.device.volume.toFixed(1) + " L"
                              : modelData.tag === "V4" ? (root.batchDevice.state === BatchController.Draining
                                                       ? "DRAIN IN PROGRESS"
                                                       : root.drainAvailable
                                                       ? "DRAIN TK1 · TAP TO CONFIRM"
                                                       : root.tankDevice.volume <= 0
                                                       ? "TK1 EMPTY"
                                                       : "DRAIN UNAVAILABLE")
                            : ""

                    color: equipmentCard.fault ? "#8f2222" : "#596368"
                    font.pixelSize: 13
                }
                MouseArea {
                    anchors.fill: parent
                    enabled: root.rpmControlEnabled && pumpCard && !equipmentCard.fault
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.rpmRequested(modelData.device, modelData.tag)
                }
                MouseArea {
                    anchors.fill: parent
                    enabled: modelData.tag === "V4" && root.drainAvailable
                    cursorShape: Qt.PointingHandCursor
                    onClicked: drainDialog.open()
                }
                HmiButton {
                    id: resetFaultButton
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 8
                    width: 140
                    height: 38

                    visible: equipmentCard.fault
                             && (modelData.tag === "P1" || modelData.tag === "M1")
                    enabled: root.resetFaultAllowed
                    text: "RESET FAULT"

                    onClicked: {
                        if (root.hardwareMode)
                        {
                            if (modelData.tag === "P1")
                                root.controllerClient.setSimulatedFault(1, false)
                            else
                                root.controllerClient.setSimulatedFault(64, false)

                            return
                        }

                        if (modelData.tag === "P1")
                            root.resetService.resetPump1()
                        else
                            root.resetService.resetMixer()
                    }

                    background: Rectangle {
                        color: resetFaultButton.enabled ? "#f7f8f8" : "#d7dadd"
                        border.color: "#b63838"
                    }

                    contentItem: Text {
                        text: resetFaultButton.text
                        color: resetFaultButton.enabled ? "#252a2d" : "#596368"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    HmiDialog {
        id: drainDialog
        parent: Overlay.overlay
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 440
        height: 240
        modal: true
        focus: true

        title: root.canApproveDrain ? "CONFIRM DRAIN" : !root.hardwareMode ? "DEMO DRAIN" : "TECHNICIAN AUTHORIZATION REQUIRED"

        background: Rectangle {
            color: "#eceeef"
            border.color: "#858e92"
            radius: 4
        }

        header: Rectangle {
            width: drainDialog.width
            implicitHeight: 56
            color: "#e8e9ea"
            border.color: "#858e92"

            Text {
                anchors.fill: parent
                anchors.leftMargin: 18
                text: drainDialog.title
                color: "#252a2d"
                font.pixelSize: 16
                font.bold: true
                verticalAlignment: Text.AlignVCenter
            }
        }

        contentItem: Text {
            width: drainDialog.availableWidth
            text: root.canApproveDrain
                  ? "Stop the current batch and drain TK1 through V4?"
                  : !root.hardwareMode
                    ? "Use the operator demo override to drain TK1 without technician authorization."
                    : "Drain is unavailable for the selected operator. Technician authorization is required."
            color: "#252a2d"
            font.pixelSize: 16
            wrapMode: Text.WordWrap
            verticalAlignment: Text.AlignVCenter
        }

        footer: Rectangle {
            width: drainDialog.width
            implicitHeight: 64
            color: "#e8e9ea"
            border.color: "#a5aaad"

            Row {
                anchors.right: parent.right
                anchors.rightMargin: 16
                anchors.verticalCenter: parent.verticalCenter
                spacing: 12

                HmiButton {
                    id: demoDrainConfirmButton
                    width: 160
                    height: 42
                    visible: !root.hardwareMode && !root.canApproveDrain
                    enabled: root.drainAvailable
                    text: "DEMO DRAIN"

                    onClicked: {
                        if (root.drainRequestService.demoRequestDrain())
                            drainDialog.close()
                    }

                    background: Rectangle {
                        color: demoDrainConfirmButton.enabled ? "#f7f8f8" : "#c5cace"
                        border.color: demoDrainConfirmButton.enabled ? "#b63838" : "#a5aaad"
                        radius: 4
                    }

                    contentItem: Text {
                        text: demoDrainConfirmButton.text
                        color: demoDrainConfirmButton.enabled ? "#8f2222" : "#879195"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                HmiButton {
                    id: confirmDrainButton
                    width: 150
                    height: 42
                    visible: root.canApproveDrain
                    enabled: root.drainAvailable
                    text: "CONFIRM"

                    onClicked: {
                        if (root.drainRequestService.requestDrain())
                            drainDialog.close()
                    }

                    background: Rectangle {
                        color: confirmDrainButton.enabled ? "#f7f8f8" : "#c5cace"
                        border.color: confirmDrainButton.enabled ? "#596368" : "#a5aaad"
                        radius: 4
                    }

                    contentItem: Text {
                        text: confirmDrainButton.text
                        color: confirmDrainButton.enabled ? "#252a2d" : "#879195"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                HmiButton {
                    id: cancelDrainButton
                    width: 120
                    height: 42
                    text: "CANCEL"
                    onClicked: drainDialog.close()

                    background: Rectangle {
                        color: cancelDrainButton.pressed ? "#c5cace" : "#f2f3f3"
                        border.color: "#858e92"
                        radius: 4
                    }

                    contentItem: Text {
                        text: cancelDrainButton.text
                        color: "#252a2d"
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }
}
