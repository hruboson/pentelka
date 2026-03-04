import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 300
    height: 500
    title: "Image Information"
    
    // Receive the painter object
    property var painter: null
    
    Component.onCompleted: {
        if (painter) {
            console.log("Image info window opened with painter")
        } else {
            console.log("Warning: painter is null")
        }
    }
    
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 15
        width: parent.width - 40
        
        Label {
            text: "Image Information"
            font.bold: true
            font.pixelSize: 18
            color: "#333333"
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 10
        }
        
        GridLayout {
            columns: 2
            columnSpacing: 15
            rowSpacing: 10
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            
            Label { text: "Type:"; font.bold: true; color: "#555555" }
            Label { 
                text: painter && painter.imageInfo ? painter.imageInfo.typeString : "N/A"
                color: "#333333"
            }
            
            Label { text: "Width:"; font.bold: true; color: "#555555" }
            Label { 
                text: painter && painter.imageInfo ? painter.imageInfo.width + " px" : "N/A"
                color: "#333333"
            }
            
            Label { text: "Height:"; font.bold: true; color: "#555555" }
            Label { 
                text: painter && painter.imageInfo ? painter.imageInfo.height + " px" : "N/A"
                color: "#333333"
            }
            
            Label { text: "Total pixels:"; font.bold: true; color: "#555555" }
            Label { 
                text: painter && painter.imageInfo 
                    ? (painter.imageInfo.width * painter.imageInfo.height).toString() + " px²" 
                    : "N/A"
                color: "#333333"
            }
            
            Label { text: "Color depth:"; font.bold: true; color: "#555555" }
            Label { 
                text: painter && painter.imageInfo ? painter.imageInfo.bitsPerPixel + " bit" : "N/A"
                color: "#333333"
            }
            
            Label { text: "File size:"; font.bold: true; color: "#555555" }
            Label { 
                text: painter && painter.imageInfo && painter.imageInfo.fileSize > 0 
                    ? formatFileSize(painter.imageInfo.fileSize) 
                    : "N/A"
                color: "#333333"
            }
            
            Label { text: "Compression:"; font.bold: true; color: "#555555" }
            Label { 
                text: painter && painter.imageInfo 
                    ? (painter.imageInfo.compression === 0 ? "None" : painter.imageInfo.compression.toString())
                    : "N/A"
                color: "#333333"
            }
            
            Label { text: "Aspect ratio:"; font.bold: true; color: "#555555" }
            Label { 
                text: painter && painter.imageInfo && painter.imageInfo.height > 0
                    ? (painter.imageInfo.width / painter.imageInfo.height).toFixed(2)
                    : "N/A"
                color: "#333333"
            }
        }
        
        Rectangle {
            height: 1
            color: "#CCCCCC"
            Layout.fillWidth: true
            Layout.topMargin: 10
            Layout.bottomMargin: 10
        }
        
        RowLayout {
            spacing: 10
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 10
            
            Button {
                text: "Close"
                implicitWidth: 80
                onClicked: root.close()
            }
        }
    }
    
    function formatFileSize(bytes) {
        if (bytes < 1024) return bytes + " B"
        if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + " KB"
        if (bytes < 1024 * 1024 * 1024) return (bytes / (1024 * 1024)).toFixed(1) + " MB"
        return (bytes / (1024 * 1024 * 1024)).toFixed(1) + " GB"
    }
}
