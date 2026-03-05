#include "painter.hpp"
#include "image/types.hpp"
#include "image/info.hpp"

#include <cmath>
#include <iostream>
#include <cstdint>

#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QUrl>
#include <QPrinter>
#include <QPrintDialog>
#include <QFile>
#include <QFileInfo>

//TODO namespace pentelka

static inline int ipart(float x) { return static_cast<int>(std::floor(x)); }
static inline float fpart(float x) { return x - std::floor(x); }
static inline float rfpart(float x) { return 1 - fpart(x); }

Painter::Painter(QWidget* parentWidget, QObject *parent)
    : QObject(parent),
      parentWidget(parentWidget) {

	imageInfo = new ImageInfo(this);
    
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

/******************
 * TOOL SELECTORS *
 *****************/

void Painter::selectSpray(){
	selectedTool = TOOLS::SPRAY;
}

void Painter::selectBrush(){
	selectedTool = TOOLS::BRUSH;
}

void Painter::setNewPatternOffset(int x, int y){
	selectedPatternOffset.first = x;
	selectedPatternOffset.second = y;
}

void Painter::selectPatternNONE(){
	selectedPattern = TOOLPATTERN::NONE;
}

void Painter::selectPatternCROSS(){
	selectedPattern = TOOLPATTERN::CROSS;
}

void Painter::selectPatternDIAGCROSS(){
	selectedPattern = TOOLPATTERN::DIAGCROSS;
}

void Painter::selectPatternDENSE(){
	selectedPattern = TOOLPATTERN::DENSE;
}

void Painter::selectEraser(){
	selectedTool = TOOLS::ERASER;
}

void Painter::selectFill(){
	selectedTool = TOOLS::FILL;
}

void Painter::selectNewText(){
	selectedTool = TOOLS::TEXT;
}

void Painter::draw(const QPoint &from, const QPoint &to, const QColor &color, int width){
	const Pattern* pat = nullptr;

	switch(selectedTool){
		case TOOLS::BRUSH: {
            switch(selectedPattern){
                case TOOLPATTERN::CROSS:
                    pat = &PATTERN_CROSS;
                    break;
                case TOOLPATTERN::DIAGCROSS:
                    pat = &PATTERN_DIAGCROSS;
                    break;
                case TOOLPATTERN::DENSE:
                    pat = &PATTERN_DENSE;
                    break;
                default:
                    break;
            }
            drawLine(from, to, color, width, pat);
            break;
        }
		case TOOLS::SPRAY:
			sprayAt(to, color, width);
			break;
		case TOOLS::ERASER:
			drawLine(from, to, backgroundColor, width);
			break;
		case TOOLS::FILL:
            switch(selectedPattern){
                case TOOLPATTERN::CROSS:
                    pat = &PATTERN_CROSS;
                    break;
                case TOOLPATTERN::DIAGCROSS:
                    pat = &PATTERN_DIAGCROSS;
                    break;
                case TOOLPATTERN::DENSE:
                    pat = &PATTERN_DENSE;
                    break;
                default:
                    break;
            }
			fillArea(to, color, pat);
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

void Painter::setPixelPatterned(int x, int y, const QColor &color, int width, const Pattern* pattern) {
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

            float dx = (px + 0.5f) - x;
            float dy = (py + 0.5f) - y;
            float dist = dx*dx + dy*dy;

            if (dist > r2)
                continue;

            // pattern sampling stays 1:1, NOT scaled
			int tx = ((px + selectedPatternOffset.first) % pattern->W + pattern->W) % pattern->W;
			int ty = ((py + selectedPatternOffset.second) % pattern->H + pattern->H) % pattern->H;

			if (pattern->data[ty][tx] == 1)
                image_buffer.setPixelColor(px, py, color);
        }
    }

    pendingUpdate = true;
}

void Painter::drawLine(const QPoint &from, const QPoint &to, const QColor &color, int width, const Pattern* pattern){
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
		if (pattern) {
		    setPixelPatterned(x0, y0, color, width, pattern);
        } else {
            setPixel(x0, y0, color, width);
        }

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

void Painter::fillArea(const QPoint &at, const QColor &color, const Pattern *pattern){
	if (!image_buffer.rect().contains(at))
        return;

    const QRgb oldColor = image_buffer.pixel(at);
    const QRgb newColor = color.rgba();
    if (oldColor == newColor)
        return;

    const int w = image_buffer.width();
    const int h = image_buffer.height();

    std::vector<bool> visited(w * h, false);

    auto idx = [&](int x, int y){ return y * w + x; };

    std::vector<QPoint> stack;
    stack.reserve(8192);
    stack.push_back(at);

    while (!stack.empty()) {
        QPoint p = stack.back();
        stack.pop_back();

        int x = p.x();
        int y = p.y();

        if (x < 0 || x >= w || y < 0 || y >= h)
            continue;

        if (visited[idx(x, y)])
            continue;

        if (image_buffer.pixel(x, y) != oldColor)
            continue;

        // Expand left
        int lx = x;
        while (lx > 0 &&
               image_buffer.pixel(lx - 1, y) == oldColor &&
               !visited[idx(lx - 1, y)])
            lx--;

        // Expand right
        int rx = x;
        while (rx + 1 < w &&
               image_buffer.pixel(rx + 1, y) == oldColor &&
               !visited[idx(rx + 1, y)])
            rx++;

        // Fill span [lx, rx]
        for (int px = lx; px <= rx; px++) {
            visited[idx(px, y)] = true;

            if (pattern){
				int tx = ((px + selectedPatternOffset.first) % pattern->W + pattern->W) % pattern->W;
				int ty = ((y + selectedPatternOffset.second) % pattern->H + pattern->H) % pattern->H;
			
				if (pattern->data[ty][tx] == 1) {
					image_buffer.setPixelColor(px, y, color);
				}
			}else{
				setPixel(px, y, color, 1);
			}
        }

        // Scan upward
        if (y > 0) {
            int px = lx;
            while (px <= rx) {

                while (px <= rx &&
                       (visited[idx(px, y - 1)] ||
                        image_buffer.pixel(px, y - 1) != oldColor))
                    px++;
                if (px > rx) break;

                int start = px;

                while (px <= rx &&
                       !visited[idx(px, y - 1)] &&
                       image_buffer.pixel(px, y - 1) == oldColor)
                    px++;

                stack.emplace_back((start + px - 1) / 2, y - 1);
            }
        }

        // Scan downward
        if (y + 1 < h) {
            int px = lx;
            while (px <= rx) {

                while (px <= rx &&
                       (visited[idx(px, y + 1)] ||
                        image_buffer.pixel(px, y + 1) != oldColor))
                    px++;
                if (px > rx) break;

                int start = px;

                while (px <= rx &&
                       !visited[idx(px, y + 1)] &&
                       image_buffer.pixel(px, y + 1) == oldColor)
                    px++;

                stack.emplace_back((start + px - 1) / 2, y + 1);
            }
        }
    }

    pendingUpdate = true;}

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

	QPen caretPen(color);
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

void Painter::rotateImage(int degrees) {
    // normalize degrees to 0-360 range
    degrees = degrees % 360;
    if (degrees < 0) degrees += 360;
    
    if (degrees != 90 && degrees != 180 && degrees != 270) {
        qWarning() << "Only 90, 180, and 270 degree rotations are supported";
        return;
    }
    
    int oldWidth = image_buffer.width();
    int oldHeight = image_buffer.height();
    
    int newWidth = (degrees == 90 || degrees == 270) ? oldHeight : oldWidth;
    int newHeight = (degrees == 90 || degrees == 270) ? oldWidth : oldHeight;

    int originalBPP = imageInfo->bitsPerPixel();
    QImage::Format originalFormat = image_buffer.format();
    
    // vector of layers TODO move this to Painter class definition
    std::vector<std::reference_wrapper<QImage>> layers = {
        image_buffer,
        image_text_buffer,
        preview_buffer
    };
    
    // layers for rotated image
    std::vector<QImage> rotatedLayers;
    rotatedLayers.reserve(layers.size());
    
	for (size_t i = 0; i < layers.size(); ++i) {
		QImage newLayer(newWidth, newHeight, originalFormat);

		if (i == 0) { // main buffer
			if (originalFormat == QImage::Format_ARGB32_Premultiplied || 
					originalFormat == QImage::Format_ARGB32) {
				newLayer.fill(Qt::white);
			} else {
				newLayer.fill(backgroundColor.rgba());
			}
		} else { // text and preview buffers - always transparent
			newLayer.fill(Qt::transparent);
		}

		rotatedLayers.push_back(std::move(newLayer));
	}

	for (size_t layerIdx = 0; layerIdx < layers.size(); ++layerIdx) {
		const QImage& sourceLayer = layers[layerIdx].get();
		QImage& targetLayer = rotatedLayers[layerIdx];

		if (sourceLayer.isNull()) continue;

		if (originalBPP <= 8 && layerIdx == 0) {
			for (int y = 0; y < oldHeight; ++y) {
				for (int x = 0; x < oldWidth; ++x) {
					int pixelIndex = sourceLayer.pixelIndex(x, y); // get color table index

					int newX, newY;
					switch (degrees) {
						case 90:
							newX = y;
							newY = newHeight - 1 - x;
							break;
						case 180:
							newX = newWidth - 1 - x;
							newY = newHeight - 1 - y;
							break;
						case 270:
							newX = newWidth - 1 - y;
							newY = x;
							break;
						default:
							return;
					}

					targetLayer.setPixel(newX, newY, pixelIndex);
				}
			}
		} else {
			// ARGB format
			for (int y = 0; y < oldHeight; ++y) {
				for (int x = 0; x < oldWidth; ++x) {
					QRgb pixel = sourceLayer.pixel(x, y);

					int newX, newY;
					switch (degrees) {
						case 90:
							newX = y;
							newY = newHeight - 1 - x;
							break;
						case 180:
							newX = newWidth - 1 - x;
							newY = newHeight - 1 - y;
							break;
						case 270:
							newX = newWidth - 1 - y;
							newY = x;
							break;
						default:
							return;
					}

					targetLayer.setPixel(newX, newY, pixel);
				}
			}
		}
	}
    
    // copy edited layers to original layer buffers
    image_buffer = std::move(rotatedLayers[0]);
    image_text_buffer = std::move(rotatedLayers[1]);
    preview_buffer = std::move(rotatedLayers[2]);
    
    imageInfo->setWidth(image_buffer.width());
    imageInfo->setHeight(image_buffer.height());
    
    pendingUpdate = true;
    emit bufferChanged();
    emit imageSizeChanged(image_buffer.width(), image_buffer.height());
}

void Painter::flipImage(AXIS axis) {
    int width = image_buffer.width();
    int height = image_buffer.height();
    
    // vector of layers TODO move this to Painter class definition
    std::vector<std::reference_wrapper<QImage>> layers = {
        image_buffer,
        image_text_buffer,
        preview_buffer
    };
    
    // vector to store the new layers after flipping
    std::vector<QImage> flippedLayers;
    flippedLayers.reserve(layers.size());
    
    for (size_t i = 0; i < layers.size(); ++i) {
        QImage newLayer(width, height, QImage::Format_ARGB32_Premultiplied);
        newLayer.fill(Qt::transparent);
        flippedLayers.push_back(std::move(newLayer));
    }
    
    for (size_t layerIdx = 0; layerIdx < layers.size(); ++layerIdx) {
        const QImage& sourceLayer = layers[layerIdx].get();
        QImage& targetLayer = flippedLayers[layerIdx];
        
        if (sourceLayer.isNull()) continue;
        
        if (axis == AXIS::HORIZONTALLY) {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int newX = width - 1 - x;
                    targetLayer.setPixel(newX, y, sourceLayer.pixel(x, y));
                }
            }
        } 
        else if (axis == AXIS::VERTICALLY) {
            for (int y = 0; y < height; ++y) {
                int newY = height - 1 - y;
                for (int x = 0; x < width; ++x) {
                    targetLayer.setPixel(x, newY, sourceLayer.pixel(x, y));
                }
            }
        }
    }
    
    // copy flipped layers to original layers
    image_buffer = std::move(flippedLayers[0]);
    image_text_buffer = std::move(flippedLayers[1]);
    preview_buffer = std::move(flippedLayers[2]);
    
    pendingUpdate = true;
    emit bufferChanged();
}

