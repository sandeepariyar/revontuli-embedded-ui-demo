import QtQuick
import QtQuick.Controls

Rectangle {
    id: screen
    width: 480
    height: 800
    color: "#0b1112"
    property int frame: 0

    Text { x: 18; y: 12; text: "19:04"; color: "#a6b9b2"; font.pixelSize: 12 }
    Text { x: 18; y: 42; text: "Comfort"; color: "#edf1e7"; font.pixelSize: 27; font.bold: true }

    Chip { x: 18; y: 88; width: 218; label: "Climate"; opacity: 0.55 }
    Chip { x: 244; y: 88; width: 218; label: "Seat"; selected: true }

    Rectangle {
        x: 0; y: 143; width: 480; height: 280; color: "#020404"
        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 0
            width: 270
            height: 270
            fillMode: Image.PreserveAspectFit
            transform: Scale { origin.x: 135; xScale: SeatModel.selected === 1 ? -1 : 1 }
            source: {
                var i = Math.round(SeatModel.recline * 24 / 100)
                return "qrc:/assets/recline/seat_" + (i < 10 ? "0" : "") + i + ".jpg"
            }
        }
        Image {
            x: SeatModel.selected === 0 ? 233 : 157
            y: 91
            width: 90
            height: 128
            transform: Scale { origin.x: 45; xScale: SeatModel.selected === 1 ? -1 : 1 }
            visible: SeatModel.preview !== 0
            source: {
                if (SeatModel.preview === 0) return ""
                var kind = SeatModel.preview === 1 ? "heat" : SeatModel.preview === 2 ? "cool" : "massage"
                var level = SeatModel.preview === 1 ? SeatModel.heat : SeatModel.preview === 2 ? SeatModel.cool : 1
                return "qrc:/assets/fx/" + kind + "-" + level + "-" + screen.frame + ".png"
            }
        }
        Chip { x: 304; y: 10; width: 80; label: "Rear left"; selected: SeatModel.selected === 0; onTapped: SeatModel.select(0) }
        Chip { x: 389; y: 10; width: 80; label: "Rear right"; selected: SeatModel.selected === 1; onTapped: SeatModel.select(1) }
        Text {
            x: 18; y: 254; color: "#bacac3"; font.pixelSize: 12
            text: SeatModel.selected === 0 ? "Comfort · Rear left" : "Comfort · Rear right"
        }
    }
    Timer { interval: 150; repeat: true; running: SeatModel.preview !== 0; onTriggered: screen.frame = (screen.frame + 1) % 4 }

    Column {
        x: 18; y: 433; spacing: 8
        ThermalRow { label: "Seat heat"; value: SeatModel.heat; accent: "#ffb249"; onChanged: function(level) { SeatModel.setHeat(level) } }
        ThermalRow { label: "Seat cooling"; value: SeatModel.cool; accent: "#80d4ff"; onChanged: function(level) { SeatModel.setCool(level) } }
        Rectangle {
            width: 444; height: 51; radius: 14; color: "#151d1e"
            Text { x: 15; anchors.verticalCenter: parent.verticalCenter; text: "Massage"; color: "#edf1e7"; font.pixelSize: 15 }
            Chip { x: 356; y: 4; label: SeatModel.massage ? "On" : "Off"; selected: SeatModel.massage; onTapped: SeatModel.toggleMassage() }
        }
        Rectangle {
            width: 444; height: 60; radius: 14; color: "#151d1e"
            Text { x: 15; y: 6; text: "Recline"; color: "#edf1e7"; font.pixelSize: 14 }
            Text { x: 396; y: 6; text: SeatModel.recline + "%"; color: "#edf1e7"; font.pixelSize: 14 }
            Slider { x: 12; y: 24; width: 420; height: 31; from: 0; to: 100; value: SeatModel.recline; onMoved: SeatModel.setRecline(Math.round(value)) }
        }
        Row {
            spacing: 8
            Chip { width: 142; label: "Save"; onTapped: SeatModel.save() }
            Chip { width: 142; label: "Recall"; opacity: SeatModel.saved ? 1 : 0.55; onTapped: SeatModel.recall() }
            Chip { width: 144; label: "Reset"; onTapped: SeatModel.reset() }
        }
    }
    Rectangle {
        y: 749; width: 480; height: 51; radius: 16; color: "#101718"
        Row {
            anchors.centerIn: parent
            spacing: 5
            Chip { width: 87; label: "Home"; opacity: 0.55 }
            Chip { width: 87; label: "Comfort"; selected: true }
            Chip { width: 87; label: "Atmosphere"; opacity: 0.55 }
            Chip { width: 87; label: "Media"; opacity: 0.55 }
            Chip { width: 87; label: "Cabin"; opacity: 0.55 }
        }
    }
}
