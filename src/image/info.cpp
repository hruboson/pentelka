#include "image/info.hpp"

ImageInfo::ImageInfo(QObject* parent) : QObject(parent) {}

void ImageInfo::setWidth(int w) {
    if (m_width != w) {
        m_width = w;
        emit widthChanged();
    }
}

void ImageInfo::setHeight(int h) {
    if (m_height != h) {
        m_height = h;
        emit heightChanged();
    }
}

void ImageInfo::setBitsPerPixel(int bpp) {
    if (m_bitsPerPixel != bpp) {
        m_bitsPerPixel = bpp;
        emit bitsPerPixelChanged();
    }
}

void ImageInfo::setFileSize(int size) {
    if (m_fileSize != size) {
        m_fileSize = size;
        emit fileSizeChanged();
    }
}

void ImageInfo::setCompression(int comp) {
    if (m_compression != comp) {
        m_compression = comp;
        emit compressionChanged();
    }
}

void ImageInfo::setType(IMAGE_TYPE type) {
    if (m_type != type) {
        m_type = type;
        emit typeChanged();
    }
}

QString ImageInfo::typeString() const {
    switch(m_type) {
        case IMAGE_TYPE::PNG: return "PNG";
        case IMAGE_TYPE::JPG: return "JPG";
        case IMAGE_TYPE::BMP: return "BMP";
        default: return "Unknown";
    }
}
