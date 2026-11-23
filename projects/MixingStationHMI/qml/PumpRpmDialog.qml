import QtQuick
import QtQuick.Controls.Basic

HmiDialog {
    id: root

    property var pumpDevice: null
    property string pumpTag: ""
    signal rpmRequested(var device, string tag, int rpm)

    parent: Overlay.overlay
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: 420
    height: 250
    modal: true
    focus: true
    title: pumpTag + " · TARGET RPM"
    standardButtons: Dialog.Ok | Dialog.Cancel

    function openFor(device, tag) {
        pumpDevice = device
        pumpTag = tag
        rpmSlider.value = device.targetRpm
        open()
    }

    Column {
        width: root.availableWidth
        spacing: 16

        Text {
            text: "TARGET: " + Math.round(rpmSlider.value) + " RPM"
            color: "#252a2d"
            font.pixelSize: 20
            font.bold: true
        }

        Slider {
            id: rpmSlider
            width: parent.width
            from: 100
            to: root.pumpDevice ? root.pumpDevice.maxRpm : 1000
            stepSize: 50
            snapMode: Slider.SnapAlways
        }

        Text {
            text: "CURRENT: "
                  + (root.pumpDevice ? Math.round(root.pumpDevice.actualRpm) : 0)
                  + " RPM"
            color: "#596368"
            font.pixelSize: 16
        }
    }

    onAccepted: {
        if (pumpDevice)
            rpmRequested(pumpDevice, pumpTag, Math.round(rpmSlider.value))
    }
}