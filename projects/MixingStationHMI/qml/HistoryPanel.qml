import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root
    required property var historyModel
    signal runSelectionRequested()
    signal trendsRequested(string runId)

    color: "#eceeef"
    border.color: "#a5aaad"

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        Row {
            width: parent.width
            height: 48
            spacing: 12

            Text {
                width: parent.width
                       - (root.historyModel.runFilter === "" ? 192 : 384)
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                color: "#252a2d"
                font.pixelSize: 20
                font.bold: true

                text: root.historyModel.runFilter === ""
                      ? "EVENT HISTORY · ALL EVENTS"
                      : "EVENT HISTORY · RUN "
                        + root.historyModel.runFilter.substring(0, 8)
            }

            HmiButton {
                id: selectRunButton
                width: 160
                height: parent.height
                text: "SELECT RUN"

                onClicked: root.runSelectionRequested()

                background: Rectangle {
                    radius: 4
                    color: selectRunButton.pressed ? "#d3d6d8" : "#f2f3f3"
                    border.color: "#858e92"
                }

                contentItem: Text {
                    text: selectRunButton.text
                    color: "#252a2d"
                    font.pixelSize: 13
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            HmiButton {
                width: 180
                height: parent.height
                visible: root.historyModel.runFilter !== ""
                text: "SHOW TRENDS"

                onClicked: root.trendsRequested(root.historyModel.runFilter)
            }
        }

        Text {
            id: noEventsText
            visible: historyRepeater.count === 0
            text: root.historyModel.runFilter === ""
                  ? "No events recorded"
                  : "No events for this run"
            color: "#596368"
            font.pixelSize: 16
        }

        Flickable {
            width: parent.width
            height: parent.height - 64
                    - (noEventsText.visible ? noEventsText.implicitHeight + 16 : 0)
            clip: true
            contentHeight: eventColumn.height

            Column {
                id: eventColumn
                width: parent.width
                spacing: 4

                Repeater {
                    id: historyRepeater
                    model: eventHistoryModel

                    delegate: Rectangle {
                        id: eventRow

                        required property string time
                        required property string level
                        required property string source
                        required property string message

                        width: eventColumn.width
                        height: 44
                        color: "#f7f8f8"
                        border.color: "#c4c9cb"

                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            color: "#252a2d"
                            font.pixelSize: 15

                            text: eventRow.time
                                  + "   " + eventRow.level
                                  + "   " + eventRow.source
                                  + "   " + eventRow.message
                        }
                    }
                }
            }
        }
    }
}