import QtQuick
import QtQuick.Controls.Basic

HmiDialog {
    id: root

    required property var usersModel
    signal userChosen(string userId)

    parent: Overlay.overlay
    width: 420
    height: 480
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    modal: true
    focus: true
    title: "SELECT DEMO USER"
    standardButtons: Dialog.Cancel

    contentItem: Column {
        id: contentColumn
        width: root.availableWidth
        height: root.availableHeight
        spacing: 12

        ListView {
            id: userListView
            width: parent.width
            height: Math.max(0, parent.height - demoNote.implicitHeight - contentColumn.spacing)
            clip: true
            spacing: 8
            model: root.usersModel

            ScrollBar.vertical: ScrollBar {}

            delegate: HmiButton {
                id: operatorButton

                required property string userId
                required property string displayName
                required property string role

                width: userListView.width
                height: 64
                text: displayName + " · " + role

                background: Rectangle {
                    color: operatorButton.down ? "#c5ced2" : "#f7f8f8"
                    border.color: "#a5aaad"
                    radius: 3
                }

                contentItem: Text {
                    text: operatorButton.text
                    color: "#252a2d"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: root.userChosen(userId)
            }
        }

        Text {
            id: demoNote
            width: parent.width
            wrapMode: Text.WordWrap
            color: "#596368"
            font.pixelSize: 13
            text: "Demo selection only · card authentication will be added later"
        }
    }
}