/************************
 *		UTILITIES		*
 ***********************/

void Painter::createBuffers(int width, int height, bool copyOldContent = false) {
    if (width <= 0 || height <= 0) return;

    QImage newMain(width, height, QImage::Format_ARGB32_Premultiplied);
    newMain.fill(backgroundColor);

    QImage newText(width, height, QImage::Format_ARGB32_Premultiplied);
    newText.fill(Qt::transparent);

    QImage newPrev(width, height, QImage::Format_ARGB32_Premultiplied);
    newPrev.fill(Qt::transparent);

    if (copyOldContent) {
        int offsetX = (width - image_buffer.width()) / 2;
        int offsetY = (height - image_buffer.height()) / 2;

        QPainter pMain(&newMain);
        pMain.drawImage(offsetX, offsetY, image_buffer);
        pMain.end();

        QPainter pText(&newText);
        pText.drawImage(offsetX, offsetY, image_text_buffer);
        pText.end();

        QPainter pPrev(&newPrev);
        pPrev.drawImage(offsetX, offsetY, preview_buffer);
        pPrev.end();
    }

    image_buffer = std::move(newMain);
    image_text_buffer = std::move(newText);
    preview_buffer = std::move(newPrev);

    pendingUpdate = true;
    emit bufferChanged();
    emit imageSizeChanged(width, height);
}

