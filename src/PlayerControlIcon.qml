import QtQuick
import QtQuick.Shapes

Item {
    id: root

    property string iconType: ""
    property color iconColor: "black"

    implicitWidth: 24
    implicitHeight: 24

    Item {
        id: drawing
        width: 24
        height: 24
        anchors.centerIn: parent

        Rectangle {
            visible: root.iconType === "slower" || root.iconType === "faster"
            x: 5
            y: 10
            width: 14
            height: 3
            color: root.iconColor
        }

        Rectangle {
            visible: root.iconType === "faster"
            x: 10
            y: 4
            width: 3
            height: 15
            color: root.iconColor
        }

        Rectangle {
            visible: root.iconType === "previous"
            x: 3
            y: 5
            width: 3
            height: 14
            color: root.iconColor
        }

        Shape {
            id: previousTriangle
            visible: root.iconType === "previous"
            x: 8
            y: 4
            width: 12
            height: 16

            ShapePath {
                fillColor: root.iconColor
                strokeWidth: 0
                startX: previousTriangle.width
                startY: 0
                PathLine { x: 0; y: previousTriangle.height / 2 }
                PathLine { x: previousTriangle.width; y: previousTriangle.height }
                PathLine { x: previousTriangle.width; y: 0 }
            }
        }

        Shape {
            id: rewindFirst
            visible: root.iconType === "rewind"
            x: 2
            y: 5
            width: 9
            height: 14

            ShapePath {
                fillColor: root.iconColor
                strokeWidth: 0
                startX: rewindFirst.width
                startY: 0
                PathLine { x: 0; y: rewindFirst.height / 2 }
                PathLine { x: rewindFirst.width; y: rewindFirst.height }
                PathLine { x: rewindFirst.width; y: 0 }
            }
        }

        Shape {
            id: rewindSecond
            visible: root.iconType === "rewind"
            x: 10
            y: 5
            width: 9
            height: 14

            ShapePath {
                fillColor: root.iconColor
                strokeWidth: 0
                startX: rewindSecond.width
                startY: 0
                PathLine { x: 0; y: rewindSecond.height / 2 }
                PathLine { x: rewindSecond.width; y: rewindSecond.height }
                PathLine { x: rewindSecond.width; y: 0 }
            }
        }

        Shape {
            id: nextTriangle
            visible: root.iconType === "next"
            x: 4
            y: 4
            width: 12
            height: 16

            ShapePath {
                fillColor: root.iconColor
                strokeWidth: 0
                startX: 0
                startY: 0
                PathLine { x: nextTriangle.width; y: nextTriangle.height / 2 }
                PathLine { x: 0; y: nextTriangle.height }
                PathLine { x: 0; y: 0 }
            }
        }

        Shape {
            id: forwardFirst
            visible: root.iconType === "forward"
            x: 5
            y: 5
            width: 9
            height: 14

            ShapePath {
                fillColor: root.iconColor
                strokeWidth: 0
                startX: 0
                startY: 0
                PathLine { x: forwardFirst.width; y: forwardFirst.height / 2 }
                PathLine { x: 0; y: forwardFirst.height }
                PathLine { x: 0; y: 0 }
            }
        }

        Shape {
            id: forwardSecond
            visible: root.iconType === "forward"
            x: 12
            y: 5
            width: 9
            height: 14

            ShapePath {
                fillColor: root.iconColor
                strokeWidth: 0
                startX: 0
                startY: 0
                PathLine { x: forwardSecond.width; y: forwardSecond.height / 2 }
                PathLine { x: 0; y: forwardSecond.height }
                PathLine { x: 0; y: 0 }
            }
        }

        Rectangle {
            visible: root.iconType === "next"
            x: 18
            y: 5
            width: 3
            height: 14
            color: root.iconColor
        }

        Shape {
            id: playTriangle
            visible: root.iconType === "play"
            x: 5
            y: 3
            width: 15
            height: 18

            ShapePath {
                fillColor: root.iconColor
                strokeWidth: 0
                startX: 0
                startY: 0
                PathLine { x: playTriangle.width; y: playTriangle.height / 2 }
                PathLine { x: 0; y: playTriangle.height }
                PathLine { x: 0; y: 0 }
            }
        }

        Row {
            visible: root.iconType === "pause"
            anchors.centerIn: parent
            spacing: 5

            Rectangle {
                width: 5
                height: 16
                color: root.iconColor
            }
            Rectangle {
                width: 5
                height: 16
                color: root.iconColor
            }
        }
    }
}
