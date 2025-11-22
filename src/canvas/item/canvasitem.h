#pragma once
#include <QQuickPaintedItem>
#include <QImage>
#include <QColor>
#include <QPen>

enum ToolType { Pen, Brush, Fill, Text };

class CanvasItem : public QQuickPaintedItem {
    Q_OBJECT
    Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor NOTIFY penColorChanged)
    Q_PROPERTY(int penWidth READ penWidth WRITE setPenWidth NOTIFY penWidthChanged)
public:
    CanvasItem(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    Q_INVOKABLE void startDraw(qreal x, qreal y);
    Q_INVOKABLE void drawTo(qreal x, qreal y);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void saveImage(const QString &filePath);
    Q_INVOKABLE void openImage(const QString &filePath);

    QColor penColor() const { return m_penColor; }
    void setPenColor(const QColor &c);
    int penWidth() const { return m_penWidth; }
    void setPenWidth(int w);

signals:
    void penColorChanged();
    void penWidthChanged();

private:
    QImage m_image;
    QPoint m_lastPoint;
    QColor m_penColor;
    int m_penWidth;
    ToolType m_currentTool;
};

