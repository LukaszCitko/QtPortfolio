import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window

    visible: true
    width: 1280
    height: 800
    title: "Pump Station HMI"

    color: "#1b1f24"

    // ============================================================
    // HEADER
    // ============================================================

    Rectangle {
        id: header

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: 48

        color: "#252a31"

        // User / Login
        Button {
            id: userButton

            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter

            width: 155
            height: 34

            text: "USER: OPERATOR"

            onClicked: loginDialog.open()
        }

        // Centered title
        Label {
            anchors.centerIn: parent

            text: "PUMP STATION #1"

            color: "white"
            font.pixelSize: 21
            font.bold: true
        }

        // System status
        Label {
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter

            text: "PROCESS: " + batchController.stateText
            color: "#b8c0ca"

            font.pixelSize: 15
            font.bold: true
        }
    }

    // ============================================================
    // MAIN CONTENT
    // ============================================================

    ColumnLayout {
        id: pageLayout

        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: eventBar.top

        anchors.margins: 16

        spacing: 12

        // ========================================================
        // MAIN PANELS
        // ========================================================

        RowLayout {
            id: mainPanels

            Layout.fillWidth: true
            Layout.fillHeight: true

            spacing: 16

            // ====================================================
            // LEFT - PUMP CONTROL
            // ====================================================

            Rectangle {
                id: pumpPanel

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 1

                radius: 10
                color: "#252a31"
                border.color: "#414852"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20

                    spacing: 12

                    // ------------------------------------------------
                    // PUMP SELECT
                    // ------------------------------------------------

                    Label {
                        text: "PUMP SELECT"

                        color: "#b8c0ca"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        spacing: 8

                        Button {
                            text: "1"

                            Layout.preferredWidth: 58
                            Layout.preferredHeight: 38

                            highlighted: true
                        }

                        Button {
                            text: "2"

                            Layout.preferredWidth: 58
                            Layout.preferredHeight: 38

                            enabled: false
                        }

                        Button {
                            text: "3"

                            Layout.preferredWidth: 58
                            Layout.preferredHeight: 38

                            enabled: false
                        }

                        Item {
                            Layout.fillWidth: true
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1

                        color: "#414852"
                    }

                    // ------------------------------------------------
                    // PUMP STATUS
                    // ------------------------------------------------

                    Label {
                        text: "PUMP 1"

                        color: "white"
                        font.pixelSize: 24
                        font.bold: true
                    }

                    Label {
                        text: pump.stateText

                        color: pump.stateText === "RUNNING"
                               ? "#7fd38b"
                               : pump.stateText === "FAULT"
                                 ? "#e06b6b"
                                 : "#b8c0ca"

                        font.pixelSize: 17
                        font.bold: true
                    }

                    // ------------------------------------------------
                    // PROCESS VALUES
                    // ------------------------------------------------

                    GridLayout {
                        Layout.fillWidth: true

                        columns: 2

                        columnSpacing: 20
                        rowSpacing: 9

                        Label {
                            text: "Actual RPM"

                            color: "#b8c0ca"
                            font.pixelSize: 15
                        }

                        Label {
                            text: Math.round(pump.actualRpm) + " RPM"

                            color: "white"
                            font.pixelSize: 18
                            font.bold: true

                            Layout.alignment: Qt.AlignRight
                        }

                        Label {
                            text: "Target RPM"

                            color: "#b8c0ca"
                            font.pixelSize: 15
                        }

                        Label {
                            text: Math.round(pump.targetRpm) + " RPM"

                            color: "white"
                            font.pixelSize: 18
                            font.bold: true

                            Layout.alignment: Qt.AlignRight
                        }

                        Label {
                            text: "Temperature"

                            color: "#b8c0ca"
                            font.pixelSize: 15
                        }

                        Label {
                            text: pump.temperatureFromSensor.toFixed(0) + " °C"

                            color: pump.temperatureFromSensor > 90
                                   ? "#e06b6b"
                                   : "white"

                            font.pixelSize: 18
                            font.bold: true

                            Layout.alignment: Qt.AlignRight
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1

                        color: "#414852"
                    }

                    // ------------------------------------------------
                    // RPM CONTROL
                    // ------------------------------------------------

                    Label {
                        text: "RPM CONTROL"

                        color: "#b8c0ca"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    Slider {
                        id: rpmSlider

                        Layout.fillWidth: true

                        from: 0
                        to: pump.maxRpm
                        stepSize: 10

                        value: pump.targetRpm

                        onMoved: {
                            pump.setTargetRpm(value)
                        }
                    }

                    Label {
                        text: Math.round(rpmSlider.value) + " RPM"

                        color: "white"
                        font.pixelSize: 15

                        Layout.alignment: Qt.AlignHCenter
                    }

                    Item {
                        Layout.fillHeight: true
                    }

                    // ------------------------------------------------
                    // MAIN CONTROLS
                    // ------------------------------------------------

                    RowLayout {


                        spacing: 10

                        Button {
                            text: pump.stateText === "RUNNING" ?
                                      "PUMP OFF" : "PUMP ON"

                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 48

                            onClicked: {
                                if (pump.stateText === "RUNNING") {
                                            controller.stopPump()
                                        } else {
                                            controller.startPump()
                                        }
                            }
                        }

                        Button {
                            text: valve1.stateText === "OPEN" ?
                                    "VALVE OFF" : "VALVE ON"

                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 48

                            onClicked: {
                                if (valve1.stateText === "OPEN") {
                                            valve1.close()
                                } else {
                                            valve1.open()
                                }
                            }
                        }

                        Button {
                            text: "SYSTEM CONTROLS"

                            Layout.preferredWidth: 250
                            Layout.preferredHeight: 48

                            onClicked: systemControlsDialog.open()
                        }
                    }
                }
            }

            // ====================================================
            // RIGHT - PROCESS OVERVIEW
            // ====================================================

            Rectangle {
                id: overviewPanel

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 1

                radius: 10
                color: "#252a31"
                border.color: "#414852"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20

                    spacing: 12

                    Label {
                        text: "PROCESS OVERVIEW"

                        color: "white"
                        font.pixelSize: 20
                        font.bold: true
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1

                        color: "#414852"
                    }

                    // ------------------------------------------------
                    // PUMPS
                    // ------------------------------------------------

                    Label {
                        text: "PUMPS"

                        color: "#b8c0ca"
                        font.pixelSize: 13
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: "PUMP 1"

                            color: "white"
                            font.pixelSize: 16
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Label {
                            text: pump.stateText

                            color: pump.stateText === "RUNNING"
                                   ? "#7fd38b"
                                   : pump.stateText === "FAULT"
                                     ? "#e06b6b"
                                     : "#b8c0ca"

                            font.pixelSize: 15
                            font.bold: true
                        }

                        Label {
                            text: Math.round(pump.actualRpm) + " RPM"

                            color: "#b8c0ca"
                            font.pixelSize: 14
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: "PUMP 2"

                            color: "#6f7782"
                            font.pixelSize: 16
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Label {
                            text: "NOT AVAILABLE"

                            color: "#6f7782"
                            font.pixelSize: 13
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: "PUMP 3"

                            color: "#6f7782"
                            font.pixelSize: 16
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Label {
                            text: "NOT AVAILABLE"

                            color: "#6f7782"
                            font.pixelSize: 13
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1

                        color: "#414852"
                    }

                    // ------------------------------------------------
                    // VALVES
                    // ------------------------------------------------

                    Label {
                        text: "VALVES"

                        color: "#b8c0ca"
                        font.pixelSize: 13
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: "VALVE 1"

                            color: "white"
                            font.pixelSize: 16
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Label {
                            text: valve1.stateText

                            color: valve1.isOpen
                                   ? "#7fd38b"
                                   : valve1.stateText === "FAULT"
                                     ? "#e06b6b"
                                     : "#b8c0ca"

                            font.pixelSize: 15
                            font.bold: true
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1

                        color: "#414852"
                    }

                    // ------------------------------------------------
                    // TANK
                    // ------------------------------------------------

                    Label {
                        text: "TANK"

                        color: "#b8c0ca"
                        font.pixelSize: 13
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Label {
                            text: "LEVEL"

                            color: "white"
                            font.pixelSize: 16
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Label {
                            text: "72 %"

                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }
                }
            }
        }

        // ========================================================
        // SIMULATION BAR
        // ========================================================

        SimulationControlPanel {
            Layout.fillWidth: true
            Layout.preferredHeight: 58
        }
    }

    // ============================================================
    // EVENT BAR
    // ============================================================

    Rectangle {
        id: eventBar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        height: 50

        color: "#20252b"
        border.color: "#414852"

        RowLayout {
            anchors.fill: parent

            anchors.leftMargin: 16
            anchors.rightMargin: 16

            spacing: 12

            Label {
                text: eventManager.currentLevel === 2
                      ? "ALARM"
                      : eventManager.currentLevel === 1
                        ? "WARNING"
                        : "INFO"

                color: eventManager.currentLevel === 2
                       ? "#e06b6b"
                       : eventManager.currentLevel === 1
                         ? "#e0b45c"
                         : "#7fa9d3"

                font.pixelSize: 13
                font.bold: true
            }

            Label {
                text: eventManager.currentSource
                      + " — "
                      + eventManager.currentMessage

                color: "white"
                font.pixelSize: 14

                Layout.fillWidth: true

                elide: Text.ElideRight
            }

            Button {
                text: "HISTORY"

                Layout.preferredWidth: 90
                Layout.preferredHeight: 34

                onClicked: {
                    // History dialog will be implemented later.
                }
            }
        }
    }

    // ============================================================
    // LOGIN DIALOG - MOCK
    // ============================================================

    Dialog {
        id: loginDialog

        title: "LOGIN"

        modal: true

        anchors.centerIn: Overlay.overlay

        width: 360

        standardButtons: Dialog.Cancel

        ColumnLayout {
            width: parent.width

            spacing: 12

            Label {
                text: "Username"

                color: "#b8c0ca"
            }

            TextField {
                id: usernameField

                Layout.fillWidth: true

                placeholderText: "Enter username"
            }

            Label {
                text: "Password"

                color: "#b8c0ca"
            }

            TextField {
                id: passwordField

                Layout.fillWidth: true

                placeholderText: "Enter password"

                echoMode: TextInput.Password
            }

            Button {
                text: "LOGIN"

                Layout.fillWidth: true

                onClicked: {
                    loginDialog.close()
                }
            }
        }
    }

    // ============================================================
    // SYSTEM CONTROLS DIALOG - MOCK
    // ============================================================

    Dialog {
        id: systemControlsDialog

        title: "SYSTEM CONTROLS"

        modal: true

        anchors.centerIn: Overlay.overlay

        width: 400

        standardButtons: Dialog.Close

        ColumnLayout {
            width: parent.width

            spacing: 14

            Label {
                text: "PUMP 1"

                color: "white"
                font.pixelSize: 20
                font.bold: true
            }

            Label {
                text: "Fault status: "
                      + pump.stateText

                color: "#b8c0ca"
                font.pixelSize: 16
            }

            Button {
                text: "RESET FAULT"

                Layout.fillWidth: true
                Layout.preferredHeight: 45

                onClicked: {
                    controller.resetPumpFault()
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1

                color: "#414852"
            }

            Label {
                text: "TECHNICIAN FUNCTIONS"

                color: "#b8c0ca"
                font.pixelSize: 13
                font.bold: true
            }

            Button {
                text: "SENSOR STATUS"

                Layout.fillWidth: true
                enabled: false
            }

            Button {
                text: "I/O STATUS"

                Layout.fillWidth: true
                enabled: false
            }

            Button {
                text: "DIAGNOSTICS"

                Layout.fillWidth: true
                enabled: false
            }
        }
    }
}
