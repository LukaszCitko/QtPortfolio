import QtQuick

Rectangle {
    id: root

    required property var tank
    required property var mixerDevice

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
            color: root.mixerDevice.stateText === "RUNNING"
                   ? "#f7f8f8" : "#c5cace"
            border.color: "#596368"
            border.width: 2
        }

        // Mixer shaft.
        Rectangle {
            x: vessel.x + vessel.width / 2 - 2
            y: 32
            width: 4
            height: 180
            color: "#596368"
        }

        // Mixer blades.
        Rectangle {
            x: vessel.x + vessel.width / 2 - 48
            y: 210
            width: 50
            height: 4
            rotation: 15
            color: "#596368"
        }

        Rectangle {
            x: vessel.x + vessel.width / 2
            y: 210
            width: 50
            height: 4
            rotation: -15
            color: "#596368"
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
            text: "TK1 · " + root.tank.stateText
            color: "#252a2d"
            font.pixelSize: 16
            font.bold: true
        }

        Text {
            text: "M1 · " + root.mixerDevice.stateText
            color: "#252a2d"
            font.pixelSize: 16
        }
    }
}

/*
// OLD VERSION OF TANK

import QtQuick

Rectangle {
    id: root

    required property var tank

    color: "#eceeef"
    border.color: "#a5aaad"

    Text {
        id: title
        anchors.top: parent.top
        anchors.topMargin: 24
        anchors.horizontalCenter: parent.horizontalCenter
        text: "MIXING TANK"
        color: "#252a2d"
        font.pixelSize: 20
        font.bold: true
    }

    Rectangle {
        id: vessel
        width: 220
        height: 265
        anchors.top: title.bottom
        anchors.topMargin: 18
        anchors.horizontalCenter: parent.horizontalCenter
        radius: 14
        color: "#f7f8f8"
        border.color: "#596368"
        border.width: 3

        Rectangle {
            anchors.left: parent.left
            anchors.leftMargin: 4
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 4

            height: (vessel.height - 8)
                    * Math.max(0, Math.min(100, root.tank.levelPercent)) / 100
            color: "#aeb7bb"
        }

        Rectangle {
            width: 4
            height: 85
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            color: "#596368"
        }

        Rectangle {
            width: 70
            height: 5
            anchors.centerIn: parent
            color: "#596368"
        }

        Text {
            anchors.centerIn: parent
            text: root.tank.levelPercent.toFixed(0) + " %"
            color: "#252a2d"
            font.pixelSize: 26
            font.bold: true
        }
    }

    Text {
        anchors.top: vessel.bottom
        anchors.topMargin: 16
        anchors.horizontalCenter: parent.horizontalCenter
        text: root.tank.stateText
              + "  ·  "
              + root.tank.volume.toFixed(1)
              + " / "
              + "100"
              + " L"
        color: "#252a2d"
        font.pixelSize: 16
    }
}
 */