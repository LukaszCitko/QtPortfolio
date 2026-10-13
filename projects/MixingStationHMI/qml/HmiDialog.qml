import QtQuick
import QtQuick.Controls.Basic

Dialog {
    id: root

    background: Rectangle {
        color: "#eceeef"
        border.color: "#858e92"
        border.width: 1
        radius: 4
    }

    header: Rectangle {
        width: root.width
        implicitHeight: 56
        color: "#e8e9ea"
        border.color: "#858e92"

        Text {
            anchors.fill: parent
            anchors.leftMargin: 18
            anchors.rightMargin: 18
            text: root.title
            color: "#252a2d"
            font.pixelSize: 16
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    footer: DialogButtonBox {
        id: buttonBox
        visible: root.standardButtons !== Dialog.NoButton
        standardButtons: root.standardButtons
        spacing: 12
        padding: 10

        background: Rectangle {
            color: "#e8e9ea"
            border.color: "#a5aaad"
        }

        delegate: HmiButton {
            implicitWidth: 120
            implicitHeight: 42
        }

        onAccepted: root.accept()
        onRejected: root.reject()
    }
}