import QtQuick
import QtQuick.Controls
import QtQuick.Window

ApplicationWindow {
    id: root
	width: 500; height: 400
	title: "Options"

    Text {
        anchors.centerIn: parent
        text: qsTr("Hello World.")
    }
}
