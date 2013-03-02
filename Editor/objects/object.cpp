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
    mBackgroundImage = 0;
    mBackgroundColor.setRgb(255, 255, 255, 0);
    mRoundedRect = false;
    mXRadius = 0;
    mYRadius = 0;
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
    return mBackgroundColor;
}

void Object::setBackgroundColor(const QColor & color)
{
    notify("backgroundColor", Utils::colorToList(color), Utils::colorToList(mBackgroundColor));
    int alpha = mBackgroundColor.alpha();
    mBackgroundColor = color;
    mBackgroundColor.setAlpha(alpha);
}

int Object::backgroundOpacity() const
{
    return mBackgroundColor.alpha();
}

void Object::setBackgroundOpacity(int alpha)
{
    int prevAlpha = mBackgroundColor.alpha();
    mBackgroundColor.setAlpha(alpha);
    notify("backgroundOpacity", alpha, prevAlpha);
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

int Object::xRadius() const
{
    return mXRadius;
}

void Object::setXRadius(int radius)
{
    mXRadius = radius;
    if (mRoundedRect) {
        QVariantMap data;
        data.insert("roundedRect", mRoundedRect);
        data.insert("xRadius", mXRadius);
        emit dataChanged(data);
    }
}

int Object::yRadius() const
{
    return mYRadius;
}


void Object::setYRadius(int radius)
{
    mYRadius = radius;
    if (mRoundedRect) {
        QVariantMap data;
        data.insert("roundedRect", mRoundedRect);
        data.insert("yRadius", mYRadius);
        emit dataChanged(data);
    }
}

bool Object::roundedRect() const
{
    return mRoundedRect;
}

void Object::setRoundedRect(bool rounded)
{
    mRoundedRect = rounded;

    QVariantMap data;
    data.insert("roundedRect", mRoundedRect);

    if (mRoundedRect) {
        data.insert("xRadius", mXRadius);
        data.insert("yRadius", mYRadius);
    }

    emit dataChanged(data);
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
    if (mBackgroundColor.alpha() > 0 || mBorderColor.isValid()) {
        painter.save();
        QBrush brush(backgroundColor());
        painter.setBrush(brush);
        painter.setOpacity(backgroundColor().alphaF());

        if (mBorderColor.isValid())
            painter.setPen(QPen(mBorderColor, mBorderWidth));
        else
            painter.setPen(QPen(backgroundColor(), mBorderWidth));

        QRect rect(mSceneRect.x(), mSceneRect.y(), contentWidth(), contentHeight());
        //rect.setWidth(rect.width()+mPadding.left()+ mPadding.right());
        //rect.setHeight(rect.height()+mPadding.top()+ mPadding.bottom());

        if (mBackgroundImage) {
            if (mBackgroundImage->movie())
                painter.drawPixmap(rect, mBackgroundImage->movie()->currentPixmap());
            else
                painter.drawPixmap(rect, *mBackgroundImage->pixmap());
        }
        else if (mRoundedRect)
            painter.drawRoundedRect(rect, mXRadius, mYRadius);
        else
            painter.drawRect(rect);

        painter.restore();
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
    if (mBackgroundImage && mBackgroundImage->path() == path)
        return;

    if (mBackgroundImage && mBackgroundImage->movie()) {
        mBackgroundImage->movie()->stop();
        mBackgroundImage->movie()->disconnect(this);
    }

    AnimationImage* image = ResourceManager::newImage(path);
    if (image && image->movie()) {
        connect(image->movie(), SIGNAL(frameChanged(int)), this, SIGNAL(dataChanged()));
        image->movie()->start();
    }

    ResourceManager::decrementReference(mBackgroundImage);
    QString prevPath = ResourceManager::imagePath(mBackgroundImage);
    mBackgroundImage = image;
    notify("backgroundImage", path, prevPath);
}

AnimationImage* Object::backgroundImage() const
{
    return mBackgroundImage;
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

    if (mPercentWidth) object.insert("width", QString("%1\%").arg(mPercentWidth));
    else object.insert("width", contentWidth());
    if (mPercentHeight) object.insert("height", QString("%1\%").arg(mPercentHeight));
    else object.insert("height", contentHeight());
    QVariantList color;
    color << mBackgroundColor.red() << mBackgroundColor.green() << mBackgroundColor.blue()
             << mBackgroundColor.alpha();
    object.insert("backgroundColor", color);

    if (mRoundedRect) {
        object.insert("roundedRect", mRoundedRect);
        object.insert("xRadius", mXRadius);
        object.insert("yRadius", mYRadius);
    }

    if (mBackgroundImage) {
        object.insert("backgroundImage", mBackgroundImage->fileName());
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

    movePoint(mOriginalResizePointIndex, QPoint(x, y));
    updateResizeRects();
    QVariantMap data;
    data.insert("width", width());
    data.insert("height", height());
    emit dataChanged(data);
}


void Object::movePoint(int which, const QPoint& point)
{
    switch(which)
    {
    case 0:
        mSceneRect.setTopLeft(point);
        updateResizeRect(0, mSceneRect.topLeft());
        break;
    case 1:
         mSceneRect.setY(point.y());
         break;
    case 2:
        mSceneRect.setTopRight(point);
        updateResizeRect(2, mSceneRect.topRight());
        break;
    case 3:
        mSceneRect.setWidth(point.x()-mSceneRect.x());
        break;
    case 4:
        mSceneRect.setBottomRight(point);
        updateResizeRect(4, mSceneRect.bottomRight());
        break;
    case 5:
        mSceneRect.setHeight(point.y()-mSceneRect.y());
        break;
    case 6:
        mSceneRect.setBottomLeft(point);
        updateResizeRect(6, mSceneRect.bottomLeft());
        break;
    case 7:
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


    if (data.contains("backgroundColor") && data.value("backgroundColor").type() == QVariant::List) {
        QColor color = Utils::listToColor(data.value("backgroundColor").toList());
        if (color.isValid())
            mBackgroundColor = color;

    }

    if (data.contains("backgroundOpacity") && data.value("backgroundOpacity").type() == QVariant::Int) {
        if (data.contains("previousValue") && data.value("previousValue").type() == QVariant::Int)
            if (data.value("previousValue").toInt() != mBackgroundColor.alpha())
                return;
        mBackgroundColor.setAlpha(data.value("backgroundOpacity").toInt());
    }

    if (data.contains("backgroundImage") && data.value("backgroundImage").type() == QVariant::String) {
        mBackgroundImage = ResourceManager::newImage(data.value("backgroundImage").toString());
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

    if (data.contains("roundedRect") && data.value("roundedRect").type() == QVariant::Bool) {
        mRoundedRect = data.value("roundedRect").toBool();

        if (mRoundedRect) {
            if (data.contains("xRadius") && data.value("xRadius").canConvert(QVariant::Int))
                mXRadius = data.value("xRadius").toInt();
            if (data.contains("yRadius") && data.value("yRadius").canConvert(QVariant::Int))
                mYRadius = data.value("yRadius").toInt();
        }
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

Scene* Object::scene()
{
    //shouldn't happen, but just in case
    if (! this->parent())
        return SceneManager::currentScene();

    //usual case - object's parent is the scene
    if (qobject_cast<Scene*>(this->parent()))
        return qobject_cast<Scene*>(this->parent());

    //in case this object is inside another object
    if (qobject_cast<Object*>(this->parent())) {
        Object* object = qobject_cast<Object*>(this->parent());
        if (object->scene())
            return object->scene();
    }

    //shouldn't happen either, but just in case
    return SceneManager::currentScene();
}

bool Object::isValidName(const QString& name)
{
    //check if parent is the scene
    Scene* scene = qobject_cast<Scene*>(parent());
    if (scene)
        return scene->isValidObjectName(name);

    //check if parent is the resource manager
    ResourceManager* resourceManager = qobject_cast<ResourceManager*>(parent());
    if (resourceManager)
        return resourceManager->isValidName(name);

    return true;
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
    if (mBackgroundImage && mBackgroundImage->movie())
        mBackgroundImage->movie()->start();
}

void Object::hide()
{
    if (mBackgroundImage && mBackgroundImage->movie())
        mBackgroundImage->movie()->stop();
}

/*void Object::setEditorWidgetFilters(const QStringList& filters)
{
    if (! mEditorWidget)
        return;

    //mEditorWidget->setFilters();
}*/
