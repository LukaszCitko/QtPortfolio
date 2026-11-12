import QtQuick
import QtQuick.Controls.Basic

HmiDialog {
    id: root
    required property var runsModel
    signal runSelected(string runId)

    parent: Overlay.overlay
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: 720
    height: 500
    modal: true
    focus: true
    title: "SELECT RUN"

    onOpened: root.runsModel.refresh()

    contentItem: Column {
        width: root.availableWidth
        height: root.availableHeight
        spacing: 8

        HmiButton {
            id: allEventsButton
            background: Rectangle {
                color: allEventsButton.down ? "#c5ced2" : "#f7f8f8"
                border.color: "#a5aaad"
            }

            contentItem: Text {
                text: allEventsButton.text
                color: "#252a2d"
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            width: parent.width
            height: 54
            text: "ALL EVENTS"

            onClicked: root.runSelected("")
        }

        ListView {
            id: runListView
            width: parent.width
            height: parent.height - 62
            clip: true
            model: root.runsModel

            boundsBehavior: Flickable.StopAtBounds

            delegate: ItemDelegate {
                id: runDelegate

                required property string runId
                required property string kind
                required property string operatorName
                required property string startedAt
                required property string outcome

                background: Rectangle {
                    color: runDelegate.down ? "#c5ced2" : "#f7f8f8"
                    border.color: "#a5aaad"
                }

                contentItem: Item {
                    Column {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        spacing: 2

                        Text {
                            width: parent.width
                            text: "RUN " + runId + " · " + kind +
                                  (kind === "BATCH" ? (operatorName === "" ? " · PARTIAL RECORD · START UNKNOWN" : " · STARTED BY: " + operatorName) : "")
                            color: "#252a2d"
                            font.pixelSize: 16
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        Text {
                            width: parent.width
                            text: (outcome === "" ? "IN PROGRESS" : outcome) + " · " +
                                  (kind === "BATCH" && operatorName === "" ? "FIRST SEEN: " : "STARTED: ") + startedAt
                            color: "#626b70"
                            font.pixelSize: 13
                            elide: Text.ElideRight
                        }
                    }
                }

                width: runListView.width
                height: 58
                font.pixelSize: 16

                onClicked: root.runSelected(runId)
            }
        }
    }
}