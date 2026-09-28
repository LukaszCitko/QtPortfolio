import QtQuick

Rectangle {
    id: root

    required property var recorder
    required property string chartTitle
    required property string series
    required property string unit
    required property real minimum
    required property real maximum
    required property string lineColor

    property int revision: 0
    onRecorderChanged: {
        revision += 1
        if (plot)
            plot.requestPaint()
    }

    function valueAt(index) {
        return series === "volume"
                ? recorder.volumeAt(index)
                : recorder.temperatureAt(index)
    }

    color: "#f7f8f8"
    border.color: "#a5aaad"

    Text {
        anchors.left: parent.left
        anchors.leftMargin: 16
        anchors.top: parent.top
        anchors.topMargin: 10
        text: root.chartTitle
        color: "#252a2d"
        font.pixelSize: 16
        font.bold: true
    }

    Text {
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.top: parent.top
        anchors.topMargin: 10
        text: {
            root.revision
            const last = root.recorder.sampleCount - 1
            return last >= 0
                    ? root.valueAt(last).toFixed(1) + " " + root.unit
                    : "NO DATA"
        }
        color: "#252a2d"
        font.pixelSize: 16
        font.bold: true
    }

    Canvas {
        id: plot
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 38
        anchors.bottomMargin: 8

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        Component.onCompleted: requestPaint()

        onPaint: {
            const ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            const left = 52
            const right = 12
            const top = 8
            const bottom = 24
            const plotWidth = width - left - right
            const plotHeight = height - top - bottom

            // Draw fixed-scale guides and value labels.
            ctx.strokeStyle = "#c4c9cb"
            ctx.fillStyle = "#596368"
            ctx.lineWidth = 1
            ctx.font = "12px sans-serif"
            ctx.textAlign = "right"

            for (let step = 0; step <= 2; ++step) {
                const fraction = step / 2
                const y = top + fraction * plotHeight
                const value = root.maximum
                              - fraction * (root.maximum - root.minimum)

                ctx.beginPath()
                ctx.moveTo(left, y)
                ctx.lineTo(left + plotWidth, y)
                ctx.stroke()
                ctx.fillText(value.toFixed(0), left - 7, y + 4)
            }

            const last = root.recorder.sampleCount - 1
            if (last < 0)
                return

            // Use only samples belonging to the latest run.
            const runId = root.recorder.runIdAt(last)
            let first = last

            while (first > 0 &&
                   root.recorder.runIdAt(first - 1) === runId) {
                --first
            }

            const startTime = root.recorder.timestampMsAt(first)
            const endTime = root.recorder.timestampMsAt(last)
            const timeSpan = Math.max(1000, endTime - startTime)
            const valueSpan = root.maximum - root.minimum

            // Draw the selected measurement as a connected line.
            ctx.strokeStyle = root.lineColor
            ctx.fillStyle = root.lineColor
            ctx.lineWidth = 2
            ctx.beginPath()

            let finalX = left
            let finalY = top + plotHeight

            for (let i = first; i <= last; ++i) {
                const time = root.recorder.timestampMsAt(i)
                const value = root.valueAt(i)
                const fraction = Math.max(
                    0, Math.min(1, (value - root.minimum) / valueSpan))

                const x = left
                          + (time - startTime) / timeSpan * plotWidth
                const y = top + (1 - fraction) * plotHeight

                if (i === first)
                    ctx.moveTo(x, y)
                else
                    ctx.lineTo(x, y)

                finalX = x
                finalY = y
            }

            ctx.stroke()

            ctx.beginPath()
            ctx.arc(finalX, finalY, 3, 0, Math.PI * 2)
            ctx.fill()

            ctx.fillStyle = "#596368"
            ctx.font = "12px sans-serif"
            ctx.textAlign = "left"
            ctx.fillText(new Date(startTime).toLocaleTimeString(),
                         left, height - 3)
            ctx.textAlign = "right"
            ctx.fillText(new Date(endTime).toLocaleTimeString(),
                         left + plotWidth, height - 3)
        }
    }

    Connections {
        target: root.recorder

        function onSamplesChanged() {
            root.revision += 1
            plot.requestPaint()
        }
    }
}