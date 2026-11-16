import QtQuick
Rectangle {
    property string label: ""
    property bool selected: false
    property color accent: "#3ddc97"
    signal tapped()
    width: 72
    height: 44
    radius: 14
    color: selected ? accent : "#20292a"
    Text {
        anchors.centerIn: parent
        text: parent.label
        color: parent.selected ? "#092419" : "#edf1e7"
        font.pixelSize: 14
    }
    MouseArea { anchors.fill: parent; onClicked: parent.tapped() }
}