bool Painter::loadImage(const QString &path) {
    QString local = QUrl(path).toLocalFile();
    if (local.isEmpty())
        local = path;

    if (local.endsWith(".bmp", Qt::CaseInsensitive)) {
        return loadBMP(local);  // custom BMP loading logic
    }

    QImage img;
    if (!img.load(local)) {
        qWarning() << "Failed to load image:" << path;
        return false;
    }

    // set main buffer to loaded image
    image_buffer = img.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    // resize text & preview buffers to match
    image_text_buffer = QImage(image_buffer.size(), QImage::Format_ARGB32_Premultiplied);
    image_text_buffer.fill(Qt::transparent);

    preview_buffer = QImage(image_buffer.size(), QImage::Format_ARGB32_Premultiplied);
    preview_buffer.fill(Qt::transparent);

    // POPULATE IMAGE INFO
    imageInfo->setWidth(image_buffer.width());
    imageInfo->setHeight(image_buffer.height());
    imageInfo->setBitsPerPixel(image_buffer.depth());

    QFile file(local);
    if (file.open(QIODevice::ReadOnly)) {
        imageInfo->setFileSize(file.size());
        file.close();
    }
    
    // set image type based on extension
    if (local.endsWith(".png", Qt::CaseInsensitive)) {
        imageInfo->setType(IMAGE_TYPE::PNG);
    }
    else if (local.endsWith(".jpg", Qt::CaseInsensitive) || local.endsWith(".jpeg", Qt::CaseInsensitive)) {
        imageInfo->setType(IMAGE_TYPE::JPG);
    }
    else if (local.endsWith(".bmp", Qt::CaseInsensitive)) {
        imageInfo->setType(IMAGE_TYPE::BMP);
    }
    else {
        imageInfo->setType(IMAGE_TYPE::UNKNOWN);
    }

    pendingUpdate = true;
    emit bufferChanged();
    emit imageSizeChanged(image_buffer.width(), image_buffer.height());

    qDebug() << "Loaded image:" << local
             << "Size:" << imageInfo->width() << "x" << imageInfo->height()
             << "BPP:" << imageInfo->bitsPerPixel()
             << "Type:" << imageInfo->typeString()
             << "File size:" << imageInfo->fileSize() << "bytes";

    return true;
}

