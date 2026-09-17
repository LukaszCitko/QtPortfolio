import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root

    visible: true
    width: 1024
    height: 600
    minimumWidth: 1024
    minimumHeight: 600

    title: "Pump Station HMI"

    property int margin: 24
    property int cardSize: 180

    Rectangle {

        anchors.fill: parent
        color: "#20242a"

// ------------------------------------------------------------
// Main area
// ------------------------------------------------------------

        Column {
            id: mainArea

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.right: simulationPanel.left

            anchors.margins: root.margin
            anchors.rightMargin: root.margin + 20

            spacing: 24

            Label {
                text: "PUMP STATION HMI"
                font.pixelSize: 30
                font.bold: true
                color: "white"
            }

            Row {
                spacing: 20

// ----------------------------------------------------
// Actual RPM
// ----------------------------------------------------

                Rectangle {
                    width: root.cardSize
                    height: root.cardSize
                    radius: 10
                    color: "#2b3038"
                    border.color: "#59616d"

                    Column {
                        anchors.centerIn: parent
                        spacing: 10

                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "ACTUAL RPM"
                            color: "#b8c0ca"
                            font.pixelSize: 12
                        }

                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: Number(pump.actualRpm).toFixed(0)
                            color: "white"
                            font.pixelSize: 42
                            font.bold: true
                        }

                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "RPM"
                            color: "#b8c0ca"
                            font.pixelSize: 8
                        }
                    }
                }

// ----------------------------------------------------
// Target RPM
// ----------------------------------------------------

                Rectangle {
                    width: root.cardSize
                    height: root.cardSize
                    radius: 10
                    color: "#2b3038"
                    border.color: "#59616d"

                    Column {
                        anchors.centerIn: parent
                        spacing: 10

                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "TARGET RPM"
                            color: "#b8c0ca"
                            font.pixelSize: 12
                        }

                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: Number(pump.targetRpm).toFixed(0)
                            color: "white"
                            font.pixelSize: 36
                            font.bold: true
                        }

                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: Math.round(rpmSlider.value) + " %"
                            color: "#b8c0ca"
                            font.pixelSize: 12
                        }
                    }
                }

// ----------------------------------------------------
// Temperature
// ----------------------------------------------------

                Rectangle {
                    width: root.cardSize
                    height: root.cardSize
                    radius: 10
                    color: "#2b3038"
                    border.color: "#59616d"

                    Column {
                        anchors.centerIn: parent
                        spacing: 10

                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "TEMP. SENSOR"
                            color: "#b8c0ca"
                            font.pixelSize: 12
                        }

                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: Number(
                                      pump.temperatureFromSensor
                                  ).toFixed(1) + " °C"
                            color: "white"
                            font.pixelSize: 36
                            font.bold: true
                        }

                        Label {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "TEMPERATURE"
                            color: "#b8c0ca"
                            font.pixelSize: 12
                        }
                    }
                }
            }

// --------------------------------------------------------
// Start / Stop
// --------------------------------------------------------

            Row {
                spacing: 20

                Button {
                    width: 150
                    height: 60

                    text: "START"
                    font.pixelSize: 12

                    onClicked: {
                        pump.start()

                    }
                }

                Button {
                    width: 150
                    height: 60

                    text: "STOP"
                    font.pixelSize: 12

                    onClicked: {
                        pump.stop()
                    }
                }
            }
        }

// ------------------------------------------------------------
// Vertical RPM slider
// ------------------------------------------------------------

        Column {
            id: rpmControl

            anchors.top: parent.top
            anchors.right: simulationPanel.left
            anchors.rightMargin: root.margin
            anchors.bottom: simulationPanel.top
            anchors.bottomMargin: 20

            width: 90

            spacing: 10

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                text: ""
                color: "white"
                font.pixelSize: 12
                font.bold: true
            }

            Slider {
                id: rpmSlider

                orientation: Qt.Vertical

                anchors.horizontalCenter: parent.horizontalCenter

                height: 300
                width: 60

                from: 0
                to: pump.maxRpm
                stepSize: 10
                snapMode: Slider.SnapAlways
                value: pump.targetRpm

                onMoved: {
                        pump.targetRpm = value
                    }

            }

            Label {
                anchors.horizontalCenter: parent.horizontalCenter

                text: Math.round(rpmSlider.value) + " RPM"

                color: "white"
                font.pixelSize: 20
                font.bold: true
            }
        }

// ------------------------------------------------------------
// Simulation panel
// ------------------------------------------------------------

        Rectangle {
            id: simulationPanel

            width: 270
            height: 240

            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: root.margin

            radius: 10

            color: "#292e36"
            border.color: "#59616d"

            Column {
                anchors.fill: parent
                anchors.margins: 16

                spacing: 12

                Label {
                    text: "SIMULATION"
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                }

                Label {
                    text: "Temperature sensor"
                    color: "#b8c0ca"
                    font.pixelSize: 12
                }

                Row {
                    spacing: 8

                    SpinBox {
                        id: simulatedTemperature

                        width: 120
                        from: 0
                        to: 150
                        value: 60
                        stepSize: 10

                        onValueChanged: {
                            pump.setTemperatureFromSensor(value)
                        }
                    }

                    Label {
                        anchors.verticalCenter: parent.verticalCenter
                        text: "°C"
                        color: "white"
                        font.pixelSize: 12
                    }
                }

                Row {
                    spacing: 8

                    Button {
                        text: "60 °C"
                        width: 70

                        onClicked: {
                            simulatedTemperature.value = 60
                        }
                    }

                    Button {
                        text: "90 °C"
                        width: 70

                        onClicked: {
                            simulatedTemperature.value = 90
                        }
                    }
                }

                Row {
                    spacing: 8

                    Button {

                        id: faultBtn
                        text: "FAULT"
                        width: 105
                        height: 45

                        onClicked: {
                            pump.setFault()


                        }

                    }
                    Button {
                        text: "RESET"
                        width: 105
                        height: 45

                        onClicked: {
                            pump.resetFault()
                        }
                    }
                }
            }
        }
    }
}