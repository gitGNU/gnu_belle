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

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QObject>
#include <QHash>

/*namespace Clipboard {
    enum OperationType {
        Copy,
        Cut
    };
}*/

#include "object.h"
#include "scene.h"

class Object;
class Scene;

class Clipboard : public QObject
{
    Q_OBJECT

public:
    enum Operation {
        Copy,
        Cut
    };

private:
    QList<Object*> mObjects;
    QList<Scene*> mScenes;
    Clipboard::Operation mOperation;

public:
    explicit Clipboard(QObject *parent = 0);
    bool hasObjects();
    bool hasScenes();
    QList<Object*> objects();
    QList<Scene*> scenes();
    Clipboard::Operation operation();
    void add(QObject*, Clipboard::Operation);
    void add(const QList<QObject*>&, Clipboard::Operation);
    void clear();
    bool isEmpty();
    
signals:
    
private slots:
    void onObjectDestroyed(QObject*);
    
};

#endif // CLIPBOARD_H