bool Painter::loadBMP(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open BMP file:" << path;
        return false;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    // BMP FILE HEADER (14 bytes)
    quint16 signature;
    quint32 fileSize, reserved, dataOffset;
    in >> signature;
    if (signature != 0x4D42) { // 'BM'
        qWarning() << "Not a valid BMP file signature";
        return false;
    }
    in >> fileSize >> reserved >> dataOffset;

	this->imageInfo->setType(IMAGE_TYPE::BMP);
	this->imageInfo->setFileSize(fileSize);

    // DIB HEADER
    quint32 dibHeaderSize;
    in >> dibHeaderSize;

    qint32 width = 0, height = 0;
    quint16 planes = 0, bitsPerPixel = 0;
    quint32 compression = 0;
    quint32 colorsUsed = 0;

    if (dibHeaderSize == 12) { // BITMAPCOREHEADER (OS/2)
        quint16 w16, h16;
        in >> w16 >> h16 >> planes >> bitsPerPixel;
        width = w16; height = h16;
    } else if (dibHeaderSize >= 40) { // BITMAPINFOHEADER or larger
        in >> width >> height >> planes >> bitsPerPixel >> compression;
        // Skip next 12 bytes to get to colorsUsed (imageSize, xPpm, yPpm)
        in.skipRawData(12); 
        in >> colorsUsed;
    } else {
        qWarning() << "Unsupported DIB header size:" << dibHeaderSize;
        return false;
    }

    // DIMENSION CHECK - let's try not to overload the program
    if (width <= 0 || std::abs(height) <= 0 || std::abs(height) > 10000 || width > 10000) {
        qWarning() << "Invalid BMP dimensions:" << width << "x" << height;
        return false;
    }

	this->imageInfo->setWidth(width);
	this->imageInfo->setHeight(height);
	this->imageInfo->setBitsPerPixel(bitsPerPixel);

    bool topDown = (height < 0);
    height = std::abs(height);

    // COLOR TABLE (for 8-bit or less)
    QVector<QRgb> colorTable;
    if (bitsPerPixel <= 8) {
        int numColors = (colorsUsed == 0) ? (1 << bitsPerPixel) : colorsUsed;
        
        // Get to the start of the table (Header + 14)
        file.seek(14 + dibHeaderSize);
        
        for (int i = 0; i < numColors; ++i) {
            quint8 b, g, r, a;
            in >> b >> g >> r;
            if (dibHeaderSize > 12) in >> a; // Windows headers use 4 bytes per entry
            colorTable.append(qRgb(r, g, b));
        }
        this->imageInfo->setColorTable(colorTable);
    } else {
        // clear any existing color table for non-indexed images
        this->imageInfo->clearColorTable();
    }

	

    // READ PIXEL DATA
	// QImage is optimized class for pixel access and manipulation
    QImage bmpImg(width, height, QImage::Format_ARGB32_Premultiplied);
    file.seek(dataOffset); // Get directly to pixel data

    int bytesPerLine = ((width * bitsPerPixel + 31) / 32) * 4;
    QByteArray lineData(bytesPerLine, 0);

    for (int y = 0; y < height; ++y) {
        if (file.read(lineData.data(), bytesPerLine) != bytesPerLine) break;

        int targetY = topDown ? y : (height - 1 - y);
        const uchar* src = reinterpret_cast<const uchar*>(lineData.constData());

        for (int x = 0; x < width; ++x) {
            QRgb pixelColor;
            if (bitsPerPixel == 24) {
                pixelColor = qRgb(src[x*3+2], src[x*3+1], src[x*3]);
            } else if (bitsPerPixel == 32) {
                pixelColor = qRgba(src[x*4+2], src[x*4+1], src[x*4], src[x*4+3]);
            } else if (bitsPerPixel == 8) {
                pixelColor = colorTable.value(src[x], qRgb(0,0,0));
            } else if (bitsPerPixel == 4) {
                int val = (x % 2 == 0) ? (src[x/2] >> 4) : (src[x/2] & 0x0F);
                pixelColor = colorTable.value(val, qRgb(0,0,0));
            } else if (bitsPerPixel == 1) {
                int val = (src[x/8] >> (7 - (x % 8))) & 0x01;
                pixelColor = colorTable.value(val, qRgb(0,0,0));
            }
            bmpImg.setPixel(x, targetY, pixelColor);
        }
    }

    // INITIALIZE BUFFER
    image_buffer = std::move(bmpImg);
    image_text_buffer = QImage(image_buffer.size(), QImage::Format_ARGB32_Premultiplied);
    image_text_buffer.fill(Qt::transparent);
    preview_buffer = QImage(image_buffer.size(), QImage::Format_ARGB32_Premultiplied);
    preview_buffer.fill(Qt::transparent);

    pendingUpdate = true;
    emit bufferChanged();
    emit imageSizeChanged(image_buffer.width(), image_buffer.height());
    return true;
}

