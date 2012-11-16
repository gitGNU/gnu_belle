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

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <QObject>
#include <QList>

#include "objects/object.h"

class Object;

class ResourceManager : public QObject
{
    Q_OBJECT

public:
    explicit ResourceManager(QObject *parent = 0);
    void addResource(Object*);
    Object* createResource(QVariantMap, bool appendToResources=true, Object* parent=0);
    bool contains(const QString&);
    void removeResource(Object*, bool del=false);

    static Object* resource(const QString&);
    static Object* resource(int);
    static void fillWithResourceData(QVariantMap&);

    Object* typeToObject(const QString&, QVariantMap& extraData, Object* parent=0);

    QString toJavascript();
    void writeToGameFile();
    bool isValidName(const QString&);
    QString newName(QString);
    QString newName(Object*);
    static void setRelativePath(const QString&);
    static ResourceManager* instance();
    static QList<Object*> resources();

    static QPixmap* newImage(const QString&);
    static QString imagePath(QPixmap*);
    static QStringList imagePaths();

    static void incrementReference(QPixmap* pixmap);
    static void decrementReference(QPixmap* pixmap);

    static void destroy();
    void removeResources(bool del);

signals:
    void resourceAdded(Object*);
    void resourceRemoved(Object*);
    void resourceRemoved(int);

public slots:


};

#endif // RESOURCE_MANAGER_H
