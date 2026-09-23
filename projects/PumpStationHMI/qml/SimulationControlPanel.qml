// id="f3d5xq"
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: panel

    radius: 8

    color: "#292e36"
    border.color: "#59616d"

    RowLayout {
        anchors.fill: parent

        anchors.leftMargin: 16
        anchors.rightMargin: 16

        spacing: 10

        Label {
            text: "SIMULATION"

            color: "white"
            font.pixelSize: 15
            font.bold: true
        }

        Rectangle {
            width: 1
            Layout.fillHeight: true

            color: "#59616d"
        }

        Label {
            text: "Temperature:"

            color: "#b8c0ca"
            font.pixelSize: 14
        }

        Button {
            text: "25 °C"

            Layout.preferredWidth: 72
            Layout.preferredHeight: 36

            onClicked: {
                pump.setTemperatureFromSensor(25)
            }
        }

        Button {
            text: "60 °C"

            Layout.preferredWidth: 72
            Layout.preferredHeight: 36

            onClicked: {
                pump.setTemperatureFromSensor(60)
            }
        }

        Button {
            text: "90 °C"

            Layout.preferredWidth: 72
            Layout.preferredHeight: 36

            onClicked: {
                pump.setTemperatureFromSensor(90)
            }
        }

        Button {
            text: "95 °C"

            Layout.preferredWidth: 72
            Layout.preferredHeight: 36

            onClicked: {
                pump.setTemperatureFromSensor(95)
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }
}
