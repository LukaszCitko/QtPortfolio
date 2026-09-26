import QtQuick

Rectangle {
    id: root

    readonly property var faults: [
        pump.stateText === "FAULT" ? "P1" : "",
        valve1.stateText === "FAULT" ? "V1" : "",
        pump2.stateText === "FAULT" ? "P2" : "",
        valve2.stateText === "FAULT" ? "V2" : "",
        pump3.stateText === "FAULT" ? "P3" : "",
        valve3.stateText === "FAULT" ? "V3" : "",
        mixer.stateText === "FAULT" ? "M1" : "",
        mixingTank.stateText === "FAULT" ? "TK1" : "",
        valve4.stateText === "FAULT" ? "V4" : ""
    ].filter(function(device) { return device !== "" })

    readonly property bool hasFault: faults.length > 0

    color: hasFault ? "#f4ddda" : "#e8e9ea"

    Rectangle {
        width: 6
        height: parent.height
        anchors.left: parent.left
        color: root.hasFault ? "#b63838" : "#879195"
    }

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.right: alarmCount.left
        anchors.rightMargin: 24
        anchors.verticalCenter: parent.verticalCenter

        text: root.hasFault
              ? "ACTIVE ALARM · " + root.faults.join(", ") + " FAULT"
              : "NO ACTIVE ALARMS"

        color: root.hasFault ? "#8f2222" : "#252a2d"
        font.pixelSize: 20
        font.bold: true
        elide: Text.ElideRight
    }

    Text {
        id: alarmCount
        anchors.right: parent.right
        anchors.rightMargin: 24
        anchors.verticalCenter: parent.verticalCenter

        text: root.faults.length + " ACTIVE"
        color: root.hasFault ? "#8f2222" : "#252a2d"
        font.pixelSize: 16
    }
}