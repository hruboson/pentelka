#pragma once

#include <QObject>
#include <QImage>
#include <QColor>

class Painter : public QObject
{
    Q_OBJECT
public:
    explicit Painter(QObject *parent = nullptr);
	
	// drawing tools
	Q_INVOKABLE void setPixel(int x, int y, const QColor &color, int width);
	Q_INVOKABLE void drawLine(const QPoint &from, const QPoint &to, const QColor &color, int width);
	Q_INVOKABLE void drawWuLine(const QPoint &from, const QPoint &to, const QColor &color, int width); // better line algorithm

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
    QImage image_buffer; // canvas content stored here
    QTimer *updateTimer;
    bool pendingUpdate = false;
};
