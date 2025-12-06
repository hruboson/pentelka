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
	function exitTextMode(){
		painter.commitText()
		canvasContainer.currentText = "";
		textMode = false;
		console.log("Exiting text mode")

		painter.selectBrush()
		painter.setPreview(false)
	}

	menuBar: MenuBar {
		Menu {
			title: qsTr("&File")
			Action { 
				text: qsTr("&New...") 
				onTriggered: {
					//TODO
				}
			}
			Action { 
				text: qsTr("&Open...") 
				onTriggered: openDialog.open()
			}

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

	FileDialog {
		id: openDialog
		title: "Open Image"
		currentFolder: shortcuts.home
		nameFilters: ["Images (*.png *.jpg *.bmp)"]
		onAccepted: {
			if (painter.loadImage(selectedFile)) {
				currentSavePath = selectedFile
			} else {
				console.log("Failed to load image:", selectedFile)
			}
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

			// separator
			Rectangle { height: 1; width: parent.width; color: "#AAAAAA" }

			Button {
				text: "Text"
				onClicked: {
					painter.selectNewText()
					painter.setPreview(true)
					textMode = true
					console.log("Entered text mode")
				}
			}

			// Text size input
			SpinBox {
				id: textSizeInput
				from: 1
				to: 200
				value: 15
				width: 50
				onValueChanged: painter.updateText(canvasContainer.currentText, canvasContainer.lastPoint, root.penColor, value)
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

			// Paint tool size input
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
					if(textMode){
						if (key === Qt.Key_Backspace) {
							currentText = currentText.slice(0, -1)
						} else if (key === Qt.Key_Escape) { // exit text mode
							root.exitTextMode()
						} else if (text.length > 0) {
							currentText += text
						}

						painter.updateText(canvasContainer.currentText, canvasContainer.lastPoint, root.penColor, textSizeInput.value)
						console.log("Text: ", currentText)
					}
				}

				Image {
					id: paintImage
					anchors.fill: parent
					source: "image://painter/current"
				}

				MouseArea {
					anchors.fill: parent
					id: canvasMouseArea
					cursorShape: textMode ? Qt.IBeamCursor : Qt.CrossCursor
					onPressed: (mouse) => {
						canvasContainer.lastPoint = Qt.point(mouse.x, mouse.y)
						console.log("Mouse click at [" + mouse.x + "," + mouse.y + "]")
						if (textMode) {
							canvasContainer.forceActiveFocus()
							if (canvasContainer.currentText != "") {
								root.exitTextMode()
							}

							// this line makes sure the caret is drawn even if text is empty
							painter.updateText(canvasContainer.currentText, canvasContainer.lastPoint, root.penColor, textSizeInput.value)
						} else {
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

	/*************************
	 * 		CONNECTIONS		 *
	 ************************/

	Connections {
		target: painter
		function onBufferChanged() {
			paintImage.source = "image://painter/current?" + Math.random()
		}
		function onImageSizeChanged(w, h) {
			console.log("Painter image size changed to", w, h)
			canvasContainer.width = w
			canvasContainer.height = h

			// center in parent
			canvasContainer.anchors.horizontalCenter = parent.horizontalCenter
			canvasContainer.anchors.verticalCenter = parent.verticalCenter

			// ensure the Image isn't independently scaling
			paintImage.fillMode = Image.PreserveAspectFit
		}
	}
 }
