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

#include "objects/object.h"

#include <QDebug>
#include <QFile>
#include <QColor>
#include <QFont>
#include <math.h>

#include "scene.h"
#include "utils.h"
#include "scene_manager.h"

static ObjectEditorWidget * mEditorWidget = 0;
static QFont mDefaultFont;

Object::Object(QObject* parent, const QString& name):
    QObject(parent)
{
    init(name);
    updateResizeRects();
}

Object::Object(const QVariantMap& data, QObject* parent):
    QObject(parent)
{
    init("Object");
    setProperties(data);
    updateResizeRects();
}

void Object::init(const QString& name)
{
    mOpacity = 255;
    mBackground.setColor(QColor(255, 255, 255, 0));
    mCornerRadius = 0;
    mType = "Object";
    mOriginalResizePointIndex = -1;
    mVisible = true;
    mEditableName = true;
    mPercentWidth = 0;
    mPercentHeight = 0;
    mResource = 0;
    mBorderWidth = 0;
    mBorderColor = QColor();
    mSelectedObject = 0;
    mKeepAspectRatio = false;
    mAspectRatio = 1;
    mScaledBackgroundImage = 0;

    //check if name is valid
    if (objectName().isEmpty()) {
        if (parent() == ResourceManager::instance())
            setObjectName(ResourceManager::instance()->newName(name));
        else {
            Scene* scene = qobject_cast<Scene*>(parent());
            if (scene)
                scene->newObjectName(name);
        }
    }
}

Object::~Object()
{
    //if (mBackgroundImage)
    //    delete mBackgroundImage;
}

QVariantMap Object::fillWithResourceData(QVariantMap data)
{
    QString name = data.value("resource").toString();

    Object* resource = ResourceManager::instance()->resource(name);
    if (! resource)
        return data;

    QVariantMap resourceData = resource->toJsonObject();
    QStringList keys = resourceData.keys();
    foreach(const QString& key, keys)
        if (! data.contains(key))
            data.insert(key, resourceData.value(key));

    return data;
}

Scene * Object::scene()
{
    Object* object = this;

    while(object->parent()) {
        Scene* scene = qobject_cast<Scene*>(object->parent());
        if (scene)
            return scene;
        object = qobject_cast<Object*>(object->parent());
        if (! object)
            break;
    }

    return 0;
}

bool Object::isValidName(const QString& name)
{
    if (name.isEmpty())
        return false;

    //check if parent is the scene
    Scene* scene = this->scene();
    if (scene)
        return scene->isValidObjectName(name);

    //check if parent is the resource manager
    ResourceManager* resourceManager = qobject_cast<ResourceManager*>(parent());
    if (resourceManager)
        return resourceManager->isValidName(name);

    return true;
}

bool Object::contains(qreal x, qreal y)
{
    if (mSceneRect.contains(x, y) || containsResizeRectAt(x, y))
        return true;
        
    return false;
}

QRect Object::sceneRect() const
{
    return mSceneRect;
}

void Object::onResizeEvent(QResizeEvent* event)
{
    return;

    int width = Scene::width() - mSceneRect.width();
    int height = Scene::height() - mSceneRect.height();
    
    mSceneRect.setWidth(mSceneRect.width() + width);
    mSceneRect.setHeight(mSceneRect.height() + height);
}

void Object::move(int x, int y)
{
    mSceneRect.moveTo(x, y);
    updateResizeRects();
    QVariantMap data;
    data.insert("x", x);
    data.insert("y", y);
    emit dataChanged(data);
    //emit positionChanged(mSceneRect.x(), mSceneRect.y());
}

void Object::dragMove(int x, int y)
{
    if (mPreviousSceneRect.isEmpty() || mPreviousSceneRect.isNull()) {
        mPreviousSceneRect = mSceneRect;
        mXDif = x - mSceneRect.x();
        mYDif = y - mSceneRect.y();
    }
    
    //since x and y represent mouse coordinates and we move the rectangle by moving its top left corner,
    //we need to know the initial distance from the point where the mouse was clicked to the top left corner.
    move(x - mXDif, y - mYDif);
}

void Object::stopMove()
{
    mPreviousSceneRects.append(mPreviousSceneRect);
    mPreviousSceneRect = QRect();
}

QList<QRect> Object::previousSceneRects() const
{
    return mPreviousSceneRects;
}

ObjectEditorWidget* Object::objectEditorWidget()
{
    return mEditorWidget;
}

