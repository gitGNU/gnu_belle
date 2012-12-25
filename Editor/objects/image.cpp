/* Copyright (C) 2012 Carlos Pais
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "image.h"

#include <QDebug>
#include <QFileInfo>
#include <QMovie>

Image::Image(QPixmap *image, QObject *parent) :
    Object(parent)
{
    init();
    setImage(new AnimationImage(image));
}

Image::Image(const QString& path, QObject *parent) :
    Object(parent)
{
    init();
    setImage(path);
}

Image::Image(const QVariantMap& data, QObject* parent):
    Object(data, parent)
{
    init();

    if (data.contains("image") && data.value("image").type() == QVariant::String)
        setImage(data.value("image").toString());
}

Image::~Image()
{
    ResourceManager::decrementReference(mImage);
}

void Image::init()
{
    mImage = 0;
    setType("Image");
    mMovie = 0;
    mCurrentFrame = 0;

    if (objectName().isEmpty())
        setObjectName(ResourceManager::instance()->newName("image"));
}

void Image::setImage(const QString& path, bool deletePrevious)
{
    setImage(ResourceManager::newImage(path));
}

void Image::setImage(AnimationImage* image)
{
    if (image == mImage)
        return;

    ResourceManager::decrementReference(mImage);
    mImage = image;
    mMovie = 0;

    if (mImage && mImage->movie()) {
        mMovie = mImage->movie();
        connect(mMovie, SIGNAL(frameChanged(int)), this, SLOT(onFrameChanged(int)));

        if (mMovie->state() != QMovie::Running)
            mMovie->start();
    }

    if (mImage && !mImage->isNull() && (width() == 0 || height() == 0)) {
        mSceneRect.setWidth(mImage->width());
        mSceneRect.setHeight(mImage->height());
    }
}

AnimationImage* Image::image() const
{
    return mImage;
}

void Image::onFrameChanged(int frame)
{
    emit dataChanged();
}

void Image::paint(QPainter & painter)
{
    Object::paint(painter);
    if (mMovie)
        painter.drawPixmap(mSceneRect, mMovie->currentPixmap());
    else if (mImage)
        painter.drawPixmap(mSceneRect, *mImage);
}

void Image::show()
{
    if (mMovie)
        mMovie->start();
}

void Image::hide()
{
    if (mMovie)
        mMovie->stop();
}

QVariantMap Image::toJsonObject()
{
    QVariantMap object = Object::toJsonObject();
    if (mImage)
        object.insert("image", mImage->toJsonObject());

    filterResourceData(object);
    return object;
}
