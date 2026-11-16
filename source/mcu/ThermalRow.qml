import QtQuick
Rectangle {
    id: row
    property string label: ""
    property int value: 0
    property color accent: "#3ddc97"
    signal changed(int level)
    width: 444
    height: 51
    radius: 14
    color: "#151d1e"
    Text { x: 15; anchors.verticalCenter: parent.verticalCenter; text: row.label; color: "#edf1e7"; font.pixelSize: 15 }
    Row {
        x: 226; y: 4; spacing: 6
        Chip { width: 48; label: "Off"; accent: row.accent; selected: row.value === 0; onTapped: row.changed(0) }
        Chip { width: 48; label: "1"; accent: row.accent; selected: row.value === 1; onTapped: row.changed(1) }
        Chip { width: 48; label: "2"; accent: row.accent; selected: row.value === 2; onTapped: row.changed(2) }
        Chip { width: 48; label: "3"; accent: row.accent; selected: row.value === 3; onTapped: row.changed(3) }
    }
}
