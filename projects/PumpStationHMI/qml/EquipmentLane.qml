import QtQuick

Rectangle {
    id: root

    required property var pumpDevice
    required property var valveDevice

    property string title: ""
    property string pumpTag: ""
    property string valveTag: ""
    property string flowArrow: "→"

    readonly property bool fault:
        pumpDevice.stateText === "FAULT"
        || valveDevice.stateText === "FAULT"

    readonly property bool highlighted:
        pumpDevice.stateText === "RUNNING" || fault

    height: highlighted ? 132 : 104
    color: highlighted ? "#f6f7f7" : "#e9ebec"
    border.color: fault ? "#b63838"
                        : highlighted ? "#596368" : "#a5aaad"
    border.width: highlighted ? 2 : 1

    Text {
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.left: parent.left
        anchors.leftMargin: 14
        height: 20
        text: root.title
        color: "#252a2d"
        font.pixelSize: 15
        font.bold: true
    }

    Item {
        id: sketch
        width: 260
        height: 70
        anchors.horizontalCenter: parent.horizontalCenter
        y: root.highlighted ? 36 : 22
        scale: root.highlighted ? 1 : 0.64


        // Connect the edge of the lane to the pump image.
           Rectangle {
               x: root.flowArrow === "←" ? 24 : 0
               width: pumpImage.x - x
               height: 2
               anchors.verticalCenter: parent.verticalCenter
               color: "#596368"
           }

           // Connect the pump image to the valve image.
           Rectangle {
               x: pumpImage.x + pumpImage.width
               width: valveImage.x - x
               height: 2
               anchors.verticalCenter: parent.verticalCenter
               color: "#596368"
           }

           // Connect the valve image to the edge of the lane.
           Rectangle {
               x: valveImage.x + valveImage.width
               width: (root.flowArrow === "←" ? sketch.width
                                              : sketch.width - 24) - x
               height: 2
               anchors.verticalCenter: parent.verticalCenter
               color: "#596368"
           }
           // Pump tile: white while running, gray while stopped.
           Rectangle {
               x: pumpImage.x
               y: pumpImage.y
               width: pumpImage.width
               height: pumpImage.height
               radius: 4
               color: root.pumpDevice.stateText === "RUNNING"
                      ? "#f7f8f8" : "#c5cace"
               border.color: "#929b9f"
           }

           // Valve tile: white while open, gray while closed.
           Rectangle {
               x: valveImage.x
               y: valveImage.y
               width: valveImage.width
               height: valveImage.height
               radius: 4
               color: root.valveDevice.stateText === "OPEN"
                      ? "#f7f8f8" : "#c5cace"
               border.color: "#929b9f"
           }


        // Pump and valve icon uses the muted asset while inactive.
           Image {
               id: pumpImage
               x: 48
               width: 60
               height: 60
               anchors.verticalCenter: parent.verticalCenter
               source: root.pumpDevice.stateText === "RUNNING"
                       ? "assets/pump.png" : "assets/pump_gray.png"
               fillMode: Image.PreserveAspectFit
               mirror: root.flowArrow === "←"
           }


           Image {
               id: valveImage
               x: 160
               width: 48
               height: 48
               anchors.verticalCenter: parent.verticalCenter
               source: root.valveDevice.stateText === "OPEN"
                       ? "assets/valve.png" : "assets/valve_gray.png"
               fillMode: Image.PreserveAspectFit
           }

        Text {
            x: root.flowArrow === "←" ? 0 : sketch.width - 24
            width: 24
            anchors.verticalCenter: parent.verticalCenter
            text: root.flowArrow
            horizontalAlignment: Text.AlignHCenter
            color: "#596368"
            font.pixelSize: 24
        }
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 20
        anchors.right: parent.right
        anchors.rightMargin: 14
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 4
        height: 18

        text: root.pumpTag + " · " + root.pumpDevice.stateText
              + "              " + root.valveTag + " · " + root.valveDevice.stateText

        color: root.fault ? "#b63838" : "#252a2d"
        font.pixelSize: 12
        elide: Text.ElideRight
    }
}