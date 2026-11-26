import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root

    required property bool controllerReady
    required property string statusText
    required property bool operatorChangeAllowed
    required property string historyStatusText
    required property string loginText
    required property bool canManageUsers

    signal loginRequested()
    signal usersRequested()


    height: 80
    color: "#e8e9ea"

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.verticalCenter: parent.verticalCenter
        text: "MIXING STATION #1"
        color: "#252a2d"
        font.pixelSize: 24
        font.bold: true
    }

    Text {
        anchors.centerIn: parent
        text: root.statusText

        color: root.controllerReady ? "#252a2d" : "#b63838"
        font.pixelSize: 20
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        visible: root.historyStatusText !== ""
        text: root.historyStatusText
        color: "#b63838"
        font.pixelSize: 14
        font.bold: true
    }

    HmiButton {
        id: loginButton
        anchors.right: parent.right
        anchors.rightMargin: 24
        anchors.verticalCenter: parent.verticalCenter
        width: 220
        height: 48

        text: root.loginText
        enabled: root.operatorChangeAllowed
        onClicked: root.loginRequested()

        background: Rectangle {
            color: "#f7f8f8"
            border.color: "#858e92"
            radius: 4
        }

        contentItem: Text {
            text: loginButton.text
            color: loginButton.enabled ? "#252a2d" : "#596368"
            font.pixelSize: 15
            font.bold: true
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    HmiButton {
        id: manageUsersButton
        anchors.right: loginButton.left
        anchors.rightMargin: 12
        anchors.verticalCenter: parent.verticalCenter
        width: 160
        height: 48

        visible: root.canManageUsers
        enabled: loginButton.enabled
        text: "USERS"

        onClicked: root.usersRequested()

        background: Rectangle {
            color: manageUsersButton.enabled ? "#f7f8f8" : "#d7dadd"
            border.color: "#858e92"
            radius: 4
        }

        contentItem: Text {
            text: manageUsersButton.text
            color: manageUsersButton.enabled ? "#252a2d" : "#596368"
            font.pixelSize: 14
            maximumLineCount: 2
            wrapMode: Text.NoWrap
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

}