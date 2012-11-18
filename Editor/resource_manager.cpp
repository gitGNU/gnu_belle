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

#include "utils.h"
#include "image.h"
#include "character.h"
#include "dialoguebox.h"
#include "textbox.h"
#include "button.h"
#include "menu.h"

static QList<Object*> mResources;
static ResourceManager* mInstance = new ResourceManager();
static QHash<QString, QPixmap*> mPixmapCache;
static QHash<QPixmap*, int> mPixmapReferences;
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
        name = Utils::incrementLastDigit(name);
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

    QHashIterator <QString, QPixmap*>it(mPixmapCache);
    QPixmap* pixmap = 0;
    while(it.hasNext()) {
        it.next();
        pixmap = static_cast<QPixmap*>(it.value());
        if (pixmap)
            delete pixmap;
    }

    mPixmapCache.clear();
    mPixmapReferences.clear();
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

QPixmap* ResourceManager::newImage(const QString& _path)
{
    QString path(_path);

     if (! mRelativePath.isEmpty()) {
         QDir dir(mRelativePath);
         if (dir.exists(path))
             path = dir.absoluteFilePath(path);
     }

    if (mPixmapCache.contains(path)) {
        incrementReference(mPixmapCache.value(path));
        return mPixmapCache.value(path);
    }

    if (! QFile::exists(path))
        return 0;

    QPixmap* pixmap = new QPixmap(path);
    mPixmapCache.insert(path, pixmap);
    mPixmapReferences.insert(pixmap, 1);
    return pixmap;
}

QString ResourceManager::imagePath(QPixmap* pixmap)
{
    if (! pixmap)
        return "";

    QHashIterator<QString, QPixmap*> it(mPixmapCache);

    while(it.hasNext()) {
        it.next();
        if (it.value() == pixmap)
            return it.key();
    }

    return "";
}

void ResourceManager::incrementReference(QPixmap* pixmap)
{
    if (! mPixmapReferences.contains(pixmap))
        return;

    int refs = mPixmapReferences.value(pixmap);
    refs++;
    mPixmapReferences.insert(pixmap, refs);
}

void ResourceManager::decrementReference(QPixmap* pixmap)
{
    if(! pixmap || ! mPixmapReferences.contains(pixmap))
        return;

    int refs = mPixmapReferences.value(pixmap);
    if (refs <= 1)
    {
        mPixmapReferences.remove(pixmap);

        QString keyPath = ResourceManager::imagePath(pixmap);
        if (! keyPath.isEmpty())
            mPixmapCache.remove(keyPath);

        delete pixmap;
    }
    else {
        refs--;
        mPixmapReferences.insert(pixmap, refs);
    }
}

QStringList ResourceManager::imagePaths()
{
    return mPixmapCache.keys();
}

void ResourceManager::setRelativePath(const QString & path)
{
    mRelativePath = path;
}
