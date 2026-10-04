import QtQuick
import QtQuick.Controls.Basic

Rectangle {
    id: root
    required property var liveSource
    required property var historySource
    readonly property var dataSource:
        root.historySource.runId === ""
        ? root.liveSource
        : root.historySource

    color: "#eceeef"
    border.color: "#a5aaad"

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Row {
            id: header
            width: parent.width
            height: 48
            spacing: 12

            Text {
                width: parent.width - 192
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                color: "#252a2d"
                font.pixelSize: 20
                font.bold: true

                text: root.historySource.runId === ""
                      ? "PROCESS TRENDS · LATEST RUN"
                      : "PROCESS TRENDS · RUN "
                        + root.historySource.runId.substring(0, 8)
            }

            Button {
                width: 180
                height: parent.height
                text: "LIVE TREND"
                enabled: root.historySource.runId !== ""
                onClicked: root.historySource.loadRun("")
            }
        }

        TrendChart {
            width: parent.width
            height: (parent.height - header.height - 24) / 2
            recorder: root.dataSource
            chartTitle: "TK1 · MIXTURE VOLUME"
            series: "volume"
            unit: "L"
            minimum: 0
            maximum: 120
            lineColor: "#476a78"
        }

        TrendChart {
            width: parent.width
           height: (parent.height - header.height - 24) / 2
            recorder: root.dataSource
            chartTitle: "TK1 · TEMPERATURE"
            series: "temperature"
            unit: "°C"
            minimum: 0
            maximum: 100
            lineColor: "#98734b"
        }
    }
}