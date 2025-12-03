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

	property bool textMode: false

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
				onClicked: {
					painter.selectBrush()
					textMode = false;
				}
			}
			Button {
				text: "Spray"
				onClicked: {
					painter.selectSpray()
					textMode = false;
				}
			}
			Button {
				text: "Eraser"
				onClicked: {
					painter.selectEraser();
					textMode = false;
				}
			}
			Button {
				text: "Text"
				onClicked: {
					painter.selectNewText()
					textMode = true
					console.log("Entered text mode")
				}
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

		FocusScope {
			anchors.fill: parent
			focus: true

			Keys.onPressed: (event) => {
				if (textMode) {
					canvasContainer.handleKey(event.key, event.text)
					event.accepted = true
				}
			}

			// canvas
			Rectangle {
				id: canvasContainer
				width: 800
				height: 600
				anchors.centerIn: parent

				property var lastPoint: null
				property point textInsertPos: Qt.point(0,0)
				property string currentText: ""

				function handleKey(key, text) {
					if (key === Qt.Key_Backspace) {
						currentText = currentText.slice(0, -1)
					} else if (key === Qt.Key_Escape) { // exit text mode
						painter.commitText()
						currentText = ""
						textMode = false
						console.log("Exiting text mode")
					} else if (text.length > 0) {
						currentText += text
					}

					painter.updateText(canvasContainer.currentText, canvasContainer.lastPoint, root.penColor, strokeSlider.value)
					console.log("Text: ", currentText)
				}

				Image {
					id: paintImage
					anchors.fill: parent
					source: "image://painter/current"
					// fillMode: Image.PreserveAspectFit  // prevent stretching
				}

				MouseArea {
					anchors.fill: parent
					onPressed: (mouse) => {
						canvasContainer.forceActiveFocus()
						if (textMode) {
							if (currentText !== "") {
								// clicking while text exists commits the text
								painter.commitText()
								currentText = ""
								textMode = false;
								console.log("Exiting text mode")
							}

							//TODO FIX THIS
							textInsertPos = Qt.point(mouse.x, mouse.y)
							console.log("New position: ", textInsertPos)
						} else {
							canvasContainer.lastPoint = Qt.point(mouse.x, mouse.y)
							painter.draw(canvasContainer.lastPoint, canvasContainer.lastPoint, penColor, strokeSlider.value)
						}
					}

					onPositionChanged: (mouse) => {
						if (canvasContainer.lastPoint) {
							painter.draw(canvasContainer.lastPoint, Qt.point(mouse.x, mouse.y), penColor, strokeSlider.value)
						}
						canvasContainer.lastPoint = Qt.point(mouse.x, mouse.y)
					}            
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