bool Painter::saveBMP(const QString &path, int bpp){
    QString local = QUrl(path).toLocalFile();
    if (local.isEmpty())
        local = path;
    
    QFile file(local);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << local;
        return false;
    }

    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);

    int width = image_buffer.width();
    int height = image_buffer.height();
    
    // check for supported bpp
    if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 24) {
        qWarning() << "Unsupported BMP bits per pixel:" << bpp;
        return false;
    }

    // calculate bytes per line (padded to 4-byte boundary)
    int bytesPerLine = ((width * bpp + 31) / 32) * 4;
    int imageSize = bytesPerLine * height;
    
    int colorTableSize = 0;
    QVector<QRgb> colorTable;
    
    if (bpp <= 8) {
        if (bpp == 1) {
			// b&w image
            colorTable.clear();
            colorTable.append(qRgb(0, 0, 0));        // black at index 0
            colorTable.append(qRgb(255, 255, 255));  // white at index 1
        } else {
            // generate colors for 4-bit and 8-bit
            QSet<QRgb> uniqueColors;
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    uniqueColors.insert(image_buffer.pixel(x, y) & 0x00FFFFFF); // ignore alpha
                }
            }

            // limit to max colors
            QList<QRgb> colors = uniqueColors.values();
            int maxColors = 1 << bpp;
            
            if (colors.size() > maxColors) {
                colors = colors.mid(0, maxColors);
            }
            
            colorTable = colors.toVector();
            
            // padding
            int targetSize = 1 << bpp;
            while (colorTable.size() < targetSize) {
                colorTable.append(qRgb(0, 0, 0));
            }
        }
        
        colorTableSize = colorTable.size() * 4; // 4 bytes per entry
        imageInfo->setColorTable(colorTable); // update imageInfo
    } else {
		// for images with no color table clear imageInfo color table
        imageInfo->clearColorTable();
    }

    // calculate file offsets
    int dibHeaderSize = 40; // BITMAPINFOHEADER
    int dataOffset = 14 + dibHeaderSize + colorTableSize;
    int fileSize = dataOffset + imageSize;

    // write header
    quint16 signature = 0x4D42; // 'BM'
    quint32 reserved = 0;
    
    out << signature;
    out << (quint32)fileSize;
    out << reserved;
    out << (quint32)dataOffset;

    // write DIB HEADER
    out << (quint32)dibHeaderSize;
    out << (qint32)width;
    out << (qint32)height;  // positive = bottom-up
    out << (quint16)1;      // color planes
    out << (quint16)bpp;
    out << (quint32)0;      // compression
    out << (quint32)imageSize;
    out << (qint32)2835;    // horizontal resolution (72 DPI ≈ 2835 pixels/meter)
    out << (qint32)2835;    // vertical resolution
    out << (quint32)colorTable.size();  // colors used
    out << (quint32)0;      // important colors (0 = all)

    // write color table
    if (bpp <= 8) {
        for (const QRgb& color : colorTable) {
            quint8 r = qRed(color);
            quint8 g = qGreen(color);
            quint8 b = qBlue(color);
            quint8 reserved = 0;
            
            out << b << g << r << reserved;  // BMP uses BGR order
        }
    }

    // write pixels
    QByteArray lineData;
    lineData.resize(bytesPerLine);
    
    for (int y = height - 1; y >= 0; --y) {  // BMP stores bottom-up
        lineData.fill(0); // clear buffer
        
        if (bpp == 24) {
            for (int x = 0; x < width; ++x) {
                QRgb pixel = image_buffer.pixel(x, y);
                lineData[x*3] = static_cast<char>(qBlue(pixel));
                lineData[x*3 + 1] = static_cast<char>(qGreen(pixel));
                lineData[x*3 + 2] = static_cast<char>(qRed(pixel));
            }
        }
        else if (bpp == 8) {
            for (int x = 0; x < width; ++x) {
                QRgb pixel = image_buffer.pixel(x, y) & 0x00FFFFFF;
                
                // Find closest color in the palette
                int bestIndex = 0;
                int bestDist = INT_MAX;
                
                for (int i = 0; i < colorTable.size(); ++i) {
                    QRgb palColor = colorTable[i];
                    int dr = qRed(pixel) - qRed(palColor);
                    int dg = qGreen(pixel) - qGreen(palColor);
                    int db = qBlue(pixel) - qBlue(palColor);
                    int dist = dr*dr + dg*dg + db*db;
                    
                    if (dist < bestDist) {
                        bestDist = dist;
                        bestIndex = i;
                    }
                }
                
                lineData[x] = static_cast<char>(bestIndex);
            }
        }
        else if (bpp == 4) {
            for (int x = 0; x < width; x += 2) {
                QRgb pixel1 = image_buffer.pixel(x, y) & 0x00FFFFFF;
                
                int bestIndex1 = 0;
                int bestDist1 = INT_MAX;
                
                for (int i = 0; i < colorTable.size(); ++i) {
                    QRgb palColor = colorTable[i];
                    int dr = qRed(pixel1) - qRed(palColor);
                    int dg = qGreen(pixel1) - qGreen(palColor);
                    int db = qBlue(pixel1) - qBlue(palColor);
                    int dist = dr*dr + dg*dg + db*db;
                    
                    if (dist < bestDist1) {
                        bestDist1 = dist;
                        bestIndex1 = i;
                    }
                }
                
                quint8 byteVal = (bestIndex1 << 4);
                
                if (x + 1 < width) {
                    QRgb pixel2 = image_buffer.pixel(x + 1, y) & 0x00FFFFFF;
                    
                    int bestIndex2 = 0;
                    int bestDist2 = INT_MAX;
                    
                    for (int i = 0; i < colorTable.size(); ++i) {
                        QRgb palColor = colorTable[i];
                        int dr = qRed(pixel2) - qRed(palColor);
                        int dg = qGreen(pixel2) - qGreen(palColor);
                        int db = qBlue(pixel2) - qBlue(palColor);
                        int dist = dr*dr + dg*dg + db*db;
                        
                        if (dist < bestDist2) {
                            bestDist2 = dist;
                            bestIndex2 = i;
                        }
                    }
                    
                    byteVal |= (bestIndex2 & 0x0F);
                }
                
                lineData[x/2] = static_cast<char>(byteVal);
            }
        }
        else if (bpp == 1) {
            for (int x = 0; x < width; x += 8) {
                quint8 byteVal = 0;
                
                for (int bit = 0; bit < 8; ++bit) {
                    if (x + bit < width) {
                        QRgb pixel = image_buffer.pixel(x + bit, y);
                        
                        // For 1-bit, use index 0 for dark colors, index 1 for light colors
                        int luminance = qGray(pixel);
                        int bitVal = (luminance > 128) ? 1 : 0;
                        
                        byteVal |= (bitVal << (7 - bit));
                    }
                }
                
                lineData[x/8] = static_cast<char>(byteVal);
            }
        }
        
        // write the line using write() instead of QDataStream
        qint64 bytesWritten = file.write(lineData);
        if (bytesWritten != bytesPerLine) {
            qWarning() << "Failed to write pixel data at line" << y 
                       << "Expected:" << bytesPerLine 
                       << "Written:" << bytesWritten;
            file.close();
            return false;
        }
    }

    file.close();
    
    // update image info
    imageInfo->setType(IMAGE_TYPE::BMP);
    imageInfo->setBitsPerPixel(bpp);
    imageInfo->setCompression(0);
    imageInfo->setWidth(width);
    imageInfo->setHeight(height);
    
    QFileInfo fileInfo(local);
    if (fileInfo.exists()) {
        imageInfo->setFileSize(fileInfo.size());
    }
    
    qDebug() << "Successfully saved BMP:" << local
             << "Size:" << width << "x" << height
             << "BPP:" << bpp
             << "Colors:" << colorTable.size()
             << "File size:" << fileInfo.size();
    
    return true;
}