void Object::setObjectEditorWidget(ObjectEditorWidget* widget)
{
    mEditorWidget = widget;
}

ObjectEditorWidget* Object::editorWidget()
{
    return mEditorWidget;
}

QColor Object::backgroundColor() const
{
    return mBackground.color();
}

void Object::setBackgroundColor(const QColor & color)
{
    notify("backgroundColor", Utils::colorToList(color), Utils::colorToList(mBackground.color()));
    int alpha = mBackground.opacity();
    mBackground.setColor(color);
    mBackground.setOpacity(alpha);
}

int Object::backgroundOpacity() const
{
    return mBackground.opacity();
}

void Object::setBackgroundOpacity(int alpha)
{
    int prevAlpha = mBackground.opacity();
    mBackground.setOpacity(alpha);
    notify("backgroundOpacity", alpha, prevAlpha);
}

int Object::opacity() const
{
    return mOpacity;
}

qreal Object::opacityF() const
{
    return (mOpacity * 1.0) / 255;
}

void Object::setOpacity(int opacity)
{
    if (opacity > 255)
        opacity = 255;
    else if (opacity < 0)
        opacity = 0;

    if (opacity != mOpacity){
        int prevOpacity = mOpacity;
        mOpacity = opacity;
        notify("opacity", mOpacity, prevOpacity);
    }
}


void Object::setWidth(int w, bool percent)
{
    int width = mSceneRect.width();

    if (percent) {
        mPercentWidth = w;
        Object * obj = qobject_cast<Object*>(parent());
        if (obj)
            width = obj->width() * w / 100;
        else if (scene())
            width = scene()->width() * w / 100;

        mSceneRect.setWidth(width);
    }
    else {
        mSceneRect.setWidth(w);
        mPercentWidth = 0;
    }

    updateResizeRects();

    notify("width", mSceneRect.width(), width);
}

void Object::setHeight(int h, bool percent)
{
    int height = mSceneRect.height();

    if (percent) {
        mPercentHeight = h;
        Object * obj = qobject_cast<Object*>(parent());
        if (obj)
            height = obj->height() * h / 100;
        else if(scene())
            height = scene()->height() * h / 100;

        mSceneRect.setHeight(height);
    }
    else {
        mPercentHeight = 0;
        mSceneRect.setHeight(h);
    }

    updateResizeRects();

    notify("height", mSceneRect.height(), height);
}

void Object::setY(int y)
{
    mSceneRect.moveTo(mSceneRect.x(), y + mPadding.top());
    updateResizeRects();
    emit dataChanged();
    //emit positionChanged(x(), y);
}

void Object::setX(int x)
{
    mSceneRect.moveTo(x + padding("left"), mSceneRect.y());
    updateResizeRects();
    QVariantMap data;
    data.insert("x", x);
    emit dataChanged(data);
    //emit positionChanged(x, y());
}

int Object::x() const
{
    return mSceneRect.x();
}

int Object::y() const
{
    return mSceneRect.y();
}

int Object::parentWidth() const
{
    if (! parent())
        return 0;
    Object* p = qobject_cast<Object*>(parent());

    if (p)
        return p->width();
    else
        return Scene::width();
}

int Object::width() const
{
    int width = contentWidth();
    width += mPadding.left() + mPadding.right();

    return width;
}

int Object::contentWidth() const
{
    int width = 0;
    if (mPercentWidth)
        width = mPercentWidth / 100 * parentWidth();
    else
        width = mSceneRect.width();

    return width;
}

int Object::parentHeight() const
{
    if (! parent())
        return 0;
    Object* p = qobject_cast<Object*>(parent());
    if (p)
        return p->height();
    else
        return Scene::height();
}

int Object::contentHeight() const
{
    int height = 0;

    if (mPercentHeight)
        height = mPercentHeight / 100 * parentHeight();
    else
        height = mSceneRect.height();

    return height;
}

int Object::height() const
{
    int height = contentHeight();
    height += mPadding.top() + mPadding.bottom();

    return height;
}

void Object::update()
{
    emit dataChanged();
}

bool Object::isRounded() const
{
    if (mCornerRadius)
        return true;
    return false;
}

void Object::setCornerRadius(int r)
{
    if (mCornerRadius != r) {
        mCornerRadius = r;
        QVariantMap data;
        data.insert("cornerRadius", r);
        emit dataChanged(data);
    }
}

