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

#include "resource_manager.h"

#include <QDebug>
#include <QMovie>

#include "utils.h"
#include "image.h"
#include "character.h"
#include "dialoguebox.h"
#include "textbox.h"
#include "button.h"
#include "menu.h"
#include "animationimage.h"

static QList<Object*> mResources;
static ResourceManager* mInstance = new ResourceManager();
static QHash<QString, AnimationImage*> mImageCache;
static QHash<AnimationImage*, int> mImageReferences;
static QHash<QString, int> mFontsPaths;
static QString mRelativePath = "";

ResourceManager::ResourceManager(QObject *parent) :
    QObject(parent)
{
}

void ResourceManager::addResource(Object * obj)
{
    if (obj) {
        if (! isValidName(obj->objectName()))
           obj->setObjectName(newName(obj));

        if (! obj->parent())
            obj->setParent(this);

        connect(obj, SIGNAL(dataChanged()), this, SIGNAL(resourceChanged()));
        mResources.append(obj);
        emit resourceAdded(obj);
    }
}

bool ResourceManager::isValidName(const QString& name)
{
    if (name.isEmpty() || name.isNull())
        return false;

    for(int i=0; i < mResources.size(); i++)
        if (mResources[i]->objectName() == name)
            return false;

    return true;
}

QString ResourceManager::newName(QString name)
{
    if (name.isEmpty() || name.isNull())
        name = "object";

    while(! isValidName(name)) {
        name = Utils::incrementLastNumber(name);
    }

    return name;
}

QString ResourceManager::newName(Object* obj)
{
   QString name = obj->objectName();
   if (name.isEmpty() || name.isNull())
        name = obj->type();

   return newName(name);
}

QList<Object*> ResourceManager::resources()
{
    return mResources;
}

Object * ResourceManager::typeToObject(const QString& type, QVariantMap& data, Object* parent)
{
    QString _type = type.toLower();

    if (_type == "object")
        return new Object(data, parent);
    else if (_type == "image")
        return new Image(data, parent);
    else if (_type == "character")
        return new Character(data, parent);
    else if (_type == "dialoguebox")
        return new DialogueBox(data, parent);
    else if (_type == "textbox")
        return new TextBox(data, parent);
    else if (_type == "button")
        return new Button(data, parent);
    else if (_type == "objectgroup")
        return new ObjectGroup(data, parent);
    else if (_type == "menu")
        return new Menu(data, parent);
    return 0;

}

Object* ResourceManager::createResource(QVariantMap data, bool appendToResources, Object* parent)
{
    Object* _resource = 0;
    QString type("");

    fillWithResourceData(data);

    if (data.contains("resource") && data.value("resource").type() == QVariant::String)
        _resource = resource(data.value("resource").toString());

    if (data.contains("type") && data.value("type").type() == QVariant::String)
        type = data.value("type").toString();

    Object *object = typeToObject(type, data, parent);
    if (object && _resource)
        object->setResource(_resource);

    if (appendToResources)
        addResource(object);
    return object;
}

void ResourceManager::removeResource(Object *object, bool del)
{
    if (mResources.contains(object)) {
        mResources.removeOne(object);
        object->disconnect(this);

        if (del && object)
            object->deleteLater();
    }
}

void ResourceManager::fillWithResourceData(QVariantMap& data)
{
    if (! data.contains("resource") || data.value("resource").type() != QVariant::String)
        return;

    QString name = data.value("resource").toString();

    Object* resource = ResourceManager::resource(name);
    if (! resource)
        return;

    QVariantMap resourceData = resource->toJsonObject();
    QStringList keys = resourceData.keys();
    foreach(const QString& key, keys)
        if (! data.contains(key))
            data.insert(key, resourceData.value(key));
}

Object *ResourceManager::resource(const QString & name)
{
    for(int i=0; i < mResources.size(); i++) {
        if (mResources[i]->objectName() == name)
            return mResources[i];
    }

    return 0;
}

Object *ResourceManager::resource(int index)
{
    if (index >= mResources.size() || index < 0)
        return 0;

    return mResources.at(index);
}


bool ResourceManager::contains(const QString & name)
{
    if ( resource(name) )
        return true;
    return false;
}

void  ResourceManager::removeResources(bool del)
{
    for(int i=mResources.size()-1; i >= 0; i--) {
        emit resourceRemoved(mResources[i]);
        if (del)
            mResources[i]->deleteLater();
    }

    mResources.clear();

    QHashIterator <QString, AnimationImage*>it(mImageCache);
    AnimationImage* image = 0;
    while(it.hasNext()) {
        it.next();
        image = static_cast<AnimationImage*>(it.value());
        if (image)
            delete image;
    }

    mImageCache.clear();
    mImageReferences.clear();
}

