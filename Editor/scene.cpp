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

#include "scene.h"

#include <QDebug>
#include <QFile>

#include "scene_manager.h"
#include "textbox.h"
#include "utils.h"

static QSize mSize;
static QPoint mPoint;
static SceneEditorWidget* mEditorWidget = 0;

Scene::Scene(QObject *parent, const QString& name):
    QObject(parent)
{
    init(name);
}

Scene::Scene(const QVariantMap& data, QObject *parent):
    QObject(parent)
{
    init("");

    if (data.contains("name") && data.value("name").type() == QVariant::String)
        setObjectName(SceneManager::validSceneName(data.value("name").toString()));

    if (data.contains("background") && data.value("background").type() == QVariant::String)
        setBackground(data.value("background").toString());

    if (data.contains("objects") && data.value("objects").type() == QVariant::List) {
        QVariantList objects = data.value("objects").toList();
        for(int i=0; i < objects.size(); i++) {
            if (objects[i].type() != QVariant::Map)
                continue;
            Object* obj = ResourceManager::instance()->createResource(objects[i].toMap(), false);
            if (obj)
                appendObject(obj);
        }
    }

    if (data.contains("actions") && data.value("actions").type() == QVariant::List) {
        QVariantList actions = data.value("actions").toList();

        for(int i=0; i < actions.size(); i++) {
            if (actions[i].type() != QVariant::Map)
                continue;

            Action *action = ActionInfoManager::typeToAction(actions[i], this);
            if (action)
                appendAction(action);
        }
    }
}


Scene::~Scene()
{
    ResourceManager::decrementReference(mBackgroundImage);
    if (mScenePixmap)
        delete mScenePixmap;

    mObjects.clear();
    mTemporaryObjects.clear();
}

void Scene::init(const QString& name)
{
    if ( parent() )
        connect(parent(), SIGNAL(resized(const QResizeEvent&)), this, SLOT(onResizeEvent(const QResizeEvent&)));
    mSelectedObject = 0;
    mHighlightedObject = 0;
    mBackgroundImage = 0;
    mScenePixmap = new QPixmap(Scene::width(), Scene::height());
    mScenePixmap->fill(Qt::gray);

    this->setObjectName(SceneManager::validSceneName(name));
}

void Scene::setEditorWidget(SceneEditorWidget* editor)
{
    mEditorWidget = editor;
}

SceneEditorWidget* Scene::editorWidget()
{
    return mEditorWidget;
}

QList<Object*> Scene::objects()
{
    return mObjects;
}

QList<Object*> Scene::temporaryObjects()
{
    return mTemporaryObjects;
}

Object* Scene::objectAt (qreal x, qreal y)
{
    for(int i=mTemporaryObjects.size()-1; i >= 0; --i)
        if (mTemporaryObjects[i]->contains(x, y))
            return mTemporaryObjects[i];

    for(int i=mObjects.size()-1; i >= 0; --i) 
        if (mObjects[i]->contains(x, y))
            return mObjects[i];

    return 0;
}

Object* Scene::object(const QString & name)
{
    for(int i=0; i < mObjects.size(); i++)
        if (mObjects[i]->objectName() == name)
            return mObjects[i];
    return 0;
}

void Scene::addCopyOfObject(Object* object, bool select)
{
    if (! object)
        return;

    QVariantMap data(object->toJsonObject());
    Object * obj = ResourceManager::instance()->createResource(data, false);
    appendObject(obj, select);
    obj->setResource(object);
}