int Object::cornerRadius()
{
    return mCornerRadius;
}

QString Object::type()
{
    return mType;
}

void Object::setType(const QString & type)
{
    mType = type;
}

void Object::paint(QPainter & painter)
{
    painter.setOpacity(opacityF());

    if (! mVisible || ! mOpacity) {
        return;
    }

    painter.setOpacity(opacityF());
    QRect rect(mSceneRect.x(), mSceneRect.y(), contentWidth(), contentHeight());

    mBackground.paint(painter, rect, mCornerRadius, opacityF());

    if (mBorderColor.isValid()) {
        painter.setPen(QPen(mBorderColor, mBorderWidth));
        if (mCornerRadius)
            painter.drawRoundedRect(rect, mCornerRadius, mCornerRadius);
        else
            painter.drawRect(rect);
    }
}

Object* Object::copy()
{
    Object * obj = ResourceManager::instance()->createResource(toJsonObject(), false);
    return obj;
}

void Object::appendEventAction(Interaction::InputEvent event, Action * action)
{
    insertEventAction(event, mEventToActions.value(event, QList<Action*>()).size(), action);
}

void Object::insertEventAction(Interaction::InputEvent event, int index, Action * action)
{
    if (action) {
        QList<Action*> actions;
        if (mEventToActions.contains(event))
            actions = mEventToActions.value(event);

        if (index >= 0 && index <= actions.size())
            actions.insert(index, action);

        mEventToActions.insert(event, actions);
    }
}

void Object::removeEventActionAt(Interaction::InputEvent event, int index, bool del)
{
   QList<Action*> actions = mEventToActions.value(event);
   if (index >= 0 && index < actions.size()) {
       Action* action = actions.takeAt(index);
       if (del)
        action->deleteLater();
       mEventToActions.insert(event, actions);
   }
}

void Object::setBackgroundImage(const QString & path)
{
    ImageFile* image = mBackground.image();

    if (image && image->path() == path)
        return;

    if (image && image->isAnimated()) {
        image->movie()->stop();
        image->movie()->disconnect(this);
    }

    ResourceManager::decrementReference(image);

    image = ResourceManager::newImage(path);
    if (image && image->isAnimated()) {
        connect(image->movie(), SIGNAL(frameChanged(int)), this, SIGNAL(dataChanged()));
        image->movie()->start();
    }

    QString prevPath("");
    if (mBackground.image())
        mBackground.image()->path();
    mBackground.setImage(image);
    notify("backgroundImage", path, prevPath);
}

ImageFile* Object::backgroundImage() const
{
    return mBackground.image();
}

QList<Action*> Object::actionsForEvent(Interaction::InputEvent event)
{
    return mEventToActions.value(event);
}

QVariantMap Object::toJsonObject()
{
    QVariantMap object;
    object.insert("name", objectName());
    object.insert("type", mType);
    object.insert("x", mSceneRect.x());
    object.insert("y", mSceneRect.y());
    object.insert("opacity", mOpacity);

    if (mPercentWidth) object.insert("width", QString("%1\%").arg(mPercentWidth));
    else object.insert("width", contentWidth());
    if (mPercentHeight) object.insert("height", QString("%1\%").arg(mPercentHeight));
    else object.insert("height", contentHeight());
    QVariantList color;
    QColor bgColor = mBackground.color();
    color << bgColor.red() << bgColor.green() << bgColor.blue()
             << bgColor.alpha();
    object.insert("backgroundColor", color);

    if (mCornerRadius) {
        object.insert("cornerRadius", mCornerRadius);
    }

    if (mBackground.image()) {
        object.insert("backgroundImage", mBackground.image()->name());
    }

    QVariantList jsonActions;
    QList<Action*> actions = mEventToActions.value(Interaction::MousePress);
    if (actions.size() > 0) {
        for(int i=0; i < actions.size(); i++)
            jsonActions.append(actions[i]->toJsonObject());
        object.insert("onMousePress", jsonActions);
    }

    jsonActions.clear();
    actions = mEventToActions.value(Interaction::MouseRelease);
    if (actions.size() > 0) {
        for(int i=0; i < actions.size(); i++)
            jsonActions.append(actions[i]->toJsonObject());
        object.insert("onMouseRelease", jsonActions);
    }

    jsonActions.clear();
    actions = mEventToActions.value(Interaction::MouseMove);
    if (actions.size() > 0) {
        for(int i=0; i < actions.size(); i++)
            jsonActions.append(actions[i]->toJsonObject());
        object.insert("onMouseMove", jsonActions);
    }


    object.insert("visible", mVisible);
    if (! mPadding.isEmpty())
        object.insert("padding", mPadding.toJsonObject());
    if (mBorderWidth > 0)
        object.insert("borderWidth", mBorderWidth);
    if (mBorderColor.isValid())
        object.insert("borderColor", Utils::colorToList(mBorderColor));

    //remove attributes that are the same in the resource
    filterResourceData(object);

    return object;
}

