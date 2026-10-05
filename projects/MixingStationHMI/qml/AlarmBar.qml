import QtQuick

Rectangle {
    id: root

    required property var eventSource
    required property var monitoredDevices

    readonly property var faults: root.monitoredDevices
        .filter(function(item) {
            return item.device.fault
        })
        .map(function(item) {
            return item.tag
        })

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
              : root.eventSource.currentMessage.length > 0
                ? root.eventSource.currentLevelText
                  + " · " + root.eventSource.currentSource
                  + " · " + root.eventSource.currentMessage
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