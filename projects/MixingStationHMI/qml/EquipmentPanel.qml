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

    signal rpmRequested(var device, string tag)

    readonly property bool drainAvailable:
        root.operatorSelected
        && root.tankDevice.volume > 0
        && !root.tankDevice.fault
        && !root.drainValveDevice.open
        && !root.drainValveDevice.fault
        && root.batchDevice.state !== BatchController.Draining

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
                Button {
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

    Dialog {
        id: drainDialog
        parent: Overlay.overlay
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 440
        height: 200
        modal: true
        focus: true
        title: root.canApproveDrain
               ? "CONFIRM DRAIN"
               : "TECHNICIAN AUTHORIZATION REQUIRED"

        standardButtons: root.canApproveDrain
                         ? (Dialog.Ok | Dialog.Cancel)
                         : Dialog.Cancel

        Text {
            width: parent.width
            text: root.canApproveDrain
                  ? "Stop the current batch and drain TK1 through V4?"
                  : "Drain is unavailable for the selected operator. Technician authorization is required."
            color: "#252a2d"
            font.pixelSize: 16
            wrapMode: Text.WordWrap
        }

        onAccepted: {
            if (root.drainAvailable)
                root.drainRequestService.requestDrain()
        }
    }
}
