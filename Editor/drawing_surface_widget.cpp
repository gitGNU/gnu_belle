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

#include "drawing_surface_widget.h"

#include <QDebug>
#include <QFont>
#include <QFontDatabase>
#include <QMenu>

#include "textbox.h"
#include "scene.h"
#include "objectgroup.h"

static QWidget *mInstance = 0;

DrawingSurfaceWidget::DrawingSurfaceWidget(SceneManager *sceneManager, QWidget *parent) :
    QWidget(parent)
{
    mMousePressed = false;
    mResizing = false;
    mSceneManager = sceneManager;
    setContextMenuPolicy(Qt::CustomContextMenu);

    setFixedSize(QSize(Scene::width(), Scene::height()));
    setMouseTracking (true);

    mInstance = this;
    mObject = 0;

    /*QWidget *widget = new QWidget(this);
    widget->setFixedHeight(Scene::height());
    widget->setFixedWidth(Scene::width());
    //this->setWidget(widget);*/

    //create context menu actions
    mCancelEditObject = new QAction(QIcon(":/media/go-up.png"), tr("Cancel edit"), this);
    mEditObject = new QAction(QIcon(":/media/go-up.png"), tr("Edit"), this);

    mMoveUp = new QAction(QIcon(":/media/go-up.png"), tr("Move to front"), this);
    mMoveDown = new QAction(QIcon(":/media/go-down.png"), tr("Move to back"), this);
    mFillWidth = new QAction(QIcon(":/media/fit-width.png"), tr("Fill scene's width"), this);

    mCopyObject = new QAction(QIcon(":/media/editcopy.png"), tr("Copy"), this);
    mCopyObject->setShortcut(QKeySequence("Ctrl+C"));
    mCopyObject->setShortcutContext(Qt::WidgetShortcut);
    addAction(mCopyObject);

    mCutObject = new QAction(QIcon(":/media/editcut.png"), tr("Cut"), this);
    mCutObject->setShortcut(QKeySequence("Ctrl+X"));
    mCutObject->setShortcutContext(Qt::WidgetShortcut);
    addAction(mCutObject);

    mPasteObject = new QAction(QIcon(":/media/editpaste.png"), tr("Paste"), this);
    mPasteObject->setShortcut(QKeySequence("Ctrl+V"));
    mPasteObject->setShortcutContext(Qt::WidgetShortcut);
    addAction(mPasteObject);

    mDeleteObject = new QAction(QIcon(":/media/delete.png"), tr("Remove"), this);
    mDeleteObject->setShortcut(QKeySequence::Delete);
    mDeleteObject->setShortcutContext(Qt::WidgetShortcut);
    addAction(mDeleteObject);

    mClearBackground = new QAction(tr("Clear Background"), this);

    connect(mEditObject, SIGNAL(triggered()), this, SLOT(onEditObjectTriggered()));
    connect(mCancelEditObject, SIGNAL(triggered()), this, SLOT(onCancelEditObjectTriggered()));
    connect(mMoveUp, SIGNAL(triggered()), this, SLOT(onMoveUpTriggered()));
    connect(mMoveDown, SIGNAL(triggered()), this, SLOT(onMoveDownTriggered()));
    connect(mFillWidth, SIGNAL(triggered()), this, SLOT(onMoveFillWidthTriggered()));
    connect(mCopyObject, SIGNAL(triggered()), this, SLOT(onCopyTriggered()));
    connect(mCutObject, SIGNAL(triggered()), this, SLOT(onCutTriggered()));
    connect(mPasteObject, SIGNAL(triggered()), this, SLOT(onPasteTriggered()));
    connect(mDeleteObject, SIGNAL(triggered()), this, SLOT(onDeleteTriggered()));
    connect(mClearBackground, SIGNAL(triggered()), this, SLOT(onClearBackgroundTriggered()));
    connect(mSceneManager, SIGNAL(updateDrawingSurfaceWidget()), this, SLOT(update()));
    connect(mSceneManager, SIGNAL(resized(const QResizeEvent&)), this, SLOT(onResize(const QResizeEvent&)));
    //connect(ResourceManager::instance(), SIGNAL(resourceChanged()), this, SLOT(update()));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onCustomContextMenuRequested(const QPoint&)));
}

DrawingSurfaceWidget::~DrawingSurfaceWidget()
{
}

void DrawingSurfaceWidget::paintEvent(QPaintEvent* paint)
{

    if (mObject)
        paintObject(this);
    else if (mSceneManager->currentScene())
        paintSceneTo(this);
    else
        return;

    emit paintFinished();
}

