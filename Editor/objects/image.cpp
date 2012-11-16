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

Image::Image(QPixmap *image, QObject *parent) :
    Object(parent)
{
    init();
    setImage(image);
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

    if (objectName().isEmpty())
        setObjectName(ResourceManager::instance()->newName("image"));
}

void Image::setImage(const QString& path, bool deletePrevious)
{
    setImage(ResourceManager::newImage(path), deletePrevious);
}

void Image::setImage(QPixmap * image, bool deletePrevious)
{
    if (image == mImage)
        return;

    //if (mImage && deletePrevious)
     //   delete mImage;

    mImage = image;

    if (mImage && !mImage->isNull() && (width() == 0 || height() == 0)) {
        mSceneRect.setWidth(mImage->width());
        mSceneRect.setHeight(mImage->height());
    }
}

QPixmap* Image::image() const
{
    return mImage;
}

QPixmap* Image::image(const QString &) const
{
    return 0;
}

void Image::paint(QPainter & painter)
{
    Object::paint(painter);

    if (mImage)
        painter.drawPixmap(mSceneRect, *mImage, mImage->rect());
}

QVariantMap Image::toJsonObject()
{
    QVariantMap object = Object::toJsonObject();
    QString imagePath = ResourceManager::imagePath(mImage);
    QFileInfo info(imagePath);
    if (mImage)
        object.insert("image", info.fileName());
    filterResourceData(object);
    return object;
}
