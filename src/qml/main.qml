import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
	id: root
	width: 1200
	height: 800
	visible: true

	signal requestQuit()
	property int currentToolCursor: Qt.CrossCursor

	property string currentSavePath: ""
	property string pendingFolder: ""
	title: {
		if (currentSavePath === "") {
            return "Pentelka - Untitled"
        } else {
            // extract only the file name
            var parts = currentSavePath.split("/")
            return "Pentelka - " + parts[parts.length - 1]
		}
	}

	property color penColor: "#000000"

	property bool textMode: false
	function exitTextMode(){
		painter.commitText()
		canvasContainer.currentText = "";
		textMode = false;
		console.log("Exiting text mode")

		painter.selectBrush()
		root.currentToolCursor = Qt.CrossCursor
		painter.setPreview(false)
	}

	menuBar: MenuBar {
		Menu {
			title: qsTr("&File")

			Action { 
				text: qsTr("&New...") 
				onTriggered: {
					if (currentSavePath === "") {
						painter.clearBuffer()
					} else {
						currentSavePath = ""
						painter.clearBuffer()
					}
				}
			}

			Action { 
				text: qsTr("&Open...") 
				onTriggered: openDialog.open()
			}

			Action {
				text: qsTr("&Save")
				onTriggered: {
					if (currentSavePath !== "") {
						painter.saveImage(currentSavePath)
					} else {
						selectFolderDialog.open()
					}
				}
			}

			Action {
				text: qsTr("Save &As...")
				onTriggered: selectFolderDialog.open()
			}

			Action {
				text: qsTr("&Print")
				onTriggered: painter.requestPrint()
			}

			MenuSeparator { }

			Action { 
				text: qsTr("&Quit") 
				onTriggered: root.requestQuit()
			}
		}

		Menu {
			title: qsTr("&Edit")
			/*Action { text: qsTr("&Cut") }
			Action { text: qsTr("&Copy") }
			Action { text: qsTr("&Paste") }*/
			/*Action {
				text: qsTr("Options")
				onTriggered: {
					var component = Qt.createComponent("options.qml")
					var window = component.createObject(root)
					window.show()
				}
			}*/
			Action { 
				text: qsTr("Resize &Canvas")
				onTriggered: resizeCanvasDialog.open()
			}
		}

		Menu {
			title: qsTr("&Help")
			Action { 
				text: qsTr("&About") 
				onTriggered: aboutDialog.open()
			}
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

	FolderDialog {
		id: selectFolderDialog
		title: "Select Destination Folder"
		onAccepted: {
			pendingFolder = selectedFolder
			fileNameDialog.open()
		}
	}

	Dialog {
		id: fileNameDialog
		title: "Enter file name"
		modal: true
		x: (root.width - implicitWidth) / 2
		y: (root.height - implicitHeight) / 2

		standardButtons: Dialog.Ok | Dialog.Cancel

		property alias fileName: nameField.text

		Column {
			spacing: 10

			TextField {
				id: nameField
				placeholderText: "image-name.png"
				text: "untitled.png"
				width: 250

				Keys.onPressed: (event) => {
					if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
						event.accepted = true
						fileNameDialog.accept()
					}
				}
			}
		}

		onAccepted: {
			if (!fileName.includes(".")) {
				nameField.text = fileName + ".png" // default extension
			}

			let fullPath = pendingFolder + "/" + nameField.text
			console.log("Saving to:", fullPath)

			if (painter.saveImage(fullPath)) {
				currentSavePath = fullPath
			} else {
				console.log("Failed to save image")
			}
		}
	}

	Dialog {
		id: resizeCanvasDialog
		title: "Resize canvas"
		modal: true
		x: (root.width - implicitWidth) / 2
		y: (root.height - implicitHeight) / 2

		standardButtons: Dialog.Ok | Dialog.Cancel
		property int newWidth: canvasContainer.width
		property int newHeight: canvasContainer.height

		onOpened: {
			// load current canvas size
			newWidth = canvasContainer.width
			newHeight = canvasContainer.height
			widthField.text = newWidth.toString()
	        heightField.text = newHeight.toString()
		}

		ColumnLayout {
			Layout.margins: 20

			RowLayout {
				spacing: 10
				Label { text: "Width:" }
				TextField {
					id: widthField
					text: resizeCanvasDialog.newWidth.toString()
					validator: IntValidator { bottom: 1; top: 10000 } // only integers
					inputMethodHints: Qt.ImhDigitsOnly
					onTextChanged: {
						if (text !== "") {
							resizeCanvasDialog.newWidth = parseInt(text)
						}
					}
				}
			}

			RowLayout {
				spacing: 10
				Label { text: "Height:" }
				TextField {
					id: heightField
					text: resizeCanvasDialog.newHeight.toString()
					validator: IntValidator { bottom: 1; top: 10000 }
					inputMethodHints: Qt.ImhDigitsOnly
					onTextChanged: {
						if (text !== "") {
							resizeCanvasDialog.newHeight = parseInt(text)
						}
					}
				}
			}
		}

		onAccepted: {
			console.log("Resizing canvas to", newWidth, "x", newHeight)
			painter.resizeCanvas(newWidth, newHeight)
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
			width: 50
			anchors.fill: parent
			spacing: 10
			anchors.margins: 5

			// brush type buttons
			Button {
				text: "Brush"
				onClicked: {
					painter.selectBrush()
					textMode = false;
					root.currentToolCursor = Qt.CrossCursor
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
					root.currentToolCursor = Qt.UpArrowCursor
				}
			}
			Button {
				text: "Fill"
				onClicked: {
					painter.selectFill();
					textMode = false;
					root.currentToolCursor = Qt.PointingHandCursor
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
					root.currentToolCursor = Qt.IBeamCursor
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

			Text {
				text: "Pattern:"
			}
			ComboBox {
				id: patternSelector
				width: parent.width
				model: ["NONE", "CROSS", "DIAGCROSS", "DENSE"]
				currentIndex: 0
				onCurrentIndexChanged: {
					switch(currentIndex) {
						case 0: painter.selectPatternNONE(); break;
						case 1: painter.selectPatternCROSS(); break;
						case 2: painter.selectPatternDIAGCROSS(); break;
						case 3: painter.selectPatternDENSE(); break;
					}
					console.log("Selected pattern:", currentText)
				}
			}

			// current color info
			Text {
				text: "Color:"
			}
			Column {
				spacing: 3
				anchors.left: parent.left
				anchors.right: parent.right

				/*Image {
					id: patternPreview
					width: parent.width - 10
					height: 25
					fillMode: Image.PreserveAspectFit
					source: "image://painter/currentPattern?" + Math.random()

					MouseArea {
						anchors.fill: parent
						onClicked: colorDialog.open()
						cursorShape: Qt.PointingHandCursor
					}
				}*/

				Rectangle {
					width: parent.width - 10
					height: 25
					radius: 4
					color: penColor
					border.width: 1
					border.color: "#555555"

					MouseArea {
						anchors.fill: parent
						onClicked: colorDialog.open()
						cursorShape: Qt.PointingHandCursor
					}
				}

				Text {
					text: "RGB: " +
					Math.round(penColor.r * 255) + ", " +
					Math.round(penColor.g * 255) + ", " +
					Math.round(penColor.b * 255)
					font.pixelSize: 11
					horizontalAlignment: Text.AlignHCenter
					width: parent.width
				}

				Text {
					text: "HEX: " + penColor
					font.pixelSize: 11
					horizontalAlignment: Text.AlignHCenter
					width: parent.width
				}
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
					cursorShape: root.currentToolCursor
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
							// refresh pattern offset
							var x = Math.floor(Math.random()*1000)
							var y = Math.floor(Math.random()*1000)
							painter.setNewPatternOffset(x, y)

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

	Dialog {
		id: aboutDialog
		title: "About Pentelka"
		modal: true
		x: (root.width - implicitWidth) / 2
		y: (root.height - implicitHeight) / 2
		standardButtons: Dialog.Ok

		background: Rectangle { // fixes KDE theme schenanigans
			color: "#FFFFFF"
			radius: 6
			border.color: "#888888"
			border.width: 1
		}

		contentItem: Column {
			spacing: 15
			padding: 20
			width: 300

			Image {
				id: appIcon
				source: "qrc:/assets/icon/pentelka_icon.png"
				width: 64
				height: 64
				anchors.horizontalCenter: parent.horizontalCenter
			}

			Text {
				text: "Pentelka\nA simple drawing application."
				font.pixelSize: 14
	            color: "#000000"
				horizontalAlignment: Text.AlignHCenter
				wrapMode: Text.Wrap
				anchors.horizontalCenter: parent.horizontalCenter
			}

			Text {
				//TODO clickable link
				text: "https://github.com/hruboson/pentelka"
            	color: "#ffca24"
				horizontalAlignment: Text.AlignHCenter
				wrapMode: Text.Wrap
				anchors.horizontalCenter: parent.horizontalCenter
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

	Connections {
		target: root
		function onRequestQuit() {
			Qt.quit();
		}
	}
}
