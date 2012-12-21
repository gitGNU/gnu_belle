#include "animationimage.h"

AnimationImage::AnimationImage(const QString& path, QObject *parent) :
    QPixmap(path)
{
    mPixmap = this;
    mMovie = 0;

    //support for animated images
    mMovie = new QMovie(path);
    if (mMovie->frameCount() <= 1) {
        mMovie->deleteLater(); //movie isn't need afterall, so delete it
        mMovie = 0;
    }
}

AnimationImage::AnimationImage(QPixmap* pixmap, QObject *parent) :
    QPixmap(*pixmap)
{
    mPixmap = pixmap;
    mMovie = 0;
}

void AnimationImage::init()
{
}

QMovie* AnimationImage::movie()
{
    return mMovie;
}

QPixmap* AnimationImage::pixmap()
{
    return mPixmap;
}

bool AnimationImage::contains(QMovie * movie)
{
    if (movie && movie == mMovie)
        return true;
    return false;
}

bool AnimationImage::contains(QPixmap * pixmap)
{
    if (pixmap && pixmap == mPixmap)
        return true;
    return false;
}

int AnimationImage::width()
{
    if (mPixmap)
        return mPixmap->width();
    else if (mMovie)
        return mMovie->currentPixmap().width();
    return 0;
}

int AnimationImage::height()
{
    if (mPixmap)
        return mPixmap->height();
    else if (mMovie)
        return mMovie->currentPixmap().height();
    return 0;
}

bool AnimationImage::isNull()
{
    if (mPixmap)
        return mPixmap->isNull();
    else if (mMovie)
        return ! mMovie->isValid();
    return true;
}

