#pragma once

#include <QQuickImageProvider>
#include <QImage>

class PainterImageProvider : public QQuickImageProvider
{
public:
    PainterImageProvider(Painter* painter)
        : QQuickImageProvider(QQuickImageProvider::Image)
        , m_painter(painter)
    {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        QImage img = m_painter->getBuffer();
        if (size) *size = img.size();
        if (requestedSize.isValid())
            img = img.scaled(requestedSize);
        return img;
    }

private:
    Painter* m_painter;
};