void  ResourceManager::destroy()
{
    mInstance->removeResources(true);
    mInstance->deleteLater();
}

ResourceManager* ResourceManager::instance()
{
    return mInstance;
}

AnimationImage* ResourceManager::newImage(const QVariant& imageData)
{
    if (imageData.type() == QVariant::String) {
        return newImage(imageData.toString());
    }
    else if (imageData.type() == QVariant::Map) {
        QVariantMap imageMap = imageData.toMap();
        if (imageMap.contains("src"))
            return newImage(imageMap.value("src").toString());
    }

    return 0;
}

AnimationImage* ResourceManager::newImage(const QString& _path)
{
    QString path(_path);

    if (! mRelativePath.isEmpty()) {
         QDir dir(mRelativePath);
         if (dir.exists(path))
             path = dir.absoluteFilePath(path);
     }

    if (mImageCache.contains(path)) {
        incrementReference(mImageCache.value(path));
        return mImageCache.value(path);
    }

    if (! QFile::exists(path))
        return 0;

    AnimationImage* image = new AnimationImage(path);
    mImageCache.insert(path, image);
    mImageReferences.insert(image, 1);

    return image;
}

QString ResourceManager::imagePath(QPixmap* pixmap, QMovie* movie)
{
    if (! pixmap && ! movie)
        return "";

    QHashIterator<QString, AnimationImage*> it(mImageCache);

    while(it.hasNext()) {
        it.next();
        if (it.value()->contains(pixmap) || it.value()->contains(movie))
            return it.key();
    }

    return "";
}

QString ResourceManager::imagePath(AnimationImage* image)
{
    if (! image)
        return "";

    return imagePath(image->pixmap(), image->movie());
}

void ResourceManager::incrementReference(AnimationImage* image)
{
    int refs = mImageReferences.value(image, -1);
    if (refs == -1)
        return;
    refs++;
    mImageReferences.insert(image, refs);
}


void ResourceManager::decrementReference(QPixmap* _pixmap)
{
    QHashIterator<QString, AnimationImage*> it(mImageCache);
    AnimationImage* image = 0;
    while(it.hasNext()) {
        it.next();
        if (it.value()->pixmap() == _pixmap) {
            image = it.value();
            break;
        }
    }

    decrementReference(image);
}

void ResourceManager::decrementReference(AnimationImage* image)
{
    if(! image || ! mImageReferences.contains(image))
        return;

    int refs = mImageReferences.value(image);
    if (refs <= 1)
    {
        mImageReferences.remove(image);

        QString keyPath = ResourceManager::imagePath(image);
        if (! keyPath.isEmpty())
            mImageCache.remove(keyPath);

        delete image;
    }
    else {
        refs--;
        mImageReferences.insert(image, refs);
    }
}

QStringList ResourceManager::imagePaths()
{
    return mImageCache.keys();
}

void ResourceManager::setRelativePath(const QString & path)
{
    mRelativePath = path;
}

int ResourceManager::newFont(const QString& path)
{
    if (mFontsPaths.contains(path))
        return mFontsPaths.value(path, -1);

    int id = QFontDatabase::addApplicationFont(path);
    mFontsPaths.insert(path, id);
    return id;
}

void ResourceManager::exportResources(const QDir& dir)
{
    foreach(const QString& path, mImageCache.keys()) {
        AnimationImage* image = mImageCache[path];
        if (image)
            image->save(dir);
    }

    QHashIterator<QString, int> it(mFontsPaths);
    QFile file(dir.absoluteFilePath("fontfaces.css"));
    if (! file.open(QFile::WriteOnly | QFile::Text))
        return;

    while(it.hasNext()) {
        it.next();
        QFileInfo info(it.key());
        //copy font file
        QFile::copy(it.key(), dir.absoluteFilePath(info.fileName()));
        //write css for font
        file.write(Utils::fontFace(info.fileName(), info.baseName()).toAscii());
        file.write("\n");
    }

    file.close();
}

QStringList ResourceManager::customFonts()
{
    QStringList fonts;
    QHashIterator<QString, int> it(mFontsPaths);
    while(it.hasNext()) {
        it.next();
        QFileInfo info(it.key());
        fonts.append(info.fileName());
    }

    return fonts;
}
