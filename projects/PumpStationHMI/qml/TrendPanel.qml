import QtQuick

Rectangle {
    id: root

    color: "#eceeef"
    border.color: "#a5aaad"

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Text {
            id: title
            height: 24
            text: "PROCESS TRENDS · LATEST RUN"
            color: "#252a2d"
            font.pixelSize: 20
            font.bold: true
        }

        TrendChart {
            width: parent.width
            height: (parent.height - title.height - 24) / 2
            recorder: trendRecorder
            chartTitle: "TK1 · MIXTURE VOLUME"
            series: "volume"
            unit: "L"
            minimum: 0
            maximum: 120
            lineColor: "#476a78"
        }

        TrendChart {
            width: parent.width
            height: (parent.height - title.height - 24) / 2
            recorder: trendRecorder
            chartTitle: "TK1 · TEMPERATURE"
            series: "temperature"
            unit: "°C"
            minimum: 0
            maximum: 100
            lineColor: "#98734b"
        }
    }
}