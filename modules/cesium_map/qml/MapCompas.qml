import QtQuick 2.12
import Industrial.Controls 1.0 as Controls

MapButton {
    id: root

    property real heading: 0
    property real pitch: 0

    round: true
    implicitHeight: Controls.Theme.baseSize * 1.5
    tipText: qsTr("Reset orientation")

    Controls.ColoredIcon {
        id: icon
        anchors.fill: parent
        anchors.margins: 1
        source: "qrc:/icons/compas.svg"
        rotation: heading
        transform: Rotation {
            origin.x: icon.width / 2;
            origin.y: icon.height / 2;
            axis { x: 1; y: 0; z: 0 }
            angle: 90 - pitch
        }
    }
}