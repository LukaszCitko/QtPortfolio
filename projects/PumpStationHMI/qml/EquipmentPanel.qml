import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root

    property bool rpmControlEnabled: false
    signal rpmRequested(var device, string tag)

    readonly property bool drainAvailable:
        operatorSession.selected
        && mixingTank.volume > 0
        && mixingTank.stateText !== "FAULT"
        && valve4.stateText === "CLOSED"
        && batchController.stateText !== "DRAINING"

    color: "#eceeef"
    border.color: "#a5aaad"

    readonly property var devices: [
        { tag: "P1", name: "WATER PUMP", device: pump },
        { tag: "V1", name: "WATER VALVE", device: valve1 },
        { tag: "P2", name: "CONCENTRATE PUMP", device: pump2 },
        { tag: "V2", name: "CONCENTRATE VALVE", device: valve2 },
        { tag: "P3", name: "TRANSFER PUMP", device: pump3 },
        { tag: "V3", name: "TRANSFER VALVE", device: valve3 },
        { tag: "M1", name: "MIXER", device: mixer },
        { tag: "TK1", name: "MIXING TANK", device: mixingTank },
        { tag: "V4", name: "DRAIN VALVE", device: valve4 }
    ]

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
                readonly property string deviceState: modelData.device.stateText
                readonly property bool fault: deviceState === "FAULT"

                readonly property bool pumpCard:
                    modelData.tag === "P1"
                    || modelData.tag === "P2"
                    || modelData.tag === "P3"

                readonly property bool active:
                    deviceState === "RUNNING"
                    || deviceState === "OPEN"
                    || deviceState === "FILLING"
                    || deviceState === "MIXING"
                    || deviceState === "TRANSFERRING"

                width: (equipmentGrid.width - 24) / 3
                height: (equipmentGrid.height - 24) / 3
                color: fault ? "#f4ddda" : active ? "#f7f8f8" : "#d7dadd"
                border.color: fault ? "#b63838" : "#a5aaad"
                border.width: fault ? 2 : 1

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

                    text: pumpCard ?
                            "TARGET " + Math.round(modelData.device.targetRpm) + " RPM · TAP TO SET"
                            : modelData.tag === "M1" ? (mixer.connected
                                                     ? Math.round(mixer.actualRpm) + " / " + Math.round(mixer.targetRpm) + " RPM"
                                                     : "DISCONNECTED")
                            : modelData.tag === "TK1" ? "VOLUME " + mixingTank.volume.toFixed(1) + " L"
                            : modelData.tag === "V4" ? (batchController.stateText === "DRAINING"
                                                    ? "DRAIN IN PROGRESS" : root.drainAvailable
                                                    ? "DRAIN TK1 · TAP TO CONFIRM"
                                                    : mixingTank.volume <= 0 ? "TK1 EMPTY" : "DRAIN UNAVAILABLE") // UNAVAILABLE FOR FAULT V4
                            : ""

                    color: "#596368"
                    font.pixelSize: 13
                }
                MouseArea {
                    anchors.fill: parent
                    enabled: root.rpmControlEnabled && pumpCard
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.rpmRequested(modelData.device, modelData.tag)
                }
                MouseArea {
                    anchors.fill: parent
                    enabled: modelData.tag === "V4" && root.drainAvailable
                    cursorShape: Qt.PointingHandCursor
                    onClicked: drainDialog.open()
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
        title: "CONFIRM DRAIN"
        standardButtons: Dialog.Ok | Dialog.Cancel

        Text {
            width: parent.width
            text: "Stop the current batch and drain TK1 through V4?"
            color: "#252a2d"
            font.pixelSize: 16
            wrapMode: Text.WordWrap
        }

        onAccepted: {
            if (root.drainAvailable)
                batchController.emergencyDrain()
        }
    }
}
