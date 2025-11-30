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

void Painter::setPixel(int x, int y, const QColor &color, int width) {
    if (width <= 1) {
        if (x >= 0 && y >= 0 && x < image_buffer.width() && y < image_buffer.height()) {
            image_buffer.setPixelColor(x, y, color);
            pendingUpdate = true;
        }
        return;
    }

    float r = width * 0.5f;
    float r2 = r * r;

    int minX = std::floor(x - r);
    int maxX = std::ceil (x + r);
    int minY = std::floor(y - r);
    int maxY = std::ceil (y + r);

    for (int py = minY; py <= maxY; ++py) {
        for (int px = minX; px <= maxX; ++px) {

            if (px < 0 || py < 0 ||
                px >= image_buffer.width() ||
                py >= image_buffer.height())
                continue;

            // distance from pixel center to brush center
            float dx = (px + 0.5f) - x;
            float dy = (py + 0.5f) - y;
            float dist = dx*dx + dy*dy;

            if (dist <= r2) {
                image_buffer.setPixelColor(px, py, color);
            }
        }
    }

    pendingUpdate = true; // mark that buffer has changed
}

void Painter::drawLine(const QPoint &from, const QPoint &to, const QColor &color, int width){
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
        setPixel(x0, y0, color, width);

        if (x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void Painter::drawWuLine(const QPoint &from, const QPoint &to, const QColor &color, int width){
	// Xaolin-Wu algorithm
	// src: https://dl.acm.org/doi/pdf/10.1145/127719.122734
	
	// Convert to floats
    float x0 = from.x();
    float y0 = from.y();
    float x1 = to.x();
    float y1 = to.y();

    // Stroke radius
    const float r = width * 0.5f;

    // Direction vector of line
    float dx = x1 - x0;
    float dy = y1 - y0;

    // Square of line length
    float len2 = dx*dx + dy*dy;
    if (len2 == 0.0f)
        return;

    float invLen2 = 1.0f / len2;

    // Bounding box of capsule (for iteration)
    int minX = std::floor(std::min(x0, x1) - r - 1);
    int maxX = std::ceil (std::max(x0, x1) + r + 1);
    int minY = std::floor(std::min(y0, y1) - r - 1);
    int maxY = std::ceil (std::max(y0, y1) + r + 1);

    // Clamp to buffer limits
    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, image_buffer.width() - 1);
    maxY = std::min(maxY, image_buffer.height() - 1);

    // Loop through all pixels in bounding box
    for (int py = minY; py <= maxY; ++py) {
        for (int px = minX; px <= maxX; ++px) {

            // Pixel center
            float cx = px + 0.5f;
            float cy = py + 0.5f;

            // Vector from start → pixel
            float vx = cx - x0;
            float vy = cy - y0;

            // Project onto line (parameter t)
            float t = (vx * dx + vy * dy) * invLen2;

            // Clamp to segment
            t = std::max(0.0f, std::min(1.0f, t));

            // Closest point on segment
            float closestX = x0 + dx * t;
            float closestY = y0 + dy * t;

            // Distance from pixel center to stroke centerline
            float distX = cx - closestX;
            float distY = cy - closestY;
            float dist = std::sqrt(distX*distX + distY*distY);

            // Compute coverage (smooth AA edge)
            float alpha = 0.0f;

            // Fading region: dist in [r-1, r]
            if (dist < r - 1.0f)
                alpha = 1.0f;
            else if (dist < r)
                alpha = (r - dist);   // linear falloff 0→1 within 1px
            else
                alpha = 0.0f;

            if (alpha > 0.0f) {
                QColor c = color;
                c.setAlphaF(c.alphaF() * alpha);
                setPixel(px, py, c, 1);   // width=1 → exact AA
            }
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