void Object::filterResourceData(QVariantMap& objectData)
{
    if (! mResource)
        return;

    objectData.insert("resource", mResource->objectName());
    QVariantMap resourceData = mResource->toJsonObject();
    QStringList keys = objectData.keys();
    foreach(const QString& key, keys) {
        if (resourceData.contains(key) && resourceData.value(key) == objectData.value(key))
            objectData.remove(key);
    }

}

void Object::resize(int pointIndex, int x, int y)
{
    //QPoint point = rect.center();
    QPoint toPoint(x, y);
    mOriginalResizePointIndex = pointIndex;
    movePoint(pointIndex, toPoint);
    updateResizeRects();

    //if (i < 8)
      //  updateResizeRects();
    //if (i < 8)
    //    emit dataChanged();
}

void Object::resize(int x, int y)
{
    if (mOriginalResizePointIndex == -1)
        return;

    QPoint point (x, y);
    movePoint(mOriginalResizePointIndex, point);

    if (mKeepAspectRatio) {
        int w2 = width();
        int h2 = round(w2/mAspectRatio);
        fixPoint(mOriginalResizePointIndex, w2, h2);
    }

    updateResizeRects();
    QVariantMap data;
    data.insert("width", width());
    data.insert("height", height());
    emit dataChanged(data);
}

void Object::fixPoint(int pindex, int w, int h)
{
    QPoint point;
    int x, y;

    switch (pindex){
        case 0:
            point = mSceneRect.bottomRight();
            x = point.x() - w;
            y = point.y() - h;
            mSceneRect.setTopLeft(QPoint(x, y));
            break;
        case 2:
            point = mSceneRect.bottomLeft();
            y = point.y() - h;
            x = point.x() + w;
            mSceneRect.setTopRight(QPoint(x, y));
            break;

        case 4:
            point = mSceneRect.topLeft();
            y = point.y() + h;
            x = point.x() + w;
            mSceneRect.setBottomRight(QPoint(x, y));
            break;

        case 6:
            point = mSceneRect.topRight();
            y = point.y() + h;
            x = point.x() - w;
            mSceneRect.setBottomLeft(QPoint(x, y));
            break;
    }
}

void Object::movePoint(int which, QPoint& point)
{
    QPoint point2;

    switch(which)
    {
    case 0:
        point2 = mSceneRect.bottomRight();
        if (point.x() > point2.x())
            point.setX(point2.x());
        if (point.y() > point2.y())
            point.setY(point2.y());
        mSceneRect.setTopLeft(point);
        updateResizeRect(0, mSceneRect.topLeft());
        break;
    case 1:
         point2 = mSceneRect.bottomLeft();
         if (point.y() < point2.y())
            mSceneRect.setY(point.y());
         break;
    case 2:
        point2 = mSceneRect.bottomLeft();
        if (point.x() < point2.x())
            point.setX(point2.x());
        if (point.y() > point2.y())
            point.setY(point2.y());
        mSceneRect.setTopRight(point);
        updateResizeRect(2, mSceneRect.topRight());
        break;
    case 3:
        point2 = mSceneRect.bottomLeft();
        if (point.x() > point2.x())
            mSceneRect.setWidth(point.x()-mSceneRect.x());
        break;
    case 4:
        point2 = mSceneRect.topLeft();
        if (point.x() < point2.x())
            point.setX(point2.x());
        if (point.y() < point2.y())
            point.setY(point2.y());
        mSceneRect.setBottomRight(point);
        updateResizeRect(4, mSceneRect.bottomRight());
        break;
    case 5:
        point2 = mSceneRect.topRight();
        if (point.y() > point2.y())
            mSceneRect.setHeight(point.y()-mSceneRect.y());
        break;
    case 6:
        point2 = mSceneRect.topRight();
        if (point.x() > point2.x())
            point.setX(point2.x());
        if (point.y() < point2.y())
            point.setY(point2.y());
        mSceneRect.setBottomLeft(point);
        updateResizeRect(6, mSceneRect.bottomLeft());
        break;
    case 7:
        point2 = mSceneRect.topRight();
        if (point.x() < point2.x())
            mSceneRect.setX(point.x());
        break;
    }
}

