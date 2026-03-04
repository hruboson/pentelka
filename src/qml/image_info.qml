import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 200; height: 400

    Text {
        anchors.centerIn: parent
        text: qsTr("Hello World.")
    }
}
