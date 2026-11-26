import QtQuick
import QtQuick.Controls.Basic

ApplicationWindow {
    id: window
    readonly property bool isHardwareMode: hardwareMode
    readonly property var processControllerClient: controllerClient
    property string activePage: "PROCESS"
    property string highlightedEquipmentTag: ""
    property var acknowledgedFaults: []
    readonly property bool controllerReady: !window.isHardwareMode || batchController.connected

    readonly property bool operatorChangeAllowed:
        !operatorSession.selected
        || batchController.state === BatchController.Idle
        || batchController.state === BatchController.Complete
        || alarmBar.hasFault
        || (window.isHardwareMode
            && (!window.controllerReady || batchController.recoveryRequired))

    readonly property bool hasUnacknowledgedFault:
        alarmBar.faults.some(function(tag) {
            return acknowledgedFaults.indexOf(tag) === -1
        })

    function openPumpRpm(pumpObject, tag) {
        rpmDialog.openFor(pumpObject, tag)
    }

    function openEquipment(tag) {
        highlightedEquipmentTag = tag
        activePage = "EQUIPMENT"
    }

    visible: true
    width: 1280
    height: 800
    title: "Mixing Station HMI - DEMO"
    color: "#dadddf"
    palette.window: "#eceeef"
    palette.windowText: "#252a2d"
    palette.base: "#f6f7f7"
    palette.alternateBase: "#e8e9ea"
    palette.text: "#252a2d"
    palette.button: "#f2f3f3"
    palette.buttonText: "#252a2d"
    palette.highlight: "#879195"
    palette.highlightedText: "#ffffff"
    palette.light: "#f7f8f8"
    palette.mid: "#a5aaad"
    palette.dark: "#596368"

    Column {
        anchors.fill: parent
        spacing: 0

        HmiHeader {
            width: parent.width

            controllerReady: window.controllerReady
            statusText: window.controllerReady
                        ? "STATUS: " + batchController.stateText
                        : (window.isHardwareMode && controllerClient.heartbeatStalled
                           ? "STATUS: CONTROLLER HEARTBEAT STOPPED"
                           : "STATUS: CONTROLLER OFFLINE")

            historyStatusText: persistenceRetryQueue.hasPermanentFailure
                               ? "HISTORY SAVE ERROR · CHECK LOGS"
                               : (persistenceRetryQueue.hasPendingWrites
                                  ? "HISTORY SAVE PENDING"
                                  : "")

            loginText: operatorSession.selected
                       ? operatorSession.operatorName + "\n" + operatorSession.operatorRole
                       : "LOGIN"
            operatorChangeAllowed: window.operatorChangeAllowed
            canManageUsers: operatorSession.canManageUsers

            onLoginRequested: operatorDialog.open()
            onUsersRequested: userManagementDialog.open()
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
                            rpmControlEnabled: operatorSession.selected && window.controllerReady
                            onRpmRequested: (device, tag) => window.openPumpRpm(device, tag)
                            pumpDevice: pump1
                            valveDevice: valve1
                            onFaultRequested: function(tag) {
                                window.openEquipment(tag)
                            }
                        }

                        EquipmentLane {
                            width: parent.width
                            title: "CONCENTRATE →   "
                            pumpTag: "P2"
                            valveTag: "V2"
                            flowArrow: "→"
                            rpmControlEnabled: operatorSession.selected && window.controllerReady
                            onRpmRequested: (device, tag) => window.openPumpRpm(device, tag)
                            pumpDevice: pump2
                            valveDevice: valve2
                            onFaultRequested: function(tag) {
                                window.openEquipment(tag)
                            }
                        }

                        EquipmentLane {
                            width: parent.width
                            title: "← PRODUCT   "
                            pumpTag: "P3"
                            valveTag: "V3"
                            flowArrow: "←"
                            rpmControlEnabled: operatorSession.selected && window.controllerReady
                            onRpmRequested: (device, tag) => window.openPumpRpm(device, tag)
                            pumpDevice: pump3
                            valveDevice: valve3
                            onFaultRequested: function(tag) {
                                window.openEquipment(tag)
                            }
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
                    pump1Device: pump1
                    pump2Device: pump2
                    pump3Device: pump3
                    hardwareMode: window.isHardwareMode
                    recoveryAllowed: operatorSession.canApproveDrain
                }
            }
            EquipmentPanel {
                anchors.fill: parent
                anchors.margins: 24
                visible: window.activePage === "EQUIPMENT"
                rpmControlEnabled: operatorSession.selected && window.controllerReady
                onRpmRequested: (device, tag) => window.openPumpRpm(device, tag)
                resetFaultAllowed: operatorSession.canResetFault && window.controllerReady
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
                hardwareMode: window.isHardwareMode
                controllerClient: window.processControllerClient
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
                drainRequestService: drainService
                hardwareMode: window.isHardwareMode
                controllerClient: window.processControllerClient
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

                        HmiButton {
                            id: selectRunButton
                            width: 160
                            height: parent.height
                            text: "SELECT RUN"

                            onClicked: runSelectionDialog.open()

                            background: Rectangle {
                                radius: 4
                                color: selectRunButton.pressed ? "#d3d6d8" : "#f2f3f3"
                                border.color: "#858e92"
                            }

                            contentItem: Text {
                                text: selectRunButton.text
                                color: "#252a2d"
                                font.pixelSize: 13
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        HmiButton {
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

                        HmiButton {
                            id: acknowledgeButton
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

                            background: Rectangle {
                                radius: 4
                                color: acknowledgeButton.pressed ? "#d3d6d8" : "#f2f3f3"
                                border.color: acknowledgeButton.enabled ? "#9d2424" : "#858e92"
                            }

                            contentItem: Text {
                                text: acknowledgeButton.text
                                color: acknowledgeButton.enabled ? "#9d2424" : "#858e92"
                                font.pixelSize: 13
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
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
                                    MouseArea {
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: window.openEquipment(modelData)
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
            onFaultRequested: function(tag) {
                window.openEquipment(tag)
            }
        }

        NavigationBar {
            width: parent.width
            activePage: window.activePage
            hasUnacknowledgedFault: window.hasUnacknowledgedFault

            onPageSelected: function(page) {
                window.highlightedEquipmentTag = ""
                window.activePage = page
            }
        }
    }

    UserManagementDialog {
        id: userManagementDialog
        usersModel: userListModel
        managementService: userManagementService
        currentUserId: operatorSession.operatorId
    }

    OperatorSelectionDialog {
        id: operatorDialog
        usersModel: userListModel

        onUserChosen: function(userId) {
            const activeBatch = batchController.state !== BatchController.Idle
                                && batchController.state !== BatchController.Complete

            if (!operatorSession.selectOperator(userId))
                return

            if (activeBatch)
                eventManager.addInfo("ACCESS", "Active batch control assumed by " + operatorSession.operatorName)

            operatorDialog.close()
        }
    }

    RunSelectionDialog {
        id: runSelectionDialog
        runsModel: runListModel

        onRunSelected: function(selectedRunId) {
            if (eventHistoryModel.selectRun(selectedRunId))
                runSelectionDialog.close()
        }
    }

    PumpRpmDialog {
        id: rpmDialog

        onRpmRequested: function(device, tag, rpm) {
            if (window.isHardwareMode) {
                if (!window.processControllerClient.setPumpTargetRpm(device.id, rpm))
                    eventManager.addWarning(tag, "Target RPM request rejected: " + window.processControllerClient.lastError)

                return
            }

            device.setTargetRpm(rpm)
        }
    }
}