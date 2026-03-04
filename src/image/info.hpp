#pragma once

#include "image/types.hpp"
#include <QString>
#include <QObject>
#include <QColor>
#include <QVariantList>

class ImageInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(int height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(int bitsPerPixel READ bitsPerPixel WRITE setBitsPerPixel NOTIFY bitsPerPixelChanged)
    Q_PROPERTY(int fileSize READ fileSize WRITE setFileSize NOTIFY fileSizeChanged)
    Q_PROPERTY(int compression READ compression WRITE setCompression NOTIFY compressionChanged)
    Q_PROPERTY(QString typeString READ typeString NOTIFY typeChanged)

    Q_PROPERTY(QVariantList colorTable READ colorTable NOTIFY colorTableChanged)
    Q_PROPERTY(int colorCount READ colorCount NOTIFY colorTableChanged)
    
public:
    explicit ImageInfo(QObject* parent = nullptr);
    
    int width() const { return m_width; }
    void setWidth(int w);
    
    int height() const { return m_height; }
    void setHeight(int h);
    
    int bitsPerPixel() const { return m_bitsPerPixel; }
    void setBitsPerPixel(int bpp);
    
    int fileSize() const { return m_fileSize; }
    void setFileSize(int size);
    
    int compression() const { return m_compression; }
    void setCompression(int comp);
    
    QString typeString() const;
    void setType(IMAGE_TYPE type);
    
    IMAGE_TYPE type() const { return m_type; }

    QVariantList colorTable() const;
    void setColorTable(const QVector<QRgb>& colors);
    void addColor(const QColor& color);
    void clearColorTable();
    int colorCount() const { return m_colorTable.size(); }
    
signals:
    void widthChanged();
    void heightChanged();
    void bitsPerPixelChanged();
    void fileSizeChanged();
    void compressionChanged();
    void typeChanged();
    void colorTableChanged();
    
private:
    int m_width = 0;
    int m_height = 0;
    int m_bitsPerPixel = 0;
    int m_fileSize = 0;
    int m_compression = 0;
    IMAGE_TYPE m_type = IMAGE_TYPE::UNKNOWN;
    QVector<QRgb> m_colorTable;
};