void DrawingSurfaceWidget::paintObject(QPaintDevice * paintDevice)
{
    if(! mObject)
        return;

    QPainter painter(paintDevice);
    mObject->paint(painter);
    drawSelection(painter, mObject);
}

void DrawingSurfaceWidget::setObject(Object* obj)
{
    mObject = obj;
    if (mObject) {
        mObject->setX(RESIZE_RECT_WIDTH/2);
        mObject->setY(RESIZE_RECT_WIDTH/2);
        setFixedSize(QSize(mObject->width()+RESIZE_RECT_WIDTH, mObject->height()+RESIZE_RECT_WIDTH));
        update();
    }
    else {
        setFixedSize(QSize(Scene::width(), Scene::height()));
    }
}

Object* DrawingSurfaceWidget::object()
{
    return mObject;
}

void DrawingSurfaceWidget::paintSceneTo(QPaintDevice * paintDevice)
{
    Scene *scene = mSceneManager->currentScene();

    if (! scene || ! paintDevice)
        return;

    QPainter painter(paintDevice);
    scene->paint(painter);
    drawSelection(painter, scene->selectedObject());
}

void DrawingSurfaceWidget::drawSelection(QPainter& painter, Object* object)
{
    if (! object)
        return;

    QRectF rectf = object->sceneRect();
    painter.drawRect(rectf);

    QBrush brush(QColor(255,0,0));
    QPen pen;
    pen.setWidth(1);
    painter.save();
    painter.setPen(pen);

    QList<QRect> rects = object->resizeRects();
    foreach(const QRect& rect, rects) {
        painter.fillRect(rect, brush);
        painter.drawRect(rect);
    }

    painter.restore();
}


void DrawingSurfaceWidget::mousePressEvent ( QMouseEvent * event )
{

    mMousePressed = true;

    if (mObject) {

    } else {

        if (! mResizing && mSceneManager->currentScene())
            mSceneManager->currentScene()->selectObjectAt(event->x(), event->y());
    }
    //emit selectionChanged(scene->selectedObject());

    /*Object *obj = scene->selectedObject();
    scene->selectObjectAt(event->x(), event->y());
    Object *newObj = scene->selectedObject();

    emit selectionChanged(scene->selectedObject());

    if (newObj != obj) {
        if (newObj && newObj->editorWidget())
            newObj->editorWidget()->updateData(newObj);

        update();
    }*/

    QWidget::mousePressEvent(event);
}

void DrawingSurfaceWidget::mouseReleaseEvent ( QMouseEvent * event)
{
    bool resize = mResizing;

    mMousePressed = false;
    mResizing = false;
    Scene *scene = mSceneManager->currentScene();

    if (! scene )
        return;

    if (scene->selectedObject() && resize)
        scene->selectedObject()->stopResizing();

    scene->stopMoveSelectedObject();

    QWidget::mouseReleaseEvent(event);
}

void DrawingSurfaceWidget::mouseMoveEvent( QMouseEvent * event)
{
    Scene *scene = mSceneManager->currentScene();

    if (! scene  || ! scene->selectedObject()) {
        if (cursor().shape() != Qt::ArrowCursor)
            setCursor(Qt::ArrowCursor);
        return;
    }

    if (mResizing) {
        scene->selectedObject()->resize(event->x(), event->y());
        update();
        return;
    }

    int i;
    QList<QRect> rects =  scene->selectedObject()->resizeRects();
    for(i=0; i < rects.size(); i++) {
        if (rects.at(i).contains(event->x(), event->y()))
            break;
    }

    if (i == rects.size()) {
        setCursor(Qt::ArrowCursor);

        if (mMousePressed) {
            scene->moveSelectedObject(event->x(), event->y());
            update();
        }
    }
    else {

        if (i % 2 == 0) {
            if (i == 0 || i == 4)
                setCursor(Qt::SizeFDiagCursor);
            else
                setCursor(Qt::SizeBDiagCursor);
        }
        else {
            if (i == 1 || i == 5)
                setCursor(Qt::SizeVerCursor);
            else
                setCursor(Qt::SizeHorCursor);
        }

        if (mMousePressed) {
            mResizing = true;
            scene->selectedObject()->resize(i, event->x(), event->y());
            update();
        }
    }
}

void DrawingSurfaceWidget::resizeEvent(QResizeEvent * event)
{
    QWidget::resizeEvent(event);
    //mSceneManager->onResizeEvent(event);
}

