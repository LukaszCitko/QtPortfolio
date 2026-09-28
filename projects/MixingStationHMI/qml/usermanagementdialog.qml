import QtQuick
import QtQuick.Controls.Basic

Dialog {
    id: root

    required property var usersModel
    required property var managementService
    required property string currentUserId
    parent: Overlay.overlay
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: 620
    height: 580
    modal: true
    focus: true
    title: "DEMO USERS"
    standardButtons: Dialog.Cancel

    contentItem: Column {
        spacing: 12

        Text {
            id: listTitle
            width: parent.width
            text: "ACTIVE USERS"
            color: "#252a2d"
            font.pixelSize: 18
            font.bold: true
        }
        Row {
            id: addUserRow
            width: parent.width
            height: 52
            spacing: 8

            TextField {
                id: newUserName
                width: parent.width - newUserRole.width
                       - addUserButton.width - 2 * parent.spacing
                height: parent.height
                placeholderText: "User name"
            }

            ComboBox {
                id: newUserRole
                width: 150
                height: parent.height
                model: ["OPERATOR", "TECHNICIAN", "ADMIN"]
            }

            Button {
                id: addUserButton
                width: 140
                height: parent.height
                text: "ADD USER"
                enabled: newUserName.text.trim().length > 0

                onClicked: {
                    if (root.managementService.addUser(
                            newUserName.text, newUserRole.currentText)) {
                        newUserName.clear()
                    }
                }
            }
        }

        Text {
            id: errorText
            width: parent.width
            height: 36
            text: root.managementService.lastError
            color: "#b63838"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }

        ListView {
            id: usersList
            width: parent.width
            height: Math.max(0, parent.height
                            - listTitle.implicitHeight
                            - addUserRow.height
                            - errorText.height
                            - 3 * parent.spacing)
            clip: true
            spacing: 8
            model: root.usersModel

            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                id: userCard

                required property string userId
                required property string displayName
                required property string role

                width: usersList.width
                height: 62
                color: "#f7f8f8"
                border.color: "#a5aaad"

                Row {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 8

                    Text {
                        width: parent.width - roleSelector.width
                               - saveRoleButton.width
                               - removeUserButton.width
                               - 3 * parent.spacing
                        height: parent.height
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        text: userCard.displayName
                        color: "#252a2d"
                        font.pixelSize: 16
                    }

                    ComboBox {
                        id: roleSelector
                        width: 150
                        height: parent.height
                        model: ["OPERATOR", "TECHNICIAN", "ADMIN"]
                        currentIndex: userCard.role === "TECHNICIAN" ? 1
                                      : userCard.role === "ADMIN" ? 2 : 0
                        enabled: userCard.userId !== root.currentUserId
                    }

                    Button {
                        id: saveRoleButton
                        width: 90
                        height: parent.height
                        text: "SAVE"
                        enabled: userCard.userId !== root.currentUserId
                                 && roleSelector.currentText !== userCard.role

                        onClicked: root.managementService.changeRole(
                                       userCard.userId, roleSelector.currentText)
                    }
                    Button {
                        id: removeUserButton
                        width: 90
                        height: parent.height
                        text: "REMOVE"
                        enabled: userCard.userId !== root.currentUserId

                        onClicked: {
                            removeDialog.targetUserId = userCard.userId
                            removeDialog.targetName = userCard.displayName
                            removeDialog.open()
                        }
                    }
                }
            }
        }
    }
    Dialog {
        id: removeDialog

        property string targetUserId: ""
        property string targetName: ""

        parent: Overlay.overlay
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 440
        height: 210
        modal: true
        focus: true
        title: "REMOVE DEMO USER"
        standardButtons: Dialog.Ok | Dialog.Cancel

        Text {
            width: parent.width
            text: "Remove " + removeDialog.targetName
                  + " from active users? Existing run history will remain."
            color: "#252a2d"
            font.pixelSize: 16
            wrapMode: Text.WordWrap
        }

        onAccepted: root.managementService.removeUser(targetUserId)
    }
}