QList<QRect> Object::resizeRects()
{
    return mResizeRects;
}

void Object::updateResizeRect(int i, const QPoint& point)
{
    int rectWidth = RESIZE_RECT_WIDTH;
    int rectWidthHalf = rectWidth/2;

    if (i >= 0 && i < mResizeRects.size()) {
        QRect rect = mResizeRects.at(i);
        rect = QRect(point.x()-rectWidthHalf, point.y()-rectWidthHalf, rectWidth, rectWidth);
        mResizeRects.replace(i, rect);
    }
}

void Object::updateResizeRects()
{
    int rectWidth = RESIZE_RECT_WIDTH;
    int rectWidthHalf = rectWidth/2;

    mResizeRects.clear();
    //top left
    QPoint point = mSceneRect.topLeft();
    QRect rect = QRect(point.x()-rectWidthHalf, point.y()-rectWidthHalf, rectWidth, rectWidth);
    mResizeRects.append(rect);

    //middle top
    point.setX(point.x() + mSceneRect.width()/2);
    rect = QRect(point.x()-rectWidthHalf, point.y()-rectWidthHalf, rectWidth, rectWidth);
    mResizeRects.append(rect);

    //top right
    point = mSceneRect.topRight();
    rect = QRect(point.x()-rectWidthHalf, point.y()-rectWidthHalf, rectWidth, rectWidth);
    mResizeRects.append(rect);

    //middle right
    point.setY(point.y()+mSceneRect.height()/2);
    rect = QRect(point.x()-rectWidthHalf, point.y()-rectWidthHalf, rectWidth, rectWidth);
    mResizeRects.append(rect);

    //bottom right
    point = mSceneRect.bottomRight();
    rect = QRect(point.x()-rectWidthHalf, point.y()-rectWidthHalf, rectWidth, rectWidth);
    mResizeRects.append(rect);

    //middle bottom
    point.setX(point.x() - mSceneRect.width()/2);
    rect = QRect(point.x()-rectWidthHalf, point.y()-rectWidthHalf, rectWidth, rectWidth);
    mResizeRects.append(rect);

    //bottom left
    point = mSceneRect.bottomLeft();
    rect = QRect(point.x()-rectWidthHalf, point.y()-rectWidthHalf, rectWidth, rectWidth);
    mResizeRects.append(rect);

    //middle left
    point.setY(point.y() - mSceneRect.height()/2);
    rect = QRect(point.x()-rectWidthHalf, point.y()-rectWidthHalf, rectWidth, rectWidth);
    mResizeRects.append(rect);
}


QRect Object::resizeRectAt(int x, int y)
{
    foreach(const QRect& rect, mResizeRects) {
        if (rect.contains(x, y))
            return rect;
    }

    return QRect();
}

bool Object::containsResizeRectAt(int x, int y)
{
    foreach(const QRect& rect, mResizeRects) {
        if (rect.contains(x, y))
            return true;
    }

    return false;
}

void Object::setHoveredResizeRect(int i)
{
    mOriginalResizePointIndex = i;
    if (i < 0 || i >= mResizeRects.size())
        mOriginalResizePointIndex = -1;
}

void Object::stopResizing()
{
    mOriginalResizePointIndex = -1;
}

bool Object::visible()
{
    return mVisible;
}

void Object::setVisible(bool visible)
{
    mVisible = visible;
    emit dataChanged();
}

Object* Object::objectAt(qreal x, qreal y)
{
    if (this->contains(x, y))
        return this;
    return 0;
}

void Object::selectObjectAt(qreal x, qreal y)
{
    mSelectedObject = objectAt(x, y);
}

Object* Object::selectedObject()
{
    return mSelectedObject;
}

void Object::setEditableName(bool editable)
{
    mEditableName = editable;
}

bool Object::editableName()
{
    return mEditableName;
}

Padding Object::padding()
{
    return mPadding;
}

int Object::padding(const QString & side)
{
    return mPadding.padding(side);
}

void Object::setPadding(const QString & side, int value){
    Padding padding = mPadding;
    padding.setPadding(side, value);
    setPadding(padding);
}

