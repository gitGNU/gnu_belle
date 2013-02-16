#include "animationimage.h"

#include <QFileInfo>

#include "utils.h"
#include "resource_manager.h"

AnimationImage::AnimationImage(const QString& path)
{
    mPixmap = new QPixmap(path);
    mMovie = 0;
    mFilePath = path;
    QFileInfo info(path);
    mFileName = info.fileName();

    //support for animated images
    mMovie = new QMovie(path);
    if (mMovie->frameCount() <= 1) {
        mMovie->deleteLater(); //movie isn't need afterall, so delete it
        mMovie = 0;
    }
    else {
        mMovie->jumpToFrame(0);
        QString movieName = mMovie->fileName();
        QString frameName = "";

        if (movieName.isEmpty())
            movieName = "image"; //shouldn't happen but just for precaution
        else //remove suffix
            movieName = QFileInfo(movieName).baseName();

        for(int i=0; i < mMovie->frameCount(); i++) {
            frameName = movieName + QString::number(i) + ".png"; //always save to PNG.
            mFramesNames.append(frameName);
            mMovie->jumpToNextFrame();
        }
    }
}

AnimationImage::AnimationImage(QPixmap* pixmap)
{
    mPixmap = pixmap;
    mMovie = 0;
    mFilePath = "";
    mFileName = "";
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

QStringList AnimationImage::framesNames() const
{
    return mFramesNames;
}

void AnimationImage::save(const QDir & dir)
{
    /*if (mMovie) {
        QMovie::MovieState prevState = mMovie->state();
        mMovie->stop();
        mMovie->jumpToFrame(0);
        for(int i=0; i < mMovie->frameCount(); i++) {
            mMovie->currentPixmap().save(Utils::newFileName(dir.absoluteFilePath(mFramesNames[i])));
            mMovie->jumpToNextFrame();
        }
        if (prevState == QMovie::Running)
            mMovie->start();
    }*/

    //save the original image
    bool saved = false;
    if (mPixmap)
        saved = mPixmap->save(dir.absoluteFilePath(mFileName));
    if (! saved)
        QFile::copy(mFilePath, dir.absoluteFilePath(mFileName));
}

QString AnimationImage::path()
{
    return mFilePath;
}

QRect AnimationImage::rect() const
{
    if (mMovie)
        return mMovie->frameRect();
    else if (mPixmap)
        return mPixmap->rect();

    return QRect();
}

void AnimationImage::setFileName(const QString & name)
{
    mFileName = name;
}

QString AnimationImage::fileName() const
{
    return mFileName;
}
