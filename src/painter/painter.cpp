#include "painter.hpp"

#include <QPainter>
#include <cmath>
#include <QTimer>

static inline int ipart(float x) { return static_cast<int>(std::floor(x)); }
static inline float fpart(float x) { return x - std::floor(x); }
static inline float rfpart(float x) { return 1 - fpart(x); }

Painter::Painter(QObject *parent)
    : QObject(parent) {
    
	image_buffer = QImage(800, 600, QImage::Format_ARGB32_Premultiplied);
    image_buffer.fill(Qt::white);

    updateTimer = new QTimer(this);
    updateTimer->setInterval(16); // ~60 FPS, put FPS in settings later
    connect(updateTimer, &QTimer::timeout, [this]() {
        if (pendingUpdate) {
            pendingUpdate = false;
            emit bufferChanged();
        }
    });
    updateTimer->start();
}

void Painter::setPixel(int x, int y, const QColor &color) {
	if (x < 0 || y < 0 || x >= image_buffer.width() || y >= image_buffer.height())
        return;

    image_buffer.setPixelColor(x, y, color);
    pendingUpdate = true; // mark that buffer has changed
}

void Painter::drawLine(const QPoint &from, const QPoint &to, const QColor &color){
	// Bresenham
	int x0 = from.x();
    int y0 = from.y();
    int x1 = to.x();
    int y1 = to.y();

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);

    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;

    int err = dx - dy;

    while (true) {
        setPixel(x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void Painter::drawWuLine(const QPoint &from, const QPoint &to, const QColor &color){
	// Xaolin-Wu algorithm
	// src: https://dl.acm.org/doi/pdf/10.1145/127719.122734
	float x0 = from.x();
    float y0 = from.y();
    float x1 = to.x();
    float y1 = to.y();

    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    float dx = x1 - x0;
    float dy = y1 - y0;
    float gradient = dx == 0 ? 1 : dy / dx;

    // Handle first endpoint
    int xEnd = ipart(x0);
    float yEnd = y0 + gradient * (xEnd - x0);
    int xpxl1 = xEnd;
    int ypxl1 = ipart(yEnd);
    auto c = color;
    if (steep) {
        setPixel(ypxl1, xpxl1, QColor(c.red(), c.green(), c.blue(), int(c.alpha() * rfpart(yEnd))));
        setPixel(ypxl1 + 1, xpxl1, QColor(c.red(), c.green(), c.blue(), int(c.alpha() * fpart(yEnd))));
    } else {
        setPixel(xpxl1, ypxl1, QColor(c.red(), c.green(), c.blue(), int(c.alpha() * rfpart(yEnd))));
        setPixel(xpxl1, ypxl1 + 1, QColor(c.red(), c.green(), c.blue(), int(c.alpha() * fpart(yEnd))));
    }
    float intery = yEnd + gradient;

    // Handle second endpoint
    xEnd = ipart(x1);
    yEnd = y1 + gradient * (xEnd - x1);
    int xpxl2 = xEnd;
    int ypxl2 = ipart(yEnd);
    if (steep) {
        setPixel(ypxl2, xpxl2, QColor(c.red(), c.green(), c.blue(), int(c.alpha() * rfpart(yEnd))));
        setPixel(ypxl2 + 1, xpxl2, QColor(c.red(), c.green(), c.blue(), int(c.alpha() * fpart(yEnd))));
    } else {
        setPixel(xpxl2, ypxl2, QColor(c.red(), c.green(), c.blue(), int(c.alpha() * rfpart(yEnd))));
        setPixel(xpxl2, ypxl2 + 1, QColor(c.red(), c.green(), c.blue(), int(c.alpha() * fpart(yEnd))));
    }

    // main loop
    if (steep) {
        for (int x = xpxl1 + 1; x < xpxl2; x++) {
            int y = ipart(intery);
            setPixel(y, x, QColor(c.red(), c.green(), c.blue(), int(c.alpha() * rfpart(intery))));
            setPixel(y + 1, x, QColor(c.red(), c.green(), c.blue(), int(c.alpha() * fpart(intery))));
            intery += gradient;
        }
    } else {
        for (int x = xpxl1 + 1; x < xpxl2; x++) {
            int y = ipart(intery);
            setPixel(x, y, QColor(c.red(), c.green(), c.blue(), int(c.alpha() * rfpart(intery))));
            setPixel(x, y + 1, QColor(c.red(), c.green(), c.blue(), int(c.alpha() * fpart(intery))));
            intery += gradient;
        }
    }
}

bool Painter::loadImage(const QString &path) {
    QImage img;
    if (!img.load(path))
        return false;

    image_buffer = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    return true;
}

bool Painter::saveImage(const QString &path) {
    return image_buffer.save(path);
}

void Painter::resizeBuffer(int width, int height) {
    if (width <= 0 || height <= 0)
        return;

    QImage newBuffer(width, height, QImage::Format_ARGB32_Premultiplied);
    newBuffer.fill(Qt::white);

    QPainter p(&newBuffer);
    p.drawImage(0, 0, image_buffer);
    p.end();

    image_buffer = newBuffer;
}

void Painter::floodFill(int x, int y, const QColor &color) {
    // TODO: flood fill implementation
}

QImage Painter::getBuffer() const {
    return image_buffer;
}

void Painter::flush() {
    pendingUpdate = true;  // force an update
}
