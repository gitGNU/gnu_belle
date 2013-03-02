/* Copyright (C) 2012, 2013 Carlos Pais 
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

#include "clipboard.h"

#include <QDebug>

Clipboard::Clipboard(QObject *parent) :
    QObject(parent)
{
}

bool Clipboard::hasObjects()
{
    return ! mObjects.isEmpty();
}

bool Clipboard::hasScenes()
{
    return ! mScenes.isEmpty();
}

QList<Object*> Clipboard::objects()
{
    return mObjects;
}

QList<Scene*> Clipboard::scenes()
{
    return mScenes;
}

Clipboard::Operation Clipboard::operation()
{
    return mOperation;
}

void Clipboard::add(QObject * obj, Clipboard::Operation op)
{
    add(QList<QObject*>()<< obj, op);
}

void Clipboard::add(const QList<QObject*>& objects, Clipboard::Operation op)
{
    this->clear();
    mOperation = op;

    foreach(QObject* obj, objects) {
        connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(onObjectDestroyed(QObject*)));
        if (qobject_cast<Object*>(obj))
            mObjects.append(qobject_cast<Object*>(obj));
        else if (qobject_cast<Scene*>(obj))
            mScenes.append(qobject_cast<Scene*>(obj));
    }
}

bool Clipboard::isEmpty()
{
    return mObjects.isEmpty();
}

void Clipboard::clear()
{
    foreach(QObject*obj, mObjects)
        obj->disconnect(this);

    foreach(QObject*obj, mScenes)
        obj->disconnect(this);

    mObjects.clear();
    mScenes.clear();
}

void Clipboard::onObjectDestroyed(QObject* object)
{
    Object* obj = qobject_cast<Object*>(object);
    if (obj){
        if (mObjects.contains(obj))
            mObjects.removeOne(obj);
    }
    else {
        Scene* scene = qobject_cast<Scene*>(object);
        if (scene && mScenes.contains(scene))
            mScenes.removeOne(scene);
    }
}
