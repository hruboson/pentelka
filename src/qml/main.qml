import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs

import Pentelka

ApplicationWindow {
    id: root
    width: 800
    height: 600
    visible: true
    title: "Pentelka"

    CanvasItem {
        id: canvas
        anchors.fill: parent
        penColor: "black"
        penWidth: 3

        MouseArea {
            anchors.fill: parent
            onPressed: canvas.startDraw(mouse.x, mouse.y)
            onPositionChanged: canvas.drawTo(mouse.x, mouse.y)
        }
    }

    // File dialogs
    /*FileDialog {
        id: openDialog
        title: "Open Image"
        currentFolder: shortcuts.home
        nameFilters: ["*.png", "*.jpg", "*.bmp"]
        onAccepted: {
            canvas.openImage(selectedFile)
        }
    }

    FileDialog {
        id: saveDialog
        title: "Save Image"
        currentFolder: shortcuts.home
        nameFilters: ["*.png", "*.jpg", "*.bmp"]
        onAccepted: {
            canvas.saveImage(selectedFile)
        }
    }*/

    // Color picker
    ColorDialog {
        id: colorDialog
        title: "Select Pen Color"
        onAccepted: canvas.penColor = color
    }

    Row {
        spacing: 10
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter

        Button { text: "Clear"; onClicked: canvas.clear() }
        //Button { text: "Open"; onClicked: openDialog.open() }
        //Button { text: "Save"; onClicked: saveDialog.open() }
        Button { text: "Pen Color"; onClicked: colorDialog.open() }
    }
}
