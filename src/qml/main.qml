import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs

ApplicationWindow {
    id: root
    width: 800
    height: 600
    visible: true
    title: "Pentelka"

    property color penColor: "black"

    // background gray
    Rectangle {
        anchors.fill: parent
        color: "#CCCCCC"

        // canvas
        Rectangle {
            id: canvasContainer
            width: 800
            height: 600
            anchors.centerIn: parent
            color: "white" // canvas background

			property var lastPoint: null

            Image {
                id: paintImage
                anchors.fill: parent
                source: "image://painter/current"
                fillMode: Image.PreserveAspectFit  // prevent stretching
            }

            MouseArea {
                anchors.fill: parent
				onPressed: (mouse) => {
					canvasContainer.lastPoint = Qt.point(mouse.x, mouse.y)
					painter.setPixel(mouse.x, mouse.y, penColor)
				}

				onPositionChanged: (mouse) => {
					if (canvasContainer.lastPoint) {
						painter.drawWuLine(canvasContainer.lastPoint, Qt.point(mouse.x, mouse.y), penColor)
					}
					canvasContainer.lastPoint = Qt.point(mouse.x, mouse.y)
				}            
			}
        }
    }

    Connections {
        target: painter
		function onBufferChanged() {
			paintImage.source = "image://painter/current?" + Math.random()
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
    /*ColorDialog {
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
	}*/
}
