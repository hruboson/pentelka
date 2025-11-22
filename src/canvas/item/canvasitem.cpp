#include "canvasitem.h"
#include <QPainter>

CanvasItem::CanvasItem(QQuickItem *parent)
    : QQuickPaintedItem(parent),
      m_penColor(Qt::black),
      m_penWidth(2),
      m_currentTool(Pen)
{
    m_image = QImage(800, 600, QImage::Format_ARGB32);
    m_image.fill(Qt::white);
}

void CanvasItem::startDraw(qreal x, qreal y) {
    m_lastPoint = QPoint(x, y);
}

void CanvasItem::drawTo(qreal x, qreal y) {
    if (m_image.isNull())
        return; // ensure QImage is valid

    QPainter painter(&m_image);
    painter.setPen(QPen(m_penColor, m_penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(m_lastPoint, QPoint(x, y));
    m_lastPoint = QPoint(x, y);

    update(); // schedule paint() call to display the image
}

void CanvasItem::paint(QPainter *painter) {
    if (m_image.isNull())
        return;
    painter->drawImage(0, 0, m_image);
}

void CanvasItem::clear() {
    if (!m_image.isNull()) {
        m_image.fill(Qt::white);
        update();
    }
}

void CanvasItem::saveImage(const QString &filePath) {
    // TODO
    return;
}

void CanvasItem::openImage(const QString &filePath) {
    // TODO
    return;
}


void CanvasItem::setPenColor(const QColor &c) { m_penColor = c; emit penColorChanged(); }
void CanvasItem::setPenWidth(int w) { m_penWidth = w; emit penWidthChanged(); }
