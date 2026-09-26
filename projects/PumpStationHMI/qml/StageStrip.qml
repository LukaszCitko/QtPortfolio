import QtQuick

Rectangle {
    id: root

    required property int currentStage

    Row {
        id: stageRow
        anchors.fill: parent
        anchors.leftMargin: 24
        anchors.rightMargin: 24
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        spacing: 8

        Repeater {
            model: [
                "   PRE-CHECK",
                "1  WATER",
                "2  CONCENTRATE",
                "3  TEMPERATURE",
                "4  MIXING",
                "5  TRANSFER"
            ]

            Rectangle {
                width: (stageRow.width - 5 * stageRow.spacing) / 6
                height: stageRow.height
                color: index === root.currentStage ? "#f4f5f5" : "#d0d3d5"
                border.color: index === root.currentStage ? "#4b565b" : "#a5aaad"
                border.width: index === root.currentStage ? 2 : 1

                Text {
                    anchors.centerIn: parent
                    text: modelData
                    color: "#252a2d"
                    font.pixelSize: 15
                    font.bold: index === root.currentStage
                }
            }
        }
    }
}