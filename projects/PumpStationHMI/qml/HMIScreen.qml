import QtQuick
import QtQuick.Controls.Basic

ApplicationWindow {
    id: window
    property string activePage: "PROCESS"
    property var acknowledgedFaults: []
    readonly property bool hasUnacknowledgedFault:

    alarmBar.faults.some(function(tag) {
        return acknowledgedFaults.indexOf(tag) === -1
    })

    function openPumpRpm(pumpObject, tag) {
        rpmDialog.pumpDevice = pumpObject
        rpmDialog.pumpTag = tag
        rpmSlider.value = pumpObject.targetRpm
        rpmDialog.open()
    }
    visible: true
    width: 1280
    height: 800
    title: "Mixing Station HMI - DEMO"
    color: "#dadddf"

    Column {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            width: parent.width
            height: 80
            color: "#e8e9ea"

            Text {
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.verticalCenter: parent.verticalCenter
                text: "MIXING STATION #1"
                color: "#252a2d"
                font.pixelSize: 24
                font.bold: true
            }

            Text {
                anchors.centerIn: parent
                text: "STATUS: " + batchController.stateText
                color: "#252a2d"
                font.pixelSize: 20
            }

            Button {
                id: loginButton
                anchors.right: parent.right
                anchors.rightMargin: 24
                anchors.verticalCenter: parent.verticalCenter
                width: 220
                height: 48

                text: operatorSession.selected
                      ? "OPERATOR: " + operatorSession.operatorName
                      : "LOGIN"

                enabled: batchController.stateText === "IDLE"
                         || batchController.stateText === "COMPLETE"

                onClicked: operatorDialog.open()

                background: Rectangle {
                    color: "#f7f8f8"
                    border.color: "#858e92"
                    radius: 4
                }

                contentItem: Text {
                    text: loginButton.text
                    color: loginButton.enabled ? "#252a2d" : "#596368"
                    font.pixelSize: 15
                    font.bold: true
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        //QML Top Strip

        StageStrip {
            width: parent.width
            height: 64
            currentStage: batchController.stageIndex
        }


        Rectangle {
            width: parent.width
            height: 488
            color: "#dadddf"

            Row {
                visible: window.activePage === "PROCESS"
                anchors.fill: parent
                anchors.margins: 24
                spacing: 24

                Rectangle {
                    width: 320
                    height: parent.height
                    color: "#eceeef"
                    border.color: "#a5aaad"

                    Column {
                        width: parent.width - 28
                        anchors.centerIn: parent
                        spacing: 24

                        EquipmentLane {
                            width: parent.width
                            title: "WATER →   "
                            pumpTag: "P1"
                            valveTag: "V1"
                            flowArrow: "→"
                            rpmControlEnabled: operatorSession.selected
                            onRpmRequested: (device, tag) => window.openPumpRpm(device, tag)
                            pumpDevice: pump
                            valveDevice: valve1
                        }

                        EquipmentLane {
                            width: parent.width
                            title: "CONCENTRATE →   "
                            pumpTag: "P2"
                            valveTag: "V2"
                            flowArrow: "→"
                            rpmControlEnabled: operatorSession.selected
                            onRpmRequested: (device, tag) => window.openPumpRpm(device, tag)
                            pumpDevice: pump2
                            valveDevice: valve2
                        }

                        EquipmentLane {
                            width: parent.width
                            title: "← PRODUCT   "
                            pumpTag: "P3"
                            valveTag: "V3"
                            flowArrow: "←"
                            rpmControlEnabled: operatorSession.selected
                            onRpmRequested: (device, tag) => window.openPumpRpm(device, tag)
                            pumpDevice: pump3
                            valveDevice: valve3
                        }
                    }
                }

                TankView {
                    width: 508
                    height: parent.height
                    tank: mixingTank
                    mixerDevice: mixer
                }
                ProcessValues {
                    width: 356
                    height: parent.height
                    tank: mixingTank
                    controller: batchController
                    hasActiveFault: alarmBar.hasFault
                    operatorSelected: operatorSession.selected
                }
            }
            EquipmentPanel {
                anchors.fill: parent
                anchors.margins: 24
                visible: window.activePage === "EQUIPMENT"
                rpmControlEnabled: operatorSession.selected
                onRpmRequested: (device, tag) => window.openPumpRpm(device, tag)
            }
            SimulationControlPanel {
                anchors.fill: parent
                anchors.margins: 24
                visible: window.activePage === "SIMULATION"
                mixerDevice: mixer
                tankDevice: mixingTank
                waterPumpDevice: pump
                batchDevice: batchController
            }
            TrendPanel {
                anchors.fill: parent
                anchors.margins: 24
                visible: window.activePage === "TRENDS"
            }
            Rectangle {
                id: eventManagerHistory
                anchors.fill: parent
                anchors.margins: 24
                visible: window.activePage === "HISTORY"
                color: "#eceeef"
                border.color: "#a5aaad"

                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Text {
                        text: "EVENT HISTORY"
                        color: "#252a2d"
                        font.pixelSize: 20
                        font.bold: true
                    }

                    Text {
                        visible: eventManager.eventCount === 0
                        text: "No events recorded"
                        color: "#596368"
                        font.pixelSize: 16
                    }

                    Flickable {
                        width: parent.width
                        height: parent.height - 44
                        clip: true
                        contentHeight: eventColumn.height

                        Column {
                            id: eventColumn
                            width: parent.width
                            spacing: 4

                            Repeater {
                                model: eventManager.eventCount

                                Rectangle {
                                    property int eventIndex: eventManager.eventCount - 1 - index

                                    width: eventColumn.width
                                    height: 44
                                    color: "#f7f8f8"
                                    border.color: "#c4c9cb"

                                    Text {
                                        anchors.fill: parent
                                        anchors.leftMargin: 12
                                        anchors.rightMargin: 12
                                        verticalAlignment: Text.AlignVCenter
                                        elide: Text.ElideRight
                                        color: "#252a2d"
                                        font.pixelSize: 15

                                        text: eventManager.eventTime(eventIndex)
                                              + "   " + eventManager.eventLevel(eventIndex)
                                              + "   " + eventManager.eventSource(eventIndex)
                                              + "   " + eventManager.eventMessage(eventIndex)
                                    }
                                }
                            }
                        }
                    }
                }
            }
            Rectangle {
                id: eventManagerAlarms
                anchors.fill: parent
                anchors.margins: 24
                visible: window.activePage === "ALARMS"
                color: "#eceeef"
                border.color: "#a5aaad"

                Column {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    //Active Alarms
                    Row {
                        width: parent.width
                        height: 44
                        spacing: 20

                        Text {
                            width: parent.width - 240
                            height: parent.height
                            text: "ACTIVE ALARMS"
                            color: "#252a2d"
                            font.pixelSize: 20
                            font.bold: true
                            verticalAlignment: Text.AlignVCenter
                        }

                        Button {
                            width: 220
                            height: 44
                            text: "ACKNOWLEDGE"
                            enabled: window.hasUnacknowledgedFault
                                     && operatorSession.selected

                            onClicked: {
                                const newAcknowledgements =
                                    alarmBar.faults.filter(function(tag) {
                                        return window.acknowledgedFaults.indexOf(tag) === -1
                                    })

                                if (newAcknowledgements.length === 0)
                                    return

                                window.acknowledgedFaults = alarmBar.faults.slice()

                                eventManager.addInfo(
                                    "ALARM",
                                    "Acknowledged " + newAcknowledgements.join(", ")
                                    + " by " + operatorSession.operatorName)
                            }
                        }
                    }

                    Text {
                        visible: !alarmBar.hasFault
                        text: "No active alarms"
                        color: "#596368"
                        font.pixelSize: 16
                    }

                    Flickable {
                        width: parent.width
                        height: parent.height - 60
                        clip: true
                        contentHeight: activeAlarmColumn.height
                        visible: alarmBar.hasFault


                        Column {
                            id: activeAlarmColumn
                            width: parent.width
                            spacing: 4

                            Repeater {
                                model: alarmBar.faults

                                Rectangle {
                                    width: activeAlarmColumn.width
                                    height: 44
                                    color: "#f4ddda"
                                    border.color: "#b63838"

                                    Text {
                                        anchors.fill: parent
                                        anchors.leftMargin: 12
                                        verticalAlignment: Text.AlignVCenter
                                        text: "ALARM  ·  " + modelData + "  ·  "
                                              + (window.acknowledgedFaults.indexOf(modelData) !== -1
                                                 ? "ACKNOWLEDGED · FAULT ACTIVE"
                                                 : "UNACKNOWLEDGED · FAULT ACTIVE")
                                        color: "#8f2222"
                                        font.pixelSize: 16
                                        font.bold: true
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        AlarmBar {
            id: alarmBar
            width: parent.width
            height: 80
            onFaultsChanged: {
                const activeFaults = faults
                window.acknowledgedFaults =
                    window.acknowledgedFaults.filter(function(tag) {
                        return activeFaults.indexOf(tag) !== -1
                    })
            }
        }

        Rectangle {
            width: parent.width
            height: 88
            color: "#d0d3d5"

            Row {
                id: navigationRow
                anchors.fill: parent
                anchors.margins: 12
                spacing: 8

                Repeater {
                    model: [
                        "PROCESS",
                        "TRENDS",
                        "HISTORY",
                        "EQUIPMENT",
                        "ALARMS",
                        "SIMULATION"
                    ]

                    Rectangle {
                        id: navigationButton

                        property bool blinkOn: true

                        width: (navigationRow.width - 5 * navigationRow.spacing) / 6
                        height: navigationRow.height
                        color: window.activePage === modelData ? "#f6f7f7" : "#d0d3d5"
                        border.width: modelData === "ALARMS" && window.hasUnacknowledgedFault ? 3 : 1
                            border.color: modelData === "ALARMS" && window.hasUnacknowledgedFault
                                          ? (blinkOn ? "#b63838" : "#a5aaad")
                                          : "#a5aaad"

                            Timer {
                                interval: 650
                                repeat: true
                                running: modelData === "ALARMS" && window.hasUnacknowledgedFault
                                onTriggered: navigationButton.blinkOn = !navigationButton.blinkOn
                            }

                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            color: modelData === "PROCESS"
                                    || modelData === "SIMULATION"
                                    || modelData === "HISTORY"
                                    || modelData === "EQUIPMENT"
                                    || modelData === "TRENDS"
                                    || modelData === "ALARMS"
                                    ? "#252a2d" : "#879195"
                            font.pixelSize: 16
                            font.bold: window.activePage === modelData
                        }

                        MouseArea {
                            anchors.fill: parent
                            enabled: modelData === "PROCESS"
                                    || modelData === "SIMULATION"
                                    || modelData === "HISTORY"
                                    || modelData === "EQUIPMENT"
                                    || modelData === "TRENDS"
                                    || modelData === "ALARMS"
                            onClicked: window.activePage = modelData
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: operatorDialog
        width: 280
        height: 340
        x: (window.width - width) / 2
        y: (window.height - height) / 2
        modal: true
        focus: true
        title: "SELECT DEMO OPERATOR"

        Column {
            width: parent.width
            spacing: 12

            Repeater {
                model: [    "Operator 01", "Operator 02"]

                Button {
                    width: operatorDialog.availableWidth
                    height: 64
                    text: modelData

                    onClicked: {
                        operatorSession.selectOperator(modelData)
                        operatorDialog.close()
                    }
                }
            }

            Text {
                text: "Demo selection only · card authentication will be added later"
                width: parent.width
                wrapMode: Text.WordWrap
                color: "#596368"
                font.pixelSize: 13
            }
            Button {
                width: operatorDialog.availableWidth
                height: 52
                text: "CANCEL"
                onClicked: operatorDialog.close()
            }
        }
    }

    Dialog {
        id: rpmDialog
        parent: Overlay.overlay
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 420
        height: 250
        modal: true
        focus: true

        property var pumpDevice: null
        property string pumpTag: ""

        title: pumpTag + " · TARGET RPM"
        standardButtons: Dialog.Ok | Dialog.Cancel

        Column {
            width: rpmDialog.availableWidth
            spacing: 16

            Text {
                text: "TARGET: " + Math.round(rpmSlider.value) + " RPM"
                color: "#252a2d"
                font.pixelSize: 20
                font.bold: true
            }

            Slider {
                id: rpmSlider
                width: parent.width
                from: 300  // Keep a running batch from losing all flow.
                to: 1800
                stepSize: 50
                snapMode: Slider.SnapAlways
            }

            Text {
                text: "CURRENT: "
                      + (rpmDialog.pumpDevice
                         ? Math.round(rpmDialog.pumpDevice.actualRpm) : 0)
                      + " RPM"
                color: "#596368"
                font.pixelSize: 16
            }
        }

        onAccepted: {
            if (pumpDevice)
                pumpDevice.setTargetRpm(rpmSlider.value)
        }
    }
}