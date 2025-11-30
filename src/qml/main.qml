import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls

ApplicationWindow {
	id: root
	width: 1200
	height: 800
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
			Action { text: qsTr("&Cut") }
			Action { text: qsTr("&Copy") }
			Action { text: qsTr("&Paste") }
		}
		Menu {
			title: qsTr("&Help")
			Action { text: qsTr("&About") }
		}
	}

	// left vertical bar with tools
	Rectangle {
		id: toolbar
		width: toolsColumn.implicitWidth + 10
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		color: "#EEEEEE"
		z: 1

		Column {
			id: toolsColumn
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

	// floating top bar with stroke width slider
	Rectangle {
		id: topBar
		height: 30
		width: strokeSliderRow.implicitWidth + 30
		anchors.topMargin: 20

		anchors.top: parent.top
		anchors.horizontalCenter: parent.horizontalCenter

		color: "#f0f0f0"
		z: 999
		radius: 10 
		border.width: 1
		border.color: "#b0b0b0"

		Row {
			id: strokeSliderRow
			anchors.centerIn: parent
			spacing: 5

			Slider {
				id: strokeSlider

				from: 1
				to: 100
				value: 5
				width: 200

				onValueChanged: {
					painter.strokeWidth = value
				}
			}

			Text {
				text: Math.round(strokeSlider.value).toString()
				verticalAlignment: Text.AlignVCenter
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
					painter.setPixel(mouse.x, mouse.y, penColor, strokeSlider.value)
				}

				onPositionChanged: (mouse) => {
					if (canvasContainer.lastPoint) {
						painter.drawLine(canvasContainer.lastPoint, Qt.point(mouse.x, mouse.y), penColor, strokeSlider.value)
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
}
