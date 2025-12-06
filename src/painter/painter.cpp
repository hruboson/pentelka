#include "painter.hpp"

#include <cmath>
#include <iostream>

#include <QPainter>
#include <QTimer>
#include <QDebug>

static inline int ipart(float x) { return static_cast<int>(std::floor(x)); }
static inline float fpart(float x) { return x - std::floor(x); }
static inline float rfpart(float x) { return 1 - fpart(x); }

Painter::Painter(QObject *parent)
    : QObject(parent) {
    
	image_buffer = QImage(800, 600, QImage::Format_ARGB32_Premultiplied);
    image_buffer.fill(Qt::white);
	image_text_buffer = QImage(image_buffer.size(), QImage::Format_ARGB32_Premultiplied);
	image_text_buffer.fill(Qt::transparent);

	preview_buffer = QImage(image_buffer.size(), QImage::Format_ARGB32_Premultiplied);
	preview_buffer.fill(Qt::transparent);

	// canvas redraw timer
    updateTimer = new QTimer(this);
    updateTimer->setInterval(16); // ~60 FPS, put FPS in settings later
    connect(updateTimer, &QTimer::timeout, [this]() {
        if (pendingUpdate) {
            pendingUpdate = false;
            emit bufferChanged();
        }
    });
    updateTimer->start();

	// text caret timer -> currently doesn't work because updateText is only called on user keypress
	/*caretTimer = new QTimer(this);
    caretTimer->setInterval(500);
    connect(caretTimer, &QTimer::timeout, [this]() {
        caretVisible = !caretVisible;
        pendingUpdate = true; // request a redraw
    });
    caretTimer->start();*/
}

void Painter::selectSpray(){
	selectedTool = TOOLS::SPRAY;
}

void Painter::selectBrush(){
	selectedTool = TOOLS::BRUSH;
}

void Painter::selectEraser(){
	selectedTool = TOOLS::ERASER;
}

void Painter::selectNewText(){
	selectedTool = TOOLS::TEXT;
}

void Painter::draw(const QPoint &from, const QPoint &to, const QColor &color, int width){
	switch(selectedTool){
		case TOOLS::BRUSH:
			drawLine(from, to, color, width);
			break;
		case TOOLS::SPRAY:
			sprayAt(to, color, width);
			break;
		case TOOLS::ERASER:
			drawLine(from, to, backgroundColor, width);
			break;
		case TOOLS::TEXT:
			break;
		default:
			break;
	}
}

/************************
 *		PAINT TOOLS		*
 ***********************/

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

void Painter::sprayAt(const QPoint &pos, const QColor &color, int radius){
    // radius determines spray area
    int particles = radius * 10;   // spray density

    for (int i = 0; i < particles; ++i) {

        // random angle & distance (uniform circular distribution)
        float angle = (float(rand()) / RAND_MAX) * 2.0f * M_PI;

        float dist = std::sqrt(float(rand()) / RAND_MAX) * radius;

        int sx = pos.x() + std::cos(angle) * dist;
        int sy = pos.y() + std::sin(angle) * dist;

        // bounds check
        if (sx < 0 || sy < 0 ||
            sx >= image_buffer.width() ||
            sy >= image_buffer.height())
            continue;

        // spray dot — 1px with color
        image_buffer.setPixelColor(sx, sy, color);
    }

    pendingUpdate = true;
}

/************************
 *		TEXT TOOLS 		*
 ***********************/

void Painter::updateText(const QString &text, const QPoint &pos, const QColor &color, int fontSize){
    // clear previous preview
    image_text_buffer.fill(Qt::transparent);
    preview_buffer.fill(Qt::transparent);

	// draw text
    if (!text.isEmpty()) {
        QPainter pText(&image_text_buffer);
        pText.setRenderHint(QPainter::Antialiasing);
        pText.setRenderHint(QPainter::TextAntialiasing);

        QFont font;
        font.setPointSize(fontSize);
        pText.setFont(font);
        pText.setPen(color);
        pText.drawText(pos, text);
    }

    // draw caret
	//if (caretVisible) { // variable connected to the caret timer
	QPainter p(&preview_buffer);
	p.setRenderHint(QPainter::Antialiasing);
	p.setRenderHint(QPainter::TextAntialiasing);

	QFont font;
	font.setPointSize(fontSize);
	p.setFont(font);

	QFontMetrics fm(font);
	int textWidth = fm.horizontalAdvance(text); // 0 if empty
	int caretHeight = fm.height();

	QPen caretPen(Qt::black);
	caretPen.setWidth(2);
	p.setPen(caretPen);

	p.drawLine(pos.x() + textWidth, pos.y() - fm.ascent(),
			pos.x() + textWidth, pos.y() - fm.ascent() + caretHeight);
	//}

    pendingUpdate = true;
}

void Painter::commitText(){
	QPainter p(&image_buffer);
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.drawImage(0, 0, image_text_buffer);
    p.end();

    // reset text buffer state
    image_text_buffer.fill(Qt::transparent);

    pendingUpdate = true;
}

/************************
 *		UTILITIES		*
 ***********************/

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
    newBuffer.fill(backgroundColor);

    QPainter p(&newBuffer);
    p.drawImage(0, 0, image_buffer);
    p.end();

    image_buffer = newBuffer;
}

void Painter::floodFill(int x, int y, const QColor &color) {
    // TODO: flood fill implementation
}

QImage Painter::getBuffer() const {
	QImage combined = image_buffer.copy();

	QPainter p(&combined);

	if(showPreview){
		p.drawImage(0, 0, preview_buffer);
	}

	p.drawImage(0, 0, image_text_buffer);

	p.end();

	return combined;
}

void Painter::flush() {
    pendingUpdate = true;  // force an update
}


void Painter::setPreview(bool show){
	showPreview = show;
}