bool Painter::saveImage(const QString &path) {
    QString local = QUrl(path).toLocalFile();
    if (local.isEmpty())
        local = path;

    if (local.endsWith(".bmp", Qt::CaseInsensitive)) {
        int bpp = imageInfo->bitsPerPixel();
        if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 24) {
            bpp = 24;  // default to 24-bit if current bpp is unsupported
        }
        return saveBMP(local, bpp);
    }

    // determine extension automatically from file name - QImage::save handles it
    bool ok = image_buffer.save(local);
    if (!ok) qWarning() << "Failed to save image:" << local;
    return ok;
}

void Painter::requestPrint() {
    QPrinter printer;
	printer.setPrinterName("Generic printer");

	QPrintDialog dialog(&printer, parentWidget);
	dialog.setWindowTitle(tr("Print Document"));
	if (dialog.exec() != QDialog::Accepted)
		return;

    QPainter painter(&printer);
    if (!painter.isActive())
        return;

    QImage finalImage = getBuffer();
    QRectF pageRect = printer.pageRect(QPrinter::DevicePixel);

    QSizeF imageSize = finalImage.size();
    imageSize.scale(pageRect.size(), Qt::KeepAspectRatio);

    int x = pageRect.x() + (pageRect.width()  - imageSize.width())  / 2;
    int y = pageRect.y() + (pageRect.height() - imageSize.height()) / 2;

    QRect targetRect(x, y, imageSize.width(), imageSize.height());

    // draw the buffer onto the printer
    painter.drawImage(targetRect, finalImage);
}

