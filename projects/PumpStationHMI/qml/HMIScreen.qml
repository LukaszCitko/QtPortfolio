import QtQuick
import QtQuick.Controls.Basic

ApplicationWindow {
    id: window
    property string activePage: "PROCESS"
    visible: true
    width: 1280
    height: 800
    title: "Mixing Station HMI"
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
                            pumpDevice: pump
                            valveDevice: valve1
                        }

                        EquipmentLane {
                            width: parent.width
                            title: "CONCENTRATE →   "
                            pumpTag: "P2"
                            valveTag: "V2"
                            flowArrow: "→"
                            pumpDevice: pump2
                            valveDevice: valve2
                        }

                        EquipmentLane {
                            width: parent.width
                            title: "← PRODUCT   "
                            pumpTag: "P3"
                            valveTag: "V3"
                            flowArrow: "←"
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
            SimulationControlPanel {
                anchors.fill: parent
                anchors.margins: 24
                visible: window.activePage === "SIMULATION"
                mixerDevice: mixer
                tankDevice: mixingTank
                batchDevice: batchController
            }
        }

        AlarmBar {
            id: alarmBar
            width: parent.width
            height: 80
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
                        width: (navigationRow.width - 5 * navigationRow.spacing) / 6
                        height: navigationRow.height
                        color: window.activePage === modelData ? "#f6f7f7" : "#d0d3d5"
                        border.color: "#a5aaad"

                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            color: modelData === "PROCESS" || modelData === "SIMULATION"
                                   ? "#252a2d" : "#879195"
                            font.pixelSize: 16
                            font.bold: window.activePage === modelData
                        }

                        MouseArea {
                            anchors.fill: parent
                            enabled: modelData === "PROCESS"
                                     || modelData === "SIMULATION"
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

}