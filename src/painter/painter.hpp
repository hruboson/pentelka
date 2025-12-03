#pragma once

#include "tools.hpp"

#include <QObject>
#include <QImage>
#include <QColor>

class Painter : public QObject
{
    Q_OBJECT
public:
    explicit Painter(QObject *parent = nullptr);

	// main drawing function
	Q_INVOKABLE void draw(const QPoint &from, const QPoint &to, const QColor &color, int width); // continuous function

	// tool selectors (should be called in qml)
	Q_INVOKABLE void selectBrush();
	Q_INVOKABLE void selectSpray();
	Q_INVOKABLE void selectEraser();
	
	// drawing tools
	Q_INVOKABLE void setPixel(int x, int y, const QColor &color, int width);
	Q_INVOKABLE void drawLine(const QPoint &from, const QPoint &to, const QColor &color, int width);
	Q_INVOKABLE void drawWuLine(const QPoint &from, const QPoint &to, const QColor &color, int width); // better line algorithm
	Q_INVOKABLE void sprayAt(const QPoint &at, const QColor &color, int width);

	// utilities
    Q_INVOKABLE bool loadImage(const QString &path);
    Q_INVOKABLE bool saveImage(const QString &path);
    Q_INVOKABLE void resizeBuffer(int width, int height);
    Q_INVOKABLE void floodFill(int x, int y, const QColor &color);

    // expose the image buffer to QML
    Q_INVOKABLE QImage getBuffer() const;
    Q_INVOKABLE void flush(); // emit bufferChanged once per frame

signals:
	void bufferChanged();

private:
	QColor backgroundColor = Qt::white;
    QImage image_buffer; // canvas content stored here
    QTimer *updateTimer;
    bool pendingUpdate = false;

	// Tool to paint with
	DRAWINGTOOLS selectedTool = DRAWINGTOOLS::BRUSH;
};