void Scene::appendObject(Object* object, bool select, bool temporarily)
{
    if (! object)
        return;

    connect(object, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
    if (object->parent() != this)
        object->setParent(this);

    //test if it's a valid name before adding the object to scene.
    object->setObjectName(newObjectName(object->objectName()));

    if (temporarily)
        mTemporaryObjects.append(object);
    else
        mObjects.append(object);

    if (select)
        selectObject(object);
}

int Scene::width()
{
    return mSize.width();
}

int Scene::height()
{
    return mSize.height();
}

QSize Scene::size()
{
    return mSize;
}

void Scene::setWidth(int w)
{
    mSize.setWidth(w);
}

void Scene::setHeight(int h)
{
    mSize.setHeight(h);
}

void Scene::setSize(const QSize& size)
{
    mSize = size;
}

void Scene::onResizeEvent(const QResizeEvent& event)
{
    emit resized(event);
}

void Scene::selectObjectAt(int x, int y) 
{
    Object *prevSelectedObject = mSelectedObject;
    mSelectedObject = objectAt(x, y);

    //usually returns the self object, but if it is an ObjectGroup it can return other objects inside it
    if (mSelectedObject)
        mSelectedObject = mSelectedObject->objectAt(x, y);

    //if (mSelectedObject != prevSelectedObject)
    selectObject(mSelectedObject);
}

void Scene::moveSelectedObject(int x, int y)
{
    if (mSelectedObject)
        mSelectedObject->dragMove(x, y);
}

void Scene::stopMoveSelectedObject()
{
    if (mSelectedObject) {
        mSelectedObject->stopMove();
        emit dataChanged();
    }
}

Object * Scene::selectedObject()
{
    return mSelectedObject;
}

void Scene::moveSelectedObjectUp()
{
    if (! mSelectedObject)
        return;

    int index = mObjects.indexOf(mSelectedObject);
    if ( index >= mObjects.size()-1)
        return;

    mObjects.removeAt(index);
    mObjects.insert(index+1, mSelectedObject);
    emit dataChanged();
}

void Scene::moveSelectedObjectDown()
{
    if (! mSelectedObject)
        return;

    int index = mObjects.indexOf(mSelectedObject);
    if ( index == 0 )
        return;

    mObjects.removeAt(index);
    mObjects.insert(index-1, mSelectedObject);
    emit dataChanged();
}

void Scene::fillWidth()
{
    if (! mSelectedObject)
        return;

    mSelectedObject->setWidth(Scene::width());
    emit dataChanged();
}

void Scene::setBackground(const QString & path)
{
    ResourceManager::decrementReference(mBackgroundImage);
    mBackgroundImage = ResourceManager::newImage(path);

    if (mBackgroundImage)
        *mBackgroundImage = mBackgroundImage->scaled(Scene::size());

    emit dataChanged();
}

void Scene::setBackgroundImage(QPixmap* image)
{
    mBackgroundImage = image;
    emit dataChanged();
}

QPixmap* Scene::backgroundImage()
{
    return mBackgroundImage;
}

void Scene::setBackgroundColor(const QColor& color)
{
    mBackgroundColor = color;
    emit dataChanged();
}

QColor Scene::backgroundColor()
{
    return mBackgroundColor;
}

QString Scene::backgroundPath()
{
    return ResourceManager::imagePath(mBackgroundImage);
}

void Scene::clearBackground()
{
    if ( mBackgroundImage ) {
        ResourceManager::decrementReference(mBackgroundImage);
        mBackgroundImage = 0;
        emit dataChanged();
    }
}

int Scene::countTextBoxes()
{
    int count = 0;
    TextBox* text = 0;

    for(int i=0; i < mObjects.size(); i++) {
        text = qobject_cast<TextBox*>(mObjects[i]);
        if (text)
            count++;
    }

    return count;
}


QPoint Scene::point()
{
    return mPoint;
}

void Scene::setPoint(const QPoint & point)
{
    mPoint = point;
}

void Scene::removeObject(Object* object, bool del, bool temporary)
{
    if (! object)
        return;

    bool removed = false;
    if (temporary && mTemporaryObjects.contains(object))
        removed = mTemporaryObjects.removeOne(object);
    else if (mObjects.contains(object))
        removed = mObjects.removeOne(object);

    if (removed) {
        if (selectedObject() == object)
            selectObject(0);
        object->disconnect(this);
        if (del)
            object->deleteLater();
        emit dataChanged();
    }
}

void Scene::removeSelectedObject(bool del)
{
    removeObject(mSelectedObject, del);
    mSelectedObject = 0;
}

void Scene::selectObject(Object* obj)
{

    mHighlightedObject = 0;
    mSelectedObject = obj;

   emit dataChanged();
   emit selectionChanged(mSelectedObject);
}

void Scene::highlightObject(Object* obj)
{
    if (mObjects.contains(obj) || ! obj) {
        mHighlightedObject = obj;
        emit dataChanged();
    }
}

Object* Scene::highlightedObject()
{
    return mHighlightedObject;
}

void Scene::deleteActionAt(int index)
{
    if (index >= 0 && index < mActions.size()) {
        Action* action = mActions.takeAt(index);
        action->deleteLater();
    }
}

void Scene::deleteAction(Action* action)
{
    if (action)
        deleteActionAt(mActions.indexOf(action));
}


QList<Action*> Scene::actions() const
{
    return mActions;
}

void Scene::setActions(const QList<Action *> & actions)
{
    mActions = actions;
}

void Scene::appendAction(Action * action)
{
    if (! action)
        return;

    action->setParent(this);
    mActions.append(action);
}

QIcon Scene::icon()
{
    if (mScenePixmap)
        return QIcon(*mScenePixmap);
    else
        return QIcon();
}

QPixmap* Scene::pixmap()
{
    return mScenePixmap;
}

QVariantMap Scene::toJsonObject()
{
    QVariantMap scene;
    QFileInfo imageInfo(ResourceManager::imagePath(mBackgroundImage));

    scene.insert("name", objectName());
    scene.insert("type", "Scene");
    scene.insert("background", imageInfo.fileName());

    QVariantList objects;
    for(int i=0; i < mObjects.size(); i++) {
        objects.append(mObjects[i]->toJsonObject());
    }
    scene.insert("objects", objects);

    QVariantList actions;
    for(int i=0; i < mActions.size(); i++) {
        actions.append(mActions[i]->toJsonObject());
    }

    scene.insert("actions", actions);

    return scene;
}

Scene* Scene::copy()
{
    Scene* scene = new Scene(this->toJsonObject(), this->parent());
    scene->setObjectName(SceneManager::validSceneName(objectName()+"_1"));
    return scene;
}

void Scene::insertAction(int row, Action* action)
{
    mActions.insert(row, action);
}

bool Scene::isValidObjectName(const QString& name)
{
    if (name.isEmpty() || name.isNull())
        return false;

    for(int i=0; i < mObjects.size(); i++)
        if (mObjects[i]->objectName() == name)
            return false;

    return true;
}

QString Scene::newObjectName(QString name)
{
    if (name.isEmpty() || name.isNull())
        name = "object";

    while(! isValidObjectName(name)) {
        name = Utils::incrementLastDigit(name);
    }

    return name;
}



