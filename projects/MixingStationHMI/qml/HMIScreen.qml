import QtQuick
import QtQuick.Controls.Basic

ApplicationWindow {
    id: window
    property string activePage: "PROCESS"
    property string highlightedEquipmentTag: ""
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
                      ? operatorSession.operatorName + "\n" + operatorSession.operatorRole
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
            Button {
                id: manageUsersButton
                anchors.right: loginButton.left
                anchors.rightMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                width: 160
                height: 48

                visible: operatorSession.canManageUsers
                enabled: loginButton.enabled
                text: "USERS"

                onClicked: userManagementDialog.open()

                background: Rectangle {
                    color: manageUsersButton.enabled ? "#f7f8f8" : "#d7dadd"
                    border.color: "#858e92"
                    radius: 4
                }

                contentItem: Text {
                    text: manageUsersButton.text
                    color: manageUsersButton.enabled ? "#252a2d" : "#596368"
                    font.pixelSize: 14
                    maximumLineCount: 2
                    wrapMode: Text.NoWrap
                    font.bold: true
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
                            pumpDevice: pump1
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
                    onMixerRequested: {
                        window.highlightedEquipmentTag = "M1"
                        window.activePage = "EQUIPMENT"
                    }
                }
                ProcessValues {
                    width: 356
                    height: parent.height

                    tank: mixingTank
                    mixerDevice: mixer
                    controller: batchController
                    hasActiveFault: alarmBar.hasFault
                    operatorSelected: operatorSession.selected
                    operatorName: operatorSession.operatorName
                }
            }
            EquipmentPanel {
                anchors.fill: parent
                anchors.margins: 24
                visible: window.activePage === "EQUIPMENT"
                rpmControlEnabled: operatorSession.selected
                onRpmRequested: (device, tag) => window.openPumpRpm(device, tag)
                resetFaultAllowed: operatorSession.canResetFault
                resetService: faultResetService
                drainRequestService: drainService
                highlightedTag: window.highlightedEquipmentTag

                waterPumpDevice: pump1
                waterValveDevice: valve1
                concentratePumpDevice: pump2
                concentrateValveDevice: valve2
                transferPumpDevice: pump3
                transferValveDevice: valve3
                mixerDevice: mixer
                tankDevice: mixingTank
                drainValveDevice: valve4
                operatorSelected: operatorSession.selected
                canApproveDrain: operatorSession.canApproveDrain
                batchDevice: batchController
            }
            SimulationControlPanel {
                anchors.fill: parent
                anchors.margins: 24
                visible: window.activePage === "SIMULATION"
                mixerDevice: mixer
                tankDevice: mixingTank
                waterPumpDevice: pump1
                batchDevice: batchController
                demoOverrideEnabled: operatorSession.operatorRole === "OPERATOR"
                resetService: faultResetService

            }
            TrendPanel {
                anchors.fill: parent
                anchors.margins: 24
                visible: window.activePage === "TRENDS"
                liveSource: trendRecorder
                historySource: trendHistorySource
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

                    Row {
                        width: parent.width
                        height: 48
                        spacing: 12

                        Text {
                            width: parent.width
                                   - (eventHistoryModel.runFilter === "" ? 192 : 384)
                            height: parent.height
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            color: "#252a2d"
                            font.pixelSize: 20
                            font.bold: true

                            text: eventHistoryModel.runFilter === ""
                                  ? "EVENT HISTORY · ALL EVENTS"
                                  : "EVENT HISTORY · RUN "
                                    + eventHistoryModel.runFilter.substring(0, 8)
                        }

                        Button {
                            width: 180
                            height: parent.height
                            text: "SELECT RUN"
                            onClicked: runSelectionDialog.open()
                        }

                        Button {
                            width: 180
                            height: parent.height
                            visible: eventHistoryModel.runFilter !== ""
                            text: "SHOW TRENDS"

                            onClicked: {
                                if (trendHistorySource.loadRun(eventHistoryModel.runFilter))
                                    window.activePage = "TRENDS"
                            }
                        }
                    }

                    Text {
                        id: noEventsText
                        visible: historyRepeater.count === 0
                        text: eventHistoryModel.runFilter === ""
                              ? "No events recorded"
                              : "No events for this run"
                        color: "#596368"
                        font.pixelSize: 16
                    }

                    Flickable {
                        width: parent.width
                        height: parent.height - 64
                                - (noEventsText.visible ? noEventsText.implicitHeight + 16 : 0)
                        clip: true
                        contentHeight: eventColumn.height

                        Column {
                            id: eventColumn
                            width: parent.width
                            spacing: 4

                            Repeater {
                                id: historyRepeater
                                model: eventHistoryModel

                                delegate: Rectangle {
                                    id: eventRow

                                    required property string time
                                    required property string level
                                    required property string source
                                    required property string message

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

                                        text: eventRow.time
                                              + "   " + eventRow.level
                                              + "   " + eventRow.source
                                              + "   " + eventRow.message
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

            eventSource: eventManager

            monitoredDevices: [
                { tag: "P1", device: pump1 },
                { tag: "V1", device: valve1 },
                { tag: "P2", device: pump2 },
                { tag: "V2", device: valve2 },
                { tag: "P3", device: pump3 },
                { tag: "V3", device: valve3 },
                { tag: "M1", device: mixer },
                { tag: "TK1", device: mixingTank },
                { tag: "V4", device: valve4 }
            ]

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
                            onClicked: {
                                window.highlightedEquipmentTag = ""
                                window.activePage = modelData
                            }
                        }
                    }
                }
            }
        }
    }
    UserManagementDialog {
        id: userManagementDialog
        usersModel: userListModel
        managementService: userManagementService
        currentUserId: operatorSession.operatorId
    }
    Dialog {
        id: operatorDialog
        parent: Overlay.overlay
        width: 420
        height: 480
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        modal: true
        focus: true
        title: "SELECT DEMO USER"
        standardButtons: Dialog.Cancel

        contentItem: Column {

            spacing: 12

            ListView {
                id: userListView
                width: parent.width
                height: Math.max(0,parent.height - demoNote.implicitHeight - parent.spacing)
                clip: true
                spacing: 8
                model: userListModel

                ScrollBar.vertical: ScrollBar {}

                delegate: Button {
                    required property string userId
                    required property string displayName
                    required property string role

                    width: userListView.width
                    height: 64
                    text: displayName + " · " + role

                    onClicked: {
                        if (operatorSession.selectOperator(userId))
                            operatorDialog.close()
                    }
                }
            }

            Text {
                id: demoNote
                width: parent.width
                wrapMode: Text.WordWrap
                color: "#596368"
                font.pixelSize: 13
                text: "Demo selection only · card authentication will be added later"
            }

        }
    }
    Dialog {
        id: runSelectionDialog
        parent: Overlay.overlay
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 720
        height: 500
        modal: true
        focus: true
        title: "SELECT RUN"

        onOpened: runListModel.refresh()

        Column {
            width: runSelectionDialog.availableWidth
            height: runSelectionDialog.availableHeight
            spacing: 8

            Button {
                width: parent.width
                height: 54
                text: "ALL EVENTS"

                onClicked: {
                    if (eventHistoryModel.selectRun(""))
                        runSelectionDialog.close()
                }
            }

            ListView {
                id: runListView
                width: parent.width
                height: parent.height - 62
                clip: true
                model: runListModel

                delegate: ItemDelegate {
                    required property string runId
                    required property string kind
                    required property string operatorName
                    required property string startedAt
                    required property string outcome

                    width: runListView.width
                    height: 58
                    font.pixelSize: 16

                    text: startedAt + " · " + kind
                          + " · " + (outcome === "" ? "IN PROGRESS" : outcome)
                          + (operatorName === "" ? "" : " · " + operatorName)

                    onClicked: {
                        if (eventHistoryModel.selectRun(runId))
                            runSelectionDialog.close()
                    }
                }
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