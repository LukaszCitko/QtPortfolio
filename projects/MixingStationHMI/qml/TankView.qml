import QtQuick

Rectangle {
    id: root

    signal mixerRequested()
    required property var tank
    required property var mixerDevice
    readonly property bool mixerFault:
        root.mixerDevice.fault
    readonly property color mixerLineColor:
        root.mixerFault ? "#b63838" : "#596368"
    readonly property real fillFraction:
        Math.max(0, Math.min(1, tank.volume / tank.capacity))

    color: "#eceeef"
    border.color: "#a5aaad"

    Text {
        id: title
        anchors.top: parent.top
        anchors.topMargin: 24
        anchors.horizontalCenter: parent.horizontalCenter
        text: "TK1 · MIXING TANK"
        color: "#252a2d"
        font.pixelSize: 20
        font.bold: true
    }

    Item {
        id: diagram
        width: 330
        height: 270
        anchors.top: title.bottom
        anchors.topMargin: 22
        anchors.horizontalCenter: parent.horizontalCenter

        // The outline represents the physical 120 L tank.
        Rectangle {
            id: vessel
            x: 50
            y: 20
            width: 200
            height: 238
            radius: 18
            color: "#f7f8f8"
            border.color: "#596368"
            border.width: 3

            // Liquid height follows physical capacity.
            Rectangle {
                x: 5
                width: parent.width - 10
                height: (parent.height - 10) * root.fillFraction
                y: parent.height - 5 - height
                radius: 7
                color: "#b4bfc4"
            }

            // Nominal batch volume is 100 L.
            Rectangle {
                x: 5
                width: parent.width - 10
                height: 2
                y: 5 + (parent.height - 10)
                   * (1 - 100 / root.tank.capacity)
                color: "#596368"
            }
        }

        // Three process connections.
        Repeater {
            model: [76, 143, 226]

            Rectangle {
                x: 18
                y: modelData
                width: vessel.x - x
                height: 3
                color: "#596368"
            }
        }

        // Mixer motor.
        Rectangle {
            x: vessel.x + vessel.width / 2 - 19
            y: 2
            width: 38
            height: 30
            radius: 3
            color: root.mixerFault ? "#f4ddda"
                   : root.mixerDevice.running
                     ? "#f7f8f8" : "#c5cace"
            border.color: root.mixerFault ? "#b63838" : "#596368"
            border.width: root.mixerFault ? 3 : 2
        }

        // Mixer shaft.
        Rectangle {
            x: vessel.x + vessel.width / 2 - 2
            y: 32
            width: 4
            height: 180
            color: root.mixerLineColor
        }

        // Mixer blades.
        Rectangle {
            x: vessel.x + vessel.width / 2 - 48
            y: 210
            width: 50
            height: 4
            rotation: 15
            color: root.mixerLineColor
        }

        Rectangle {
            x: vessel.x + vessel.width / 2
            y: 210
            width: 50
            height: 4
            rotation: -15
            color: root.mixerLineColor
        }

        // Touch target for the mixer motor.
        MouseArea {
            x: vessel.x + vessel.width / 2 - width / 2
            y: 0
            width: 80
            height: 56
            cursorShape: Qt.PointingHandCursor
            onClicked: root.mixerRequested()
        }

        Text {
            x: vessel.x + vessel.width + 10
            y: vessel.y
            text: "120 L MAX"
            color: "#596368"
            font.pixelSize: 13
        }

        Text {
            x: vessel.x + vessel.width + 10
            y: vessel.y + 5 + (vessel.height - 10)
               * (1 - 100 / root.tank.capacity) - 9
            text: "100 L"
            color: "#252a2d"
            font.pixelSize: 14
            font.bold: true
        }

        Text {
            x: vessel.x + vessel.width + 10
            y: vessel.y + vessel.height - 20
            text: "0 L"
            color: "#596368"
            font.pixelSize: 13
        }
    }

    Column {
        anchors.top: diagram.bottom
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 3

        Text {
            text: (root.tank.temperature < 58.0 && root.tank.volume > 0)
                  ? "TK1 · HEATING MODE · " + root.tank.stateText
                  : "TK1 · " + root.tank.stateText
            color: "#252a2d"
            font.pixelSize: 16
            font.bold: true
        }

        Text {
            text: "M1 · " + root.mixerDevice.stateText
            color: root.mixerFault ? "#b63838" : "#252a2d"
            font.pixelSize: 16
        }
    }
}
