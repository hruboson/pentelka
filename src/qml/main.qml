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

	Canvas {
		id: canvas
		anchors {
			left: parent.left
			right: parent.right
			top: parent.top
			bottom: parent.bottom
			margins: 8
		}
		property real lastX
		property real lastY
		property color color: "#000000"

		onPaint: {
			var ctx = getContext('2d')
			ctx.lineWidth = 2 
			ctx.strokeStyle = canvas.color
			ctx.beginPath()
			ctx.moveTo(lastX, lastY)
			lastX = area.mouseX
			lastY = area.mouseY
			ctx.lineTo(lastX, lastY)
			ctx.stroke()
		}
		MouseArea {
			id: area
			anchors.fill: parent
			onPressed: {
				canvas.lastX = mouseX
				canvas.lastY = mouseY
			}
			onPositionChanged: {
				canvas.requestPaint()
			}
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