void DrawingSurfaceWidget::onCustomContextMenuRequested(const QPoint& point)
{
    if (! mSceneManager->currentScene() || mObject)
        return;

    QMenu menu;
    Object* selectedObject = mSceneManager->currentScene()->selectedObject();

    if (selectedObject) {
        ObjectGroup * objectGroup = qobject_cast<ObjectGroup*>(selectedObject);
        if (objectGroup) {
            if (objectGroup->editingMode())
                menu.addAction(mCancelEditObject);
            else
                menu.addAction(mEditObject);
        }
        else if (selectedObject->hasObjectAsParent())
            menu.addAction(mCancelEditObject);

        menu.addAction(mMoveUp);
        menu.addAction(mMoveDown);
        menu.addAction(mFillWidth);
        menu.addSeparator();
        menu.addAction(mCopyObject);
        menu.addAction(mCutObject);
        if (! mSceneManager->clipboard()->isEmpty())
            menu.addAction(mPasteObject);
        menu.addAction(mDeleteObject);
    }
    else {
        if (! mSceneManager->clipboard()->isEmpty())
            menu.addAction(mPasteObject);
        if (mSceneManager->currentScene()->backgroundImage())
            menu.addAction(mClearBackground);
    }

    if (! menu.actions().isEmpty())
        menu.exec(mapToGlobal(point));

    mMousePressed = false;
}

void DrawingSurfaceWidget::onMoveUpTriggered()
{
    if (mSceneManager->currentScene())
        mSceneManager->currentScene()->moveSelectedObjectUp();
}

void DrawingSurfaceWidget::onMoveDownTriggered()
{
    if (mSceneManager->currentScene())
        mSceneManager->currentScene()->moveSelectedObjectDown();
}

void DrawingSurfaceWidget::onMoveFillWidthTriggered()
{
    if (mSceneManager->currentScene())
        mSceneManager->currentScene()->fillWidth();
}

void DrawingSurfaceWidget::onCopyTriggered()
{
    performOperation(Clipboard::Copy);
}

void DrawingSurfaceWidget::onCutTriggered()
{
    performOperation(Clipboard::Cut);
}

void DrawingSurfaceWidget::performOperation(Clipboard::Operation op)
{
    if (mSceneManager->currentScene() && mSceneManager->currentScene()->selectedObject()) {
        mSceneManager->clipboard()->clear();
        mSceneManager->setSrcScene(mSceneManager->currentScene());
        mSceneManager->clipboard()->add(mSceneManager->currentScene()->selectedObject(), op);
    }
}

void DrawingSurfaceWidget::onPasteTriggered()
{
    if (mSceneManager->currentScene()) {
        QList<Object*> objects = mSceneManager->clipboard()->objects();
        Clipboard::Operation op = mSceneManager->clipboard()->operation();
        Scene * srcScene = 0;

        foreach(Object* obj, objects) {
            srcScene = qobject_cast<Scene*>(obj->parent());

            if (op == Clipboard::Copy) {
                obj = obj->copy();
            }
            else if (op == Clipboard::Cut && srcScene) {
                srcScene->removeObject(obj, true);
            }

            mSceneManager->currentScene()->appendObject(obj);
        }

        mSceneManager->clipboard()->clear();
    }
}

void DrawingSurfaceWidget::onDeleteTriggered()
{
    if (mSceneManager->currentScene() && mSceneManager->currentScene()->selectedObject()) {
        mSceneManager->clipboard();
        mSceneManager->currentScene()->removeSelectedObject(true);
    }
}

void DrawingSurfaceWidget::onClearBackgroundTriggered()
{
    if (mSceneManager->currentScene())
        mSceneManager->currentScene()->clearBackground();
}

void DrawingSurfaceWidget::onEditObjectTriggered()
{
    if (mSceneManager->currentScene() && mSceneManager->currentScene()->selectedObject()) {
        ObjectGroup *objGroup = qobject_cast<ObjectGroup*>(mSceneManager->currentScene()->selectedObject());
        if (objGroup)
            objGroup->setEditingMode(true);
    }
}

void DrawingSurfaceWidget::onCancelEditObjectTriggered()
{
    if (mSceneManager->currentScene() && mSceneManager->currentScene()->selectedObject()) {
        Object* selectedObject = mSceneManager->currentScene()->selectedObject();
        ObjectGroup *objGroup = qobject_cast<ObjectGroup*>(selectedObject);

        if (! objGroup && selectedObject->parent()){
            objGroup = qobject_cast<ObjectGroup*>(selectedObject->parent());
        }

        if (objGroup)
            objGroup->setEditingMode(false);
    }
}


bool DrawingSurfaceWidget::eventFilter(QObject * obj, QEvent * ev)
{
    return false;
}

QWidget * DrawingSurfaceWidget::instance()
{
    return mInstance;
}

void DrawingSurfaceWidget::onResize(const QResizeEvent & event)
{
    this->setFixedSize(event.size());
}
