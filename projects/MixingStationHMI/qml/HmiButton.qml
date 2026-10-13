import QtQuick
import QtQuick.Controls.Basic

Button {
    id: root

    implicitWidth: Math.max(120, buttonText.implicitWidth + 32)
    implicitHeight: 44

    background: Rectangle {
        radius: 4
        color: !root.enabled ? "#c5cace" : root.down ? "#d3d6d8" : "#f7f8f8"
        border.color: root.enabled ? "#596368" : "#a5aaad"
        border.width: 1
    }

    contentItem: Text {
        id: buttonText
        text: root.text
        color: root.enabled ? "#252a2d" : "#879195"
        font.family: root.font.family
        font.pixelSize: root.font.pixelSize > 0 ? root.font.pixelSize : 14
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}