void Painter::resizeCanvas(int width, int height) {
    createBuffers(width, height, true);
}

void Painter::resizeBuffer(int width, int height) {
	if (width <= 0 || height <= 0)
		return;

    // create new buffer and paint old content into it
    QImage newBuffer(width, height, QImage::Format_ARGB32_Premultiplied);
    newBuffer.fill(backgroundColor);

    QPainter p(&newBuffer);
    p.drawImage(0, 0, image_buffer);
    p.end();

    image_buffer = newBuffer;

    // update text and preview buffers to match new size
    image_text_buffer = QImage(image_buffer.size(), QImage::Format_ARGB32_Premultiplied);
    image_text_buffer.fill(Qt::transparent);

    preview_buffer = QImage(image_buffer.size(), QImage::Format_ARGB32_Premultiplied);
    preview_buffer.fill(Qt::transparent);

    pendingUpdate = true;
    emit bufferChanged();
    emit imageSizeChanged(width, height);
}

void Painter::clearBuffer(int width, int height) {
    delete imageInfo;
    imageInfo = new ImageInfo(this);
    createBuffers(width, height, false);
}

void Painter::clearBuffer(){
	clearBuffer(800, 600);
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

/*QImage Painter::getPatternPreview(int width, int height, const QColor &color) const {
    QImage preview(width, height, QImage::Format_ARGB32_Premultiplied);
    preview.fill(Qt::white);

    QPainter qPainter(&preview);
    qPainter.setRenderHint(QPainter::Antialiasing);

    // use the same draw function for consistency
    QPoint from(0, 0);
    QPoint to(width - 1, height - 1);

    const Pattern* pat = nullptr;
    switch(selectedPattern){
        case TOOLPATTERN::CROSS: pat = &PATTERN_CROSS; break;
        case TOOLPATTERN::DIAGCROSS: pat = &PATTERN_DIAGCROSS; break;
        case TOOLPATTERN::DENSE: pat = &PATTERN_DENSE; break;
        default: pat = nullptr; break;
    }

    // temporarily draw into the preview QImage
    QImage originalBuffer = image_buffer;
    const_cast<Painter*>(this)->image_buffer = preview;
    const_cast<Painter*>(this)->drawLine(from, to, color, 10, pat);
    const_cast<Painter*>(this)->image_buffer = originalBuffer;

    return preview;
}*/