void Object::setPadding(const Padding& padding)
{
    /*if (mPadding.top() != padding.top() || mPadding.bottom() != padding.bottom())
        mSceneRect.setHeight(height());

    if (mPadding.left() != padding.left() || mPadding.right() != padding.right())
        mSceneRect.setWidth(width());*/

    /*if (mPadding.top() != padding.top())
        mBoundingRect.setTop(sceneRect().top()+padding.top());
    if (mPadding.left() != padding.left())
        mBoundingRect.setLeft(sceneRect().left()+padding.left());
    if (mPadding.bottom() != padding.bottom())
        mBoundingRect.setBottom(sceneRect().bottom()+padding.bottom());
    if (mPadding.right() != padding.right())
        mBoundingRect.setRight(sceneRect().right()+padding.right());*/

    mPadding = padding;
    emit dataChanged();
}

void Object::setResource(Object* resource)
{
    if (mResource)
        mResource->disconnect(this);

    mResource = resource;
    connect(mResource, SIGNAL(dataChanged(const QVariantMap&)), this, SLOT(onResourceChanged(const QVariantMap&)));
    connect(mResource, SIGNAL(destroyed()), this, SLOT(onResourceDestroyed()));
}

Object* Object::resource()
{
    return mResource;
}

void Object::onResourceDestroyed()
{
    mResource = 0;
}

bool Object::hasObjectAsParent()
{
    if (! this->parent())
        return false;

    if (qobject_cast<Object*>(this->parent()))
        return true;

    return false;
}

void Object::setProperties(const QVariantMap &data)
{

    if (data.contains("resource") && data.value("resource").type() == QVariant::String) {
        mResource = ResourceManager::instance()->resource(data.value("resource").toString());
    }

    if (data.contains("name") && data.value("name").type() == QVariant::String)
        setObjectName(data.value("name").toString());

    if (data.contains("opacity") && data.value("opacity").canConvert(QVariant::Int))
        setOpacity(data.value("opacity").toInt());

    if (data.contains("backgroundColor") && data.value("backgroundColor").type() == QVariant::List) {
        QColor color = Utils::listToColor(data.value("backgroundColor").toList());
        if (color.isValid())
            mBackground.setColor(color);
    }

    if (data.contains("backgroundOpacity") && data.value("backgroundOpacity").type() == QVariant::Int) {
        if (data.contains("previousValue") && data.value("previousValue").type() == QVariant::Int)
            if (data.value("previousValue").toInt() != mBackground.opacity())
                return;

        mBackground.setOpacity(data.value("backgroundOpacity").toInt());
    }

    if (data.contains("backgroundImage") && data.value("backgroundImage").type() == QVariant::String) {
        mBackground.setImage(data.value("backgroundImage").toString());
    }

    if (data.contains("x") && data.value("x").canConvert(QVariant::Int)) {
        mSceneRect.setX(data.value("x").toInt());
    }

    if (data.contains("y") && data.value("y").canConvert(QVariant::Int))
        mSceneRect.setY(data.value("y").toInt());

    if (data.contains("width")) {
        if (data.value("width").type() == QVariant::String) {
            QString width = data.value("width").toString();
            setWidth(Utils::parseSize(width), Utils::isPercentSize(width));
        }
        else if (data.value("width").canConvert(QVariant::Int))
            mSceneRect.setWidth(data.value("width").toInt());
    }

    if (data.contains("height") && data.value("height").canConvert(QVariant::Int)) {
        if (data.value("height").type() == QVariant::String) {
            QString height = data.value("height").toString();
            setHeight(Utils::parseSize(height), Utils::isPercentSize(height));
        }
        else if (data.value("height").canConvert(QVariant::Int))
            mSceneRect.setHeight(data.value("height").toInt());
    }

    if (data.contains("cornerRadius") && data.value("cornerRadius").canConvert(QVariant::Int)) {
        mCornerRadius = data.value("cornerRadius").toInt();
    }

    if (data.contains("visible") && data.value("visible").type() == QVariant::Bool)
        setVisible(data.value("visible").toBool());

    QVariantList eventActions;
    QVariantMap actionData;
    QList<Action*> actions;
    Action* action;
    if (data.contains("onMouseMove") && data.value("onMouseMove").type() == QVariant::List) {
        eventActions = data.value("onMouseMove").toList();
        for(int i=0; i < eventActions.size(); i++) {
            actionData = eventActions[i].toMap();
            action = ActionInfoManager::typeToAction(actionData, this);
            if (action)
                actions.append(action);
        }
        mEventToActions.insert(Interaction::MouseMove, actions);
    }

    actions.clear();
    if (data.contains("onMousePress") && data.value("onMousePress").type() == QVariant::List) {
        eventActions = data.value("onMousePress").toList();

        for(int i=0; i < eventActions.size(); i++) {
            actionData = eventActions[i].toMap();
            action = ActionInfoManager::typeToAction(actionData, this);
            if (action)
                actions.append(action);
        }
        mEventToActions.insert(Interaction::MousePress, actions);
    }

    actions.clear();
    if (data.contains("onMouseRelease") && data.value("onMouseRelease").type() == QVariant::List) {
        eventActions = data.value("onMouseRelease").toList();

        for(int i=0; i < eventActions.size(); i++) {
            actionData = eventActions[i].toMap();
            action = ActionInfoManager::typeToAction(actionData, this);
            if (action)
                actions.append(action);
        }
        mEventToActions.insert(Interaction::MouseRelease, actions);
    }

    if (data.contains("boderWidth") && data.value("borderWidth").canConvert(QVariant::Int))
        mBorderWidth = data.value("borderWidth").toInt();
    if (data.contains("borderColor") && data.value("borderColor").type() == QVariant::List)
        mBorderColor = Utils::listToColor(data.value("borderColor").toList());

    mPadding = Padding(data.value("padding").toMap());
}

