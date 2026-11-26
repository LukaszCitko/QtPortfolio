import QtQuick

Rectangle {
    id: root

    required property string activePage
    required property bool hasUnacknowledgedFault
    signal pageSelected(string page)

    height: 88
    color: "#d0d3d5"

    Row {
        id: navigationRow
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Repeater {
            model: ["PROCESS", "TRENDS", "HISTORY", "EQUIPMENT", "ALARMS", "SIMULATION"]

            Rectangle {
                id: navigationButton

                property bool blinkOn: true

                width: (navigationRow.width - 5 * navigationRow.spacing) / 6
                height: navigationRow.height
                color: root.activePage === modelData ? "#f6f7f7" : "#d0d3d5"
                border.width: modelData === "ALARMS" && root.hasUnacknowledgedFault ? 3 : 1
                border.color: modelData === "ALARMS" && root.hasUnacknowledgedFault
                              ? (blinkOn ? "#b63838" : "#a5aaad")
                              : "#a5aaad"

                Timer {
                    interval: 650
                    repeat: true
                    running: modelData === "ALARMS" && root.hasUnacknowledgedFault
                    onTriggered: navigationButton.blinkOn = !navigationButton.blinkOn
                }

                Text {
                    anchors.centerIn: parent
                    text: modelData
                    color: "#252a2d"
                    font.pixelSize: 16
                    font.bold: root.activePage === modelData
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.pageSelected(modelData)
                }
            }
        }
    }
}