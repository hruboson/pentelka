import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls

ApplicationWindow {
	id: root
	width: 800
	height: 600
	visible: true
	title: "Pentelka"

	property color penColor: "#000000"

	menuBar: MenuBar {
		Menu {
			title: qsTr("&File")
			Action { text: qsTr("&New...") }
			Action { text: qsTr("&Open...") }
			Action { text: qsTr("&Save") }
			Action { text: qsTr("Save &As...") }
			MenuSeparator { }
			Action { text: qsTr("&Quit") }
		}
		Menu {
			title: qsTr("&Edit")
			Action { text: qsTr("Cu&t") }
			Action { text: qsTr("&Copy") }
			Action { text: qsTr("&Paste") }
		}
		Menu {
			title: qsTr("&Help")
			Action { text: qsTr("&About") }
		}
	}

	Rectangle {
		id: toolbar
		width: columnContent.implicitWidth + 10
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		color: "#EEEEEE"
		z: 1

		Column {
			id: columnContent
			anchors.fill: parent
			spacing: 10
			anchors.margins: 5

			// brush type buttons
			Button {
				text: "Brush"
				onClicked: console.log("Brush selected")
			}
			Button {
				text: "Spray"
				onClicked: console.log("Spray selected")
			}
			Button {
				text: "Eraser"
				onClicked: console.log("Eraser selected")
			}

			// separator
			Rectangle { height: 1; width: parent.width; color: "#AAAAAA" }

			// color picker button
			Button {
				text: "Color"
				onClicked: colorDialog.open()
			}
		}
	}

	ColorDialog {
		id: colorDialog
		title: "Select Tool Color"
	    selectedColor: penColor
		onAccepted: {
			penColor = selectedColor
			console.log(penColor)
		}
	}

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