void Object::notify(const QString & key, const QVariant & value, const QVariant & prevValue)
{
    QVariantMap data;
    data.insert(key, value);
    data.insert("previousValue", prevValue);
    emit dataChanged(data);
}

void Object::onParentResized(int w, int h)
{
    if (mPercentWidth)
        setWidth(width()*w/100);

    if(mPercentHeight)
        setHeight(height()*h/100);
}

int Object::percentWidth() const
{
    return mPercentWidth;
}

int Object::percentHeight() const
{
    return mPercentHeight;
}

int Object::borderWidth()
{
    return mBorderWidth;
}

void Object::setBorderWidth(int w)
{
    mBorderWidth = w;
    emit dataChanged();
}

QColor Object::borderColor()
{
    return mBorderColor;
}

void Object::setBorderColor(const QColor & color)
{
    mBorderColor = color;
    emit dataChanged();
}

QString Object::defaultFontFamily()
{
    return mDefaultFont.family();
}

void Object::setDefaultFontFamily(const QString& family)
{
    mDefaultFont.setFamily(family);
}

int Object::defaultFontSize()
{
    return mDefaultFont.pixelSize();
}

void Object::setDefaultFontSize(int size)
{
    mDefaultFont.setPixelSize(size);
}

void Object::setDefaultFont(const QFont& font)
{
    mDefaultFont = font;
}

QFont Object::defaultFont()
{
    return mDefaultFont;
}

void Object::onResourceChanged(const QVariantMap & data)
{
    QVariantMap _data = data;
    //ignore coordinates
    if (_data.contains("x"))
        _data.remove("x");
    if (_data.contains("y"))
        _data.remove("y");
    setProperties(_data);
}

void Object::show()
{
    if (mBackground.image() && mBackground.image()->movie())
        mBackground.image()->movie()->start();
}

void Object::hide()
{
    if (mBackground.image() && mBackground.image()->movie())
        mBackground.image()->movie()->stop();
}

bool Object::keepAspectRatio()
{
    return mKeepAspectRatio;
}

void Object::setKeepAspectRatio(bool keep)
{
    mKeepAspectRatio = keep;
    if (mKeepAspectRatio && mAspectRatio != 1) {
        int w = width();
        int h = round(w/mAspectRatio);
        mSceneRect.setHeight(h);
        updateResizeRects();
    }
}

bool Object::setName(const QString & name)
{
    if (this->isValidName(name)){
        setObjectName(name);
        QVariantMap data;
        data.insert("name", name);
        emit dataChanged(data);
        return true;
    }

   return false;
}

QString Object::name()
{
    return objectName();
}

void Object::updateAspectRatio()
{
    mAspectRatio = (float) width() / height();
}

/*void Object::setEditorWidgetFilters(const QStringList& filters)
{
    if (! mEditorWidget)
        return;

    //mEditorWidget->setFilters();
}